/***************************************************************************
 * program: demo06(.cpp)
 * author: jiebei
 * 这是一个使用select的网络通信服务端程序，它接收客户端的消息并原封不动返回。使用信号2（Ctrl-c）或信号15可以终止信号。
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
#include <time.h>
#define MAX_BACKLOG 512

int listen_fd = -1;
// 监听socket和所有连接socket
fd_set readfds;
// 记录最大的socket
int maxfd = -1;
// 程序退出时的清理函数
void exit_fun(int sig);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo06 port\n");
		printf("Example: ./demo06 8888\n");
		return 0;
	}

	signal(2, exit_fun);
	signal(15, exit_fun);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd == -1)
	{
		printf("server socket() failed!\n");
		printf("errno[%d] info[%s]\n", errno, strerror(errno));
		exit(-1);
	}
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	int opt = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

	if(bind(listen_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		printf("server bind() failed!\n");
		printf("errno[%d] info[%s]\n", errno, strerror(errno));
		close(listen_fd);
		exit(-1);
	}
	if(listen(listen_fd, MAX_BACKLOG) == -1)
	{
		printf("server listen() failed!\n");
		printf("errto[%d] info[%s]\n", errno, strerror(errno));
		close(listen_fd);
		exit(-1);
	}
	FD_ZERO(&readfds);
	maxfd = listen_fd;
	FD_SET(listen_fd, &readfds);
	fd_set tmpfds;
	FD_ZERO(&tmpfds);
	// 以下可以设置select的超时机制
	struct timeval st_timeval;
	memset(&st_timeval, 0, sizeof(struct timeval));
	st_timeval.tv_sec = 10;
	st_timeval.tv_usec = 0;
	while(true)
	{
		tmpfds = readfds;
		int cntfd = select(maxfd+1, &tmpfds, NULL, NULL, NULL);
		// 这是带10秒超时机制的select
		// int cntfd = select(maxfd+1, &tmpfds, NULL, NULL, &st_timeval);
		if(cntfd == -1)
		{
			printf("select() failed!\n");
			exit_fun(-1);
		}
		if(cntfd == 0)
		{
			printf("select() timeout...\n");
			continue;
		}
		for(int i = 0; i <= maxfd; i++)
		{
			if(FD_ISSET(i, &tmpfds) == 0)
			{
				continue;
			}
			if(i == listen_fd)
			{
				int connfd = accept(listen_fd, NULL, NULL);
				if(connfd == -1)
				{
					printf("accept() failed!\n");
					printf("errno[%d] info[%s]\n", errno, strerror(errno));
					exit_fun(-1);
				}
				// 新连接的sock如果大于maxfd，就更新maxfd
				// 将新连接的socket加入readfds
				FD_SET(connfd, &readfds);
				if(connfd > maxfd)
				{
					maxfd = connfd;
				}
			}
			else
			{
				char buf[512];
				memset(buf, 0, sizeof(buf));
				int ret = recv(i, buf, 512, 0);
				if(ret <= 0)
				{
					// 客户端主动断开，需要判断当前断开的socket是否maxfd并处理
					printf("client[%d] disconnect...\n", i);
					close(i);
					FD_CLR(i, &readfds);
					if(i == maxfd)
					{
						for(int jj = maxfd-1; jj >= 0; jj--)
						{
							if(FD_ISSET(jj, &readfds))
							{
								maxfd = jj;
								break;
							}
						}
					}

				}
				else
				{
					// 接受消息并原文返回
					printf("recv from client[%d]: ==%s==\n", i, buf);
					send(i, buf, 512, 0);
					printf("send to client[%d]: ==%s==\n", i, buf);
				}

			}
		}
	}

	return 0;
}
void exit_fun(int sig)
{
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);

	for(int ii = 0; ii <= maxfd; ii++)
	{
		if(FD_ISSET(ii, &readfds))
		{
			printf("socket[%d] close...\n", ii);
			close(ii);
		}
	}
	printf("server exit...\n");
	exit(0);
}
