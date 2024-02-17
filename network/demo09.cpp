/********************************************************************************************
 * program: demo09.cpp
 * author: jiebei
 * 本程序是反向代理程序的内网端程序，接通被代理服务程序和通过外网反向代理程序中转的外网客户端程序。参数为外网服务器地址与外网代理程序和内网代理程序通信的端口。
 * 程序流程：
 * 1 外网代理程序初始化和内网代理程序通信的socket（监听socket）
 * 2 内网代理程序向外网代理程序发起连接，外网代理程序接受内网代理程序的连接建立控制通道
 * 3 外网代理程序解析被代理服务程序的参数（文件），逐个建立和外网客户端程序通信的socket（监听socket）
 * 3 循环执行 4
 * 4 当外网代理程序收到外网客户端程序连接请求后，向内网代理程序通过控制通道发送外网客户端程序请求的内网被代理程序的信息。内网代理程序接收外网代理程序通过控制通道发来的被代理程序的信息向被代理服务程序和外网代理程序发起连接，并且将这两端socket对接。外网代理程序接受内网代理程序的连接后同样地将内网代理程序的socket和外网客户端socket对接。这样建立了被代理程序和外网客户程序的连接后，外网客户端程序就可以访问被代理程序。
 * Usage: ./demo09 out_ip out_port
 * Example: ./demo09 127.0.0.1 8866
 *******************************************************************************************/
#include "libsocket.h"
#define MAX_FD_NUM 1024
struct pollfd sockets[MAX_FD_NUM];
int cmdsocket;
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
		printf("Usage: ./demo09 out_ip out_port\n");
		printf("Example: ./demo09 127.0.0.1 8866\n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		sockets[ii].fd = -1;
		opposite_socket[ii] = -1;
	}
	// 向外网代理程序发起连接，建立控制通道
	if((cmdsocket = init_client_and_connect(argv[1], atoi(argv[2]))) == -1)
	{
		printf("内网代理程序错误[0]-init_client_and_connect() failed\n");
		exit(-1);
	}
	// 设置非阻塞
	set_nonblock(cmdsocket);
	maxfd = cmdsocket;
	sockets[cmdsocket].fd = cmdsocket;
	sockets[cmdsocket].events = POLLIN;
	
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
			// 如果是控制socket，就需要接收控制信息并同时向外网代理程序和由控制信息指定的内网被代理程序发起连接
			if(fd == cmdsocket)
			{
				char cmdbuf[100];
				memset(cmdbuf, 0, sizeof(cmdbuf));
				int rsize = recv(sockets[fd].fd, cmdbuf, sizeof(cmdbuf), 0);
				if(rsize <= 0)
				{
					// 错误处理
					continue;
				}
				char *sep_pos = strstr(cmdbuf, "<sep>");
				char ip[sep_pos-cmdbuf+1];
				memset(ip, 0, sizeof(ip));
				strncpy(ip, cmdbuf, sep_pos-cmdbuf);
				// 向内网被代理程序发起连接
				int in_connfd = init_client_and_connect(ip, atoi(sep_pos+5));
				if(in_connfd == -1)
				{
					continue;
				}
				// 向外网代理程序发起连接
				int out_connfd = init_client_and_connect(argv[1], atoi(argv[2]));
				if(out_connfd == -1)
				{
					close(in_connfd);
					continue;
				}
				// 对接外网代理程序和内网被代理程序
				set_nonblock(in_connfd);
				set_nonblock(out_connfd);
				sockets[in_connfd].fd = in_connfd;
				sockets[in_connfd].events = POLLIN;
				maxfd = maxfd > in_connfd ? maxfd : in_connfd;
				sockets[out_connfd].fd = out_connfd;
				sockets[out_connfd].events = POLLIN;
				maxfd = maxfd > out_connfd ? maxfd : out_connfd;
				opposite_socket[in_connfd] = out_connfd;
				opposite_socket[out_connfd] = in_connfd;
				continue;
			}
			// 如果不是控制socket，将数据发给对端
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
