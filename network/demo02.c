/*****************************************************************************
 * 程序名：demo02.c
 * 作者：jiebei
 * 功能：这是一个网络通信客户端，它连接一个服务端后可以由终端向服务端发消息，并接收服务端的消息。Ctrl-c或kill可以终止该程序
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

int sockfd = -1;

void cleanup(int sig)
{
	if(sockfd != -1) close(sockfd);
	printf("client exit...\n");
	exit(0);
}
int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("Usage: ./demo02 ip port\n");
		printf("example: ./demo02 127.0.0.1 8888\n");
		return -1;
	}
	signal(2, cleanup);
	signal(15, cleanup);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("cilent socket() failed!\nerrno[%d]-[%s]\n", errno, strerror(errno));
		return -1;
	}
	struct sockaddr_in skad;
	memset(&skad, 0, sizeof(skad));
	skad.sin_family = AF_INET;
	skad.sin_addr.s_addr = inet_addr(argv[1]);
	skad.sin_port = htons((in_port_t)atoi(argv[2]));


	if(connect(sockfd, (struct sockaddr *)&skad, sizeof(skad)) == -1)
	{
		printf("cilent connect() failed!\nerrno[%d]-[%s]\n", errno, strerror(errno));
		close(sockfd);
		return -1;

	}
	char buf[512];

	while(1)
	{
		memset(buf, 0, sizeof(buf));
		scanf("%[^\n]%*c", buf);
		// write(sockfd, buf, 512);
		send(sockfd, buf, 512, 0);
		// read(sockfd, buf, 512);
		recv(sockfd, buf, 512, 0);
		printf("recv: ===%s===\n", buf);
	}
	close(sockfd);
	return 0;
}
