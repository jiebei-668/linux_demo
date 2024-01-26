/***************************************************************************
 * program: demo07(.cpp)
 * author: jiebei
 * 这是一个使用poll的网络通信服务端程序，它接收客户端的消息并原封不动返回。使用信号2（Ctrl-c）或信号15可以终止信号。
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
#include <poll.h>
#define MAX_FD_NUM 1024
#define MAX_BACKLOG 512
struct pollfd sockets[MAX_FD_NUM];
// 程序退出时，关闭所有的socket
void exit_fun(int sig);
int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo07 port\n");
		printf("Example: ./demo07 8888\n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)
	{
		printf("socket() failed!\n");
		printf("errno[%d] info[%s]\n", errno, strerror(errno));
		exit(-1);
	}
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(atoi(argv[1]));

	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
	if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		printf("server bind() failed!\n");
		printf("errno[%d] info[%s]\n", errno, strerror(errno));
		close(listenfd);
		exit(-1);
	}
	if(listen(listenfd, MAX_BACKLOG) == -1)
	{
		printf("server listen() failed!\n");
		printf("errno[%d] info[%s]\n", errno, strerror(errno));
		close(listenfd);
		exit(-1);
	}
	// 初始化sockets
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		sockets[ii].fd = -1;
	}
	sockets[listenfd].fd = listenfd;
	sockets[listenfd].events = POLLIN;
	int maxfd = listenfd;

	char buf[512];
	memset(buf, 0, sizeof(buf));
	while(true)
	{
		int events_num = poll(sockets, maxfd+1, -1);
		if(events_num < 0)
		{
			// 这里清理掉资源（关socket）后退出
			exit_fun(-1);
		}
		if(events_num == 0)
		{
			printf("poll timeout...\n");
			continue;
		}
		for(int ii = 0; ii <= maxfd; ii++)
		{
			if(sockets[ii].fd == -1 || (sockets[ii].revents & POLLIN) == 0)
			{
				continue;
			}
			if(sockets[ii].fd == listenfd)
			{
				int connfd = accept(listenfd, NULL, NULL);
				if(connfd == -1)
				{
					printf("accept() failed!\n");
					printf("errno[%d] info[%s]\n", errno, strerror(errno));
					exit_fun(-1);
				}
				// 新连接的sock如果大于maxfd，就更新maxfd
				// 将新连接的socket加入sockets
				sockets[connfd].fd = connfd;
				sockets[connfd].events = POLLIN;
				if(connfd > maxfd)
				{
					maxfd = connfd;
				}
				continue;
			}
			// 转发消息或关闭连接
			int ret = recv(sockets[ii].fd, buf, 512, 0);
			if(ret <= 0)
			{
				printf("客户端[%d]已断开连接。。。\n", sockets[ii].fd);
				if(maxfd == sockets[ii].fd)
				{
					for(int ii = maxfd-1; ii >= 0; ii--)
					{
						if(~sockets[ii].fd)
						{
							maxfd = sockets[ii].fd;
							break;
						}
					}
				}
				close(sockets[ii].fd);
				sockets[ii].fd = -1;
			}
			else
			{
				printf("recv from client[%d]: ==%s==\n", sockets[ii].fd, buf);
				send(sockets[ii].fd, buf, 512, 0);
				printf("send to client[%d]: ===%s===\n", sockets[ii].fd, buf);
			}

		}
	}
	return 0;
}
// 程序退出时，关闭所有的socket
void exit_fun(int sig)
{
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);
	// printf("exit_fun POLLIN=%d POLLOUT=%d\n", POLLIN, POLLOUT);
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		if(~sockets[ii].fd)
		{
			printf("socket[%d] closed...\n", sockets[ii].fd);
			close(sockets[ii].fd);
			sockets[ii].fd = -1;
		}
	}
	printf("server exit...\n");
	exit(0);
}
