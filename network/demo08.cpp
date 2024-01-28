/****************************************************************************
 * program: demo08.cpp
 * author: jiebei
 * 本程序使用poll实现正向代理
 * 使用信号2（Ctrl-c)或信号15可以终止程序
 * Usage: ./demo08 proxy_port server_ip server_port
 * Example: ./demo08 6666 127.0.0.1 8888
 * 注：还暂时没有用配置文件来配置多个代理服务器的地址，后续更新
 **************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "libsocket.h"
#include <fcntl.h>

// 最大的打开的socket数量
#define MAX_FD_NUM 1024
// listen()的第二个参数
#define MAX_BACKLOG 512
struct pollfd sockets[MAX_FD_NUM];
// 记录所有非监听socket的对端socket
// 监听socket的对端socket记为-1
int opposite_socket[MAX_FD_NUM];
/* 留作扩展
// 记录所有监听socket的sockaddr，具体个数由参数文件配置
// 如果socket=ii是监听socket，那么该监听socket的sockaddr是listensockaddrs[ii], 也就是说该socet对应的代理服务器的sozkaddr是listensockaddrs[ii]
// struct sockaddr_in listensockaddrs[MAX_FD_NUM];

// 记录所有的被代理服务器的sockaddr，具体个数由参数文件配置
// 如果socket=ii是非监听socket，那么对应的被代理服务器的sockaddr是connectsockaddrs[ii]
// 当由客户端连接到代理程序某个监听socket=ii时，代理程序connect()被代理服务器时需要connectsockaddrs[ii]
// struct sockaddr_in connectsockaddrs[MAX_FD_NUM];

// 如果socket=ii是监听socket，那么is_listenfd[ii]为true
bool is_listenfd[MAX_FD_NUM];
*/
// 记录最大socket
int maxfd = -1;

// 程序退出函数，主要需要关闭所有socket
void exit_fun(int sig);

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		printf("Usage: ./demo08 proxy_port server_ip server_port\n");
		printf("Example: ./demo08 6666 127.0.0.1 8888\n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		sockets[ii].fd = -1;
		opposite_socket[ii] = -1;
	}
	maxfd = -1;
	// 初始化所有监听端口
	// 这里可以扩展为读取配置文件，初始化所有监听端口
	int listenfd;
	int listenport = atoi(argv[1]);
	int connectport = atoi(argv[3]);
	listenfd = init_server("127.0.0.1", listenport);
	if(listenfd == -1)
	{
		exit_fun(-1);
	}
	// 设置监听socket为非阻塞
	//fcntl(listenfd, F_SETFL, O_NONBLOCK|fcntl(listenfd, F_GETFL));
	set_nonblock(listenfd);

	sockets[listenfd].fd = listenfd;
	sockets[listenfd].events = POLLIN;
	maxfd = listenfd > maxfd ? listenfd : maxfd;
	opposite_socket[listenfd] = -1;

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
			if(fd == listenfd)
			{
				int connfd = accept(fd, NULL, NULL);
				if(connfd == -1)
				{
					// 处理失败，给个提示
					printf("proxy accept(%d) failed!\n", fd);
					printf("errno[%d] info[%s]\n", errno, strerror(errno));
					continue;
				}
				// 设置为非阻塞
				//fcntl(connfd, F_SETFL, O_NONBLOCK|fcntl(connfd, F_GETFL));
				set_nonblock(connfd);
				sockets[connfd].fd = connfd;
				sockets[connfd].events = POLLIN;
				maxfd = connfd > maxfd ? connfd : maxfd;
				int connfd_opposite = init_client_and_connect(argv[2], connectport);
				if(connfd_opposite == -1)
				{
					// 处理失败
					close(connfd);
					sockets[connfd].fd = -1;
					printf("proxy accept success, but socket() or connect() failed!\n");
					printf("errno[%d] info[%s]\n", errno, strerror(errno));
					continue;
				}
				// 设置为非阻塞
				//fcntl(connfd_opposite, F_SETFL, O_NONBLOCK|fcntl(connfd_opposite, F_GETFL));
				set_nonblock(connfd_opposite);
				
				sockets[connfd_opposite].fd = connfd_opposite;
				sockets[connfd_opposite].events = POLLIN;
				maxfd = connfd_opposite > maxfd ? connfd_opposite : maxfd;
				opposite_socket[connfd] = connfd_opposite;
				opposite_socket[connfd_opposite] = connfd;
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
