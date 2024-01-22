/*****************************************************************************
 * 程序名：demo03.cpp
 * 作者：jiebei
 * 功能：这是一个多线程网络通信服务端，它接受客户端连接后将接受到的来自客户端的消息原封不动发送回去。Ctrl-c或kill可以终止该程序
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

#define MAX_BACKLOG 512

int listen_fd = -1;
// 存放所有连接的客户端socketid的容器
std::vector<int> v_connfd;
// 存放所有与客户端通信的子线程的线程id
std::vector<pthread_t> v_thid;
// 和客户端进行通信的线程函数
void *thmain(void *);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo03 port\n");
		printf("example: ./demo03 8888\n");
		return -1;
	}
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
	int connfd = -1;
	while(true)
	{
		connfd = accept(listen_fd, NULL, NULL);
		if(connfd == -1)
		{
			continue;
		}
		pthread_t id;
		pthread_create(&id, NULL, thmain, (void *)(long)connfd);
	}
	return 0;
}
void thmain_exit(int connfd)
{
	printf("客户端[%d]已断开连接。。。\n", connfd);
	close(connfd);
	for(int ii = 0; ii < v_connfd.size(); ii++)
	{
		if(v_connfd[ii] == connfd)
		{
			v_connfd[ii] = 0;
			break;

		}
	}
	exit(0);
}
// @param: 强转为void *类型的连接的客户端的sockid
void *thmain(void *arg)
{
	int connfd = (int)(long)arg;
	int ret = -1;
	char buf[512];
	memset(buf, 0, sizeof(buf));
	while(true)
	{
		ret = recv(connfd, buf, 512, 0);
		if(ret == -1)
		{
			thmain_exit(connfd);
		}
		printf("recv from client[%d]: ==%s==\n", connfd, buf);
		ret = send(connfd, buf, 512, 0);
		if(ret == -1)
		{
			thmain_exit(connfd);
		}
		printf("send to client[%d]: ==%s==\n", connfd, buf);
	}

	return NULL;
}
