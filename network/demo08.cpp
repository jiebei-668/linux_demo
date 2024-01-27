#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <poll.h>
#include <time.h>

#define MAX_FD_NUM 1024
#define MAX_BACKLOG 512
struct pollfd sockets[MAX_FD_NUM];
// 记录所有非监听端口的对端口
// 监听端口的对端口记为-1
int opposite_socket[MAX_FD_NUM];
// 记录所有作为监听socket的socket的sockaddr，具体个数由参数文件配置
// 如果socket=ii是监听socket，那么对应的代理服务器的sozkaddr是listensockaddrs[ii]
struct sockaddr_in listensockaddrs[MAX_FD_NUM];
// 记录所有的被代理服务器的sockaddr，具体个数由参数文件配置
// 如果socket=ii是监听socket，那么对应的被代理服务器的sockaddr是connectsockaddrs[ii]
struct sockaddr_in connectsockaddrs[MAX_FD_NUM];
// 如果fd==ii是监听端口，那么is_listenfd为true
bool is_listenfd[MAX_FD_NUM];
// 记录最大fd
int maxfd = -1;

// 程序退出函数
void exit_fun(int sig);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo08 configfile\n");
		printf("Example: ./demo08 ./forwardproxy.config\n");
		exit(-1);
	}
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		sockets[ii].fd = -1;
		opposite_socket[ii] = -1;
		memset(&listensockaddrs[ii], 0, sizeof(struct sockaddr_in));
		memset(&connectsockaddrs[ii], 0, sizeof(struct sockaddr_in));
		is_listenfd[ii] = false;
	}
	maxfd = -1;
	// 初始化所有监听端口
	// 目前暂时只设定两个监听端口，分别监听8881和8882，分别对应被代理服务器127.0.0.1-8883和127.0.0.1-8884
	// 这里可以扩展为读取配置文件，初始化所有监听端口
	int listenports[2] = {8881, 8882};
	int connectports[2] = {8883, 8884};
	for(int ii = 0; ii < sizeof(listenports)/sizeof(listenports[0]); ii++)
	{
		int listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if(listenfd == -1)
		{
			printf("proxy socket failed!\n");
			printf("errno[%d] info[%s]\n", errno, strerror(errno));
			exit_fun(-1);
		}

		sockets[listenfd].fd = listenfd;
		sockets[listenfd].events = POLLIN;
		listensockaddrs[listenfd].sin_family = AF_INET;
		listensockaddrs[listenfd].sin_addr.s_addr = inet_addr("127.0.0.1");
		listensockaddrs[listenfd].sin_port = htons(listenports[ii]);
		connectsockaddrs[listenfd].sin_family = AF_INET;
		connectsockaddrs[listenfd].sin_addr.s_addr = inet_addr("127.0.0.1");
		connectsockaddrs[listenfd].sin_port = htons(connectports[ii]);

		int opt = 1;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
		if(bind(listenfd, (struct sockaddr *)&listensockaddrs[listenfd], sizeof(listensockaddrs[listenfd])) == -1)
		{
			printf("proxy bind() failed!\n");
			printf("errno[%d] info[%s]\n", errno, strerror(errno));
			exit_fun(-1);
		}
		if(listen(listenfd, MAX_BACKLOG) == -1)
		{
			printf("proxy listen() failed!\n");
			printf("errno[%d] info[%s]\n", errno, strerror(errno));
			exit_fun(-1);
		}
		if(listenfd > maxfd)
		{
			maxfd = listenfd;
		}
		// 这段代码可以放下面，而设置pollfd的代码必须放上面，因为socket()后需要bind和listen()，如果失败，在exit_fun中需要关闭socket，所以设置pollfd的fd的代码必须紧跟socket
		is_listenfd[listenfd] = true;
		opposite_socket[listenfd] = -1;
	}

	while(true)
	{
		int event_num = poll(sockets, maxfd+1, -1);
		if(event_num == -1)
		{
			// 处理poll错误的代码
			exit(-1);
		}
		if(event_num == 0)
		{
			printf("poll() timeout...\n");
			continue;
		}
		for(int fd = 0; fd <= maxfd; fd++)
		{
			if(sockets[fd].fd == -1 || (sockets[fd].revents&POLLIN) == 0)
			{
				continue;
			}
			// 处理监听socket的事件
			if(is_listenfd[fd] == true)
			{
				int connfd = accept(fd, NULL, NULL);
				if(connfd == -1)
				{
					// 处理失败，给个提示
					continue;
				}
				sockets[connfd].fd = connfd;
				sockets[connfd].events = POLLIN;
				maxfd = connfd > maxfd ? connfd : maxfd;
				int connfd_opposite = socket(AF_INET, SOL_SOCKET, 0);
				if(connfd_opposite == -1)
				{
					// 处理失败
					close(connfd);
					continue;
				}
				sockets[connfd_opposite].fd = connfd_opposite;
				sockets[connfd_opposite].events = POLLIN;
				maxfd = connfd_opposite > maxfd ? connfd_opposite : maxfd;
				int connect_ret = connect(connfd_opposite, (struct sockaddr *)&connectsockaddrs[fd], sizeof(connectsockaddrs[fd]));
				if(connect_ret == -1)
				{
					// 处理失败
					close(connfd);
					close(connfd_opposite);
					continue;
				}
				else
				{
					opposite_socket[connfd] = connfd_opposite;
					opposite_socket[connfd_opposite] = connfd;
				}

			}
			// 处理非监听socket的事件
			else
			{
				char buf[1600];
				memset(buf, 0, sizeof(buf));
				int rsize = recv(fd, buf, 1600, 0);
				send(opposite_socket[fd], buf, rsize, 0);
			}
		}
		
		
	}
	return 0;
}
// 程序退出函数
void exit_fun(int sig)
{
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		if(~sockets[ii].fd)
		{
			printf("socket[%d] closed...\n", sockets[ii].fd);
			close(sockets[ii].fd);
			sockets[ii].fd = -1;
		}
	}
	exit(0);
}
