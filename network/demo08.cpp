/****************************************************************************
 * program: demo08.cpp
 * author: jiebei
 * 本程序使用poll实现正向代理，先暂时固定用127.0.0.1 8881代理127.0.0.1-8883，用127.0.0.1-8882代理127.0.0.1-8884
 * 使用信号2（Ctrl-c)或信号15可以终止程序
 * Usage: ./demo08 configfile
 * Example: ./demo08 ./forwardproxy.config
 * 注：还暂时没有用配置文件来配置代理的服务器的地址，后续更新
 **************************************************************************/
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

// 最大的打开的socket数量
#define MAX_FD_NUM 1024
// listen()的第二个参数
#define MAX_BACKLOG 512
struct pollfd sockets[MAX_FD_NUM];
// 记录所有非监听socket的对端socket
// 监听socket的对端socket记为-1
int opposite_socket[MAX_FD_NUM];
// 记录所有监听socket的sockaddr，具体个数由参数文件配置
// 如果socket=ii是监听socket，那么该监听socket的sockaddr是listensockaddrs[ii], 也就是说该socet对应的代理服务器的sozkaddr是listensockaddrs[ii]
struct sockaddr_in listensockaddrs[MAX_FD_NUM];
// 记录所有的被代理服务器的sockaddr，具体个数由参数文件配置
// 如果socket=ii是非监听socket，那么对应的被代理服务器的sockaddr是connectsockaddrs[ii]
// 当由客户端连接到代理程序某个监听socket=ii时，代理程序connect()被代理服务器时需要connectsockaddrs[ii]
struct sockaddr_in connectsockaddrs[MAX_FD_NUM];
// 如果socket=ii是监听socket，那么is_listenfd[ii]为true
bool is_listenfd[MAX_FD_NUM];
// 记录最大socket
int maxfd = -1;

// 程序退出函数，主要需要关闭所有socket
void exit_fun(int sig);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo08 configfile\n");
		printf("Example: ./demo08 ./forwardproxy.config\n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
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
		maxfd = listenfd > maxfd ? listenfd : maxfd;
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
			printf("proxy poll() failed!\n");
			printf("errno[%d] info[%s]\n", errno, strerror(errno));
			exit(-1);
		}
		// 暂时没有设置tiemout
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
					printf("proxy accept(%d) failed!\n", fd);
					printf("errno[%d] info[%s]\n", errno, strerror(errno));
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
					sockets[connfd].fd = -1;
					printf("proxy accept success, but socket() failed!\n");
					printf("errno[%d] info[%s]\n", errno, strerror(errno));
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
					sockets[connfd].fd = -1;
					close(connfd_opposite);
					sockets[connfd_opposite].fd = -1;
					printf("proxy accept() and socket() succuss, but connect() failed!\n");
					continue;
				}
				else
				{
					opposite_socket[connfd] = connfd_opposite;
					opposite_socket[connfd_opposite] = connfd;
				}
			}
			// 处理非监听socket的事件
			// 若是断开连接，需要关闭两端的sockets
			// 其他只需要将消息发给对端
			else
			{
				char buf[1600];
				memset(buf, 0, sizeof(buf));
				int rsize = recv(fd, buf, 1600, 0);
				// 一方断开时，同时关闭两端的socket
				if(rsize == 0)
				{
					close(fd);
					sockets[fd].fd = -1;
					close(opposite_socket[fd]);
					sockets[opposite_socket[fd]].fd = -1;
				}
				// 没有断开时，将消息发给对端
				else
				{
					send(opposite_socket[fd], buf, rsize, 0);
				}
			}
		}
		
		
	}
	return 0;
}
// 程序退出函数
void exit_fun(int sig)
{
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);
	// 将所有pollfd中的socket关闭
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
