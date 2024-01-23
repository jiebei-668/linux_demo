/*****************************************************************************
 * 程序名：demo05.cpp
 * 作者：jiebei
 * 功能：这是一个多进程网络通信服务端，它接受客户端连接后将接受到的来自客户端的消息原封不动发送回去。信号2（Ctrl-c）或信号15可以终止该程序
 ***************************************************************************/
#include <stdio.h>
#include <vector>
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

#define MAX_BACKLOG 512

int listen_fd = -1;
int connfd = -1;
// 存放所有与客户端通信的子线程的线程id
std::vector<pthread_t> v_thid;
// 操作v_thid的锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// 和客户端进行通信的线程函数
void *thmain(void *);
// 与客户端通信的线程的清理函数
void thmain_cleanup(void *arg);
// 服务端主进程退出时的清理函数，要关listen socket，并给所有子进程发信号让其退出
void father_exit(int sig);
// 服务端进行通信的子进程退出时的清理函数，要关socket
void son_exit(int sig);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo05 port\n");
		printf("example: ./demo05 8888\n");
		return -1;
	}
	signal(SIGCHLD, SIG_IGN);
	signal(2, father_exit);
	signal(15, father_exit);

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
	while(true)
	{
		connfd = accept(listen_fd, NULL, NULL);
		if(fork() == 0)
		{
			// son
			signal(2, son_exit);
			signal(15, son_exit);
			close(listen_fd);
			int ret = -1;
			char buf[512];
			memset(buf, 0, sizeof(buf));
			while(true)
			{
				ret = recv(connfd, buf, 512, 0);
				if(ret <= 0)
				{
					break;
				}
				printf("recv from client[%d]: ==%s==\n", connfd, buf);
				ret = send(connfd, buf, 512, 0);
				if(ret <= 0)
				{
					break;
				}
				printf("send to client[%d]: ==%s==\n", connfd, buf);
			}
			printf("客户端[%d]已断开连接。。。\n", connfd);
			close(connfd);
			printf("进程[%d]已退出。。。\n", getpid());
			exit(0);
		}
		else
		{
			//father
			close(connfd);
		}
	}
	return 0;
}
void father_exit(int sig)
{
	// 屏蔽信号，防止信号处理函数嵌套
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);

	close(listen_fd);

	printf("father process[%d] receive sig[%d], killing all son process...\n", getpid(), sig);
	kill(0, 15);
	sleep(2);
	printf("father process[%d] exit..., already killed all son process...\n", getpid());
	exit(0);
}
void son_exit(int sig)
{
	// 屏蔽信号，防止信号处理函数嵌套
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);

	close(connfd);

	printf("process[%d] receive sig[%d] and exit...\n", getpid(), sig);
	exit(0);
}
