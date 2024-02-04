/*****************************************************************************
 * program: demo03.cpp
 * author: jiebei
 * 功能：使用管道实现父子进程通信，父进程通过管道向子进程发送100条消息，子进程读取后打印出来。
 * Usage: ./demo03
 ****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int fd[2];
int main(int argc, char* argv[])
{
	fd[0] = fd[1] = -1;
	if(pipe(fd) == -1)
	{
		printf("pipe() failed...\n");
		printf("errno[%d] info[%s]\n", errno, strerror(errno));
		exit(-1);
	}
	char buf[100];
	memset(buf, 0, sizeof(buf));
	if(fork())
	{
		signal(SIGCHLD, SIG_IGN);
		close(fd[0]);
		fd[0] = -1;
		// father
		for(int ii = 0; ii < 100; ii++)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "this %02d msg from father", ii);
			write(fd[1], buf, strlen(buf)+1);
		}
		
	}
	else
	{
		// son
		close(fd[1]);
		fd[1] = -1;
		for(int ii = 0; ii < 100; ii++)
		{
			memset(buf, 0, sizeof(buf));
			int rrize = read(fd[0], buf, 24);
			printf("son recv: %s\n", buf);
		}
	}
	if(~fd[0])
	{
		close(fd[0]);
	}
	if(~fd[1])
	{
		close(fd[1]);
	}
	return 0;
}
