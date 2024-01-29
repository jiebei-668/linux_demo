/********************************************************************************************
 * program: demo10.cpp
 * author: jiebei
 * 本程序是反向代理程序的内网端程序，接通被代理服务程序和通过外网反向代理程序中转的外网客户端程序。目前暂时由参数写死外网的代理端口和内网的端口以及被代理服务程序的地址，且暂时只代理一个，后续由配置文件指定，可指定多个代理。
 * 程序流程：外网代理程序和内网代理程序初始化监听端口，当外网代理程序收到连接请求后向内网代理程序发起连接，内网代理程序接收连接并向被代理服务程序发起连接。这样建立了被代理程序和外网客户程序的连接后，外网客户端程序就可以访问被代理程序。
 * Usage: ./demo10 out_port in_port
 * Example: ./demo10 8866 6688
 *******************************************************************************************/
#include "libsocket.h"
#define MAX_FD_NUM 1024
struct pollfd sockets[MAX_FD_NUM];
int maxfd;
// 记录对端socket的结构，如果没有对端socket为-1
// 如果ii的对端为jj，那么opposite_socket[ii]=jj opposite_socket[jj]=ii
int opposite_socket[MAX_FD_NUM];
// 退出函数，清理资源，关socket
void exit_fun(int sig);
// 更新maxfd的值
int update_maxfd();

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("Usage: ./demo10 out_port in_port\n");
		printf("Example: ./demo10 8866 6688 \n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		sockets[ii].fd = -1;
		opposite_socket[ii] = -1;
	}
	int listenfd = init_server("127.0.0.1", atoi(argv[1]));
	if(listenfd == -1)
	{
		printf("init_server() failed...\n");
		exit(-1);
	}
	// 设置可重用端口
	set_reuseaddr(listenfd);
	// 设置非阻塞
	set_nonblock(listenfd);
	// 更新pollfd
	sockets[listenfd].fd = listenfd;
	sockets[listenfd].events = POLLIN;
	// 更新maxfd
	maxfd = listenfd;
	
	while(true)
	{
		int event_num = poll(sockets, maxfd+1, -1);
		if(event_num == -1)
		{
			// 处理错误并退出
			printf("poll() failed...\n");
			printf("errno[%d] info[%s]\n", errno, strerror(errno));
			exit_fun(-1);
		}
		if(event_num == 0)
		{
			// 处理超时
			continue;
		}
		for(int fd = 0; fd <= maxfd; fd++)
		{
			if(sockets[fd].fd == -1 || (sockets[fd].revents&POLLIN) == 0)
			{
				continue;
			}
			if(fd == listenfd)
			{
				int connfd = accept(listenfd, NULL, NULL);
				if(connfd == -1)
				{
					continue;
				}
				int connfd_opposite = init_client_and_connect("127.0.0.1", atoi(argv[2]));
				if(connfd_opposite == -1)
				{
					close(connfd);
					continue;
				}
				// 设置非阻塞
				set_nonblock(connfd);
				set_nonblock(connfd_opposite);
				// 更新maxfd
				maxfd = connfd_opposite > maxfd ? connfd_opposite: maxfd;
				maxfd = connfd > maxfd ? connfd : maxfd;
				// 更新pollfd结构体
				sockets[connfd].fd = connfd;
				sockets[connfd].events = POLLIN;
				sockets[connfd_opposite].fd = connfd_opposite;
				sockets[connfd_opposite].events = POLLIN;
				// 更新记录对端socket数组
				opposite_socket[connfd] = connfd_opposite;
				opposite_socket[connfd_opposite] = connfd;
				continue;
			}
			// 如果不是监听socket，将数据发给对端
			char buf[1600];
			memset(buf, 0, sizeof(buf));
			int rsize = recv(fd, buf, sizeof(buf), 0);
			if(rsize <= 0)
			{
				close(fd);
				close(opposite_socket[fd]);
				sockets[fd].fd = -1;
				sockets[opposite_socket[fd]].fd = -1;
				maxfd = update_maxfd();
			}
			else
			{
				send(opposite_socket[fd], buf, rsize, 0);
			}
		}
	}

	return 0;
}
void exit_fun(int sig)
{
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		if(~sockets[ii].fd)
		{
			close(sockets[ii].fd);
			sockets[ii].fd = -1;
		}
	}
	exit(-1);
}
int update_maxfd()
{
	int ii = MAX_FD_NUM;
	for( ii = MAX_FD_NUM-1; ii >= 0; ii--)
	{
		if(~sockets[ii].fd)
		{
			break;
		}
	}
	return sockets[ii].fd;
}
