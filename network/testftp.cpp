#include "libsocket.h"
#include <signal.h>
#include <sys/poll.h>

int cmdfd = -1;
int listenfd = -1;
int datafd = -1;
int maxfd = -1;
void exit_fun(int sig);
int main(int argc, char* argv[])
{
	cmdfd = init_client_and_connect("127.0.0.1", 21);
	if(cmdfd == -1)
	{
		exit_fun(-1);
	}
	maxfd = cmdfd;
	signal(2, exit_fun);
	signal(15, exit_fun);
	char buf[200];
	memset(buf, 0, sizeof(buf));
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	//sprintf(buf, "Conne");
	//send(cmdfd, buf, strlen(buf), 0);
//				memset(buf, 0, sizeof(buf));
//				recv(cmdfd, buf, sizeof(buf), 0);
//				printf("%s", buf);
//				fflush(stdout);
//	memset(buf, 0, sizeof(buf));
//	sprintf(buf, "USER jiebei\r\n");
//	send(cmdfd, buf, strlen(buf), 0);
//				memset(buf, 0, sizeof(buf));
//				recv(cmdfd, buf, sizeof(buf), 0);
//				printf("%s", buf);
//				fflush(stdout);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "USER jiebei\r\n");
	send(cmdfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "PASS 668668\r\n");
	send(cmdfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "NOOP\r\n");
	send(cmdfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "TYPE A\r\n");
	send(cmdfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "PWD\r\n");
	send(cmdfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "STOR testaaa.h\r\n");
	send(cmdfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
	listenfd = init_server("127.0.0.1", 6666);
	maxfd = maxfd > listenfd ? maxfd : listenfd;
	set_reuseaddr(listenfd);
	set_nonblock(listenfd);
	struct pollfd fds[1024];
	for(int ii = 0; ii < 1024; ii++)
	{
		fds[ii].fd = -1;
	}
	fds[listenfd].fd = listenfd;
	fds[listenfd].events = POLLIN;
	fds[cmdfd].fd = cmdfd;
	fds[cmdfd].events = POLLIN;
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	//memset(buf, 0, sizeof(buf));
	//sprintf(buf, "PORT 127,0,0,1,26,10\r\n");
	//send(cmdfd, buf, strlen(buf), 0);
	//			memset(buf, 0, sizeof(buf));
	//			recv(cmdfd, buf, sizeof(buf), 0);
	//			printf("%s", buf);
	//			fflush(stdout);
	while(true)
	{

		int eventnum = poll(fds, maxfd+1, -1);
		for(int fd = 0; fd <= maxfd; fd++)
		{
			if(fds[fd].fd == -1 || (fds[fd].events&POLLIN) == 0)
			{
				continue;
			}
			if(fd == listenfd)
			{
				int datafd = accept(listenfd, NULL, NULL);
				fds[datafd].fd = datafd;
				fds[datafd].events = POLLIN;
				continue;
			}
			if(fd == cmdfd)
			{
				memset(buf, 0, sizeof(buf));
				recv(cmdfd, buf, sizeof(buf), 0);
				printf("%s", buf);
				fflush(stdout);
				continue;
			}
			if(fd == 0)
			{
				memset(buf, 0, sizeof(buf));
				read(0, buf, sizeof(buf));
				send(cmdfd, buf, strlen(buf), 0);
				continue;
			}
			if(fd == datafd)
			{
				memset(buf, 0, sizeof(buf));
				recv(datafd, buf, sizeof(buf), 0);
				printf("data:%s", buf);
				fflush(stdout);
				continue;

			}
		}

	}
	return 0;
}
void exit_fun(int sig)
{
	if(cmdfd != -1)
	{
		close(cmdfd);
	}
	if(listenfd != -1)
	{
		close(listenfd);
	}
	if(datafd != -1)
	{
		close(datafd);
	}
	exit(-1);
}
