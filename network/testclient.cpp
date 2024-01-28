#include "libsocket.h"
int main(int argc, char* argv[])
{
	int clientfd = init_client("127.0.0.1", 8888);
	if(clientfd == -1)
	{
		exit(-1);
	}
	char sendbuf[1600];
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "there're totally 30 bytes.....");
	char recvbuf[1600];
	memset(recvbuf, 0, sizeof(recvbuf));
	for(int ii = 0; ii < 2000; ii++)
	{
		printf("send: %s\n", sendbuf);
		/*
		int rsize = send(clientfd, sendbuf, strlen(sendbuf), 0);
		if(rsize < 0)
		{
			close(clientfd);
			exit(-1);
		}
		if(rsize == 0)
		{
			printf("server closed...\n");
			close(clientfd);
			exit(-1);
		}
		*/
		if(tcp_send(clientfd, sendbuf, strlen(sendbuf)) == false)
		{
			close(clientfd);
			exit(-1);
		}
		// printf("recv: %s\n", buf);
	}
	return 0;
}
