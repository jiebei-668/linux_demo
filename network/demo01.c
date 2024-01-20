/*****************************************************************************
 * 程序名：demo01.c
 * 作者：jiebei
 * 功能：这是一个网络通信服务端，它接受一个客户端连接后将接受到的来自客户端的消息原封不动发送回去。Ctrl-c或kill可以终止该程序
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define MAX_BACKLOG  512

int listen_sock = -1;
int client_sock = -1;


void cleanup(int sig)
{
	if(listen_sock != -1) close(listen_sock);
	if(client_sock != -1) close(client_sock);
	printf("server exit...\n");
	exit(0);
}
int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo01 port\n");
		printf("example: ./demo01 8888\n");
		return -1;
	}
	signal(2, cleanup);
	signal(15, cleanup);
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0)
	{
		printf("server socket() failed!\nerrno[%d]-[%s]\n", errno, strerror(errno));
		return -1;
	}
	struct sockaddr_in skad;
	memset(&skad, 0, sizeof(skad));
	skad.sin_family = AF_INET;
	skad.sin_addr.s_addr = inet_addr("127.0.0.1");
	skad.sin_port = htons((in_port_t)atoi(argv[1]));


	if(bind(listen_sock, (struct sockaddr *)&skad, sizeof(skad)) == -1)
	{
		printf("server bind() failed!\nerrno[%d]-[%s]\n", errno, strerror(errno));
		close(listen_sock);
		return -1;
	}

	if(listen(listen_sock, MAX_BACKLOG) == -1)
	{
		printf("server listen() failed!\nerrno[%d]-[%s]\n", errno, strerror(errno));
		close(listen_sock);
		return -1;
	}
	client_sock = accept(listen_sock, NULL, NULL);
	if(client_sock == -1)
	{
		printf("server accept() failed!\nerrno[%d]-[%s]\n", errno, strerror(errno));
		close(listen_sock);
		return -1;
	}
	char buf[512];
	memset(buf, 0, sizeof(buf));
	int len;
	// while((len = read(client_sock, buf, 512)) != 0)
	while((len = recv(client_sock, buf, 512, 0)) != 0)
	{
		printf("recv: ==%s==\n", buf);
		// write(client_sock, buf, len);
		send(client_sock, buf, len, 0);
		printf("write: ===%s===\n", buf);
		memset(buf, 0, sizeof(buf));
	}
	close(listen_sock);
	close(client_sock);
	return 0;
}
