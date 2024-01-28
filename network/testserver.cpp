#include "libsocket.h"
#include <unistd.h>

int main(int argc, char* argv[])
{
	int listenfd = init_server("127.0.0.1", 8888);
	if(listenfd == -1)
	{
		exit(-1);
	}
	int connfd = accept(listenfd, NULL, NULL);
	char buf[1600];
	for(int ii = 0; ii < 2000; ii++)
	{
		memset(buf, 0, sizeof(buf));
		/*
		int rsize = recv(connfd, buf, sizeof(buf), 0);
		if(rsize == 0)
		{
			printf("client closed...\n");
			close(connfd);
			close(listenfd);
			exit(-1);
		}
		if(rsize < 0)
		{
			printf("recv() failed...\n");
			close(connfd);
			close(listenfd);
			exit(-1);
		}
		*/
		if(tcp_recv(connfd, buf, 1600) == false)
		{
			close(connfd);
			close(listenfd);
			exit(-1);
		}
		printf("%s\n", buf);
		//printf("%d\n", rsize);
		//usleep(100);
	}
	close(listenfd);
	close(connfd);
	return 0;
}
