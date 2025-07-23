#include "../Channel.h"
#include "../Eventloop.h"
#include <assert.h>
#include <sys/poll.h>
#include "stdio.h"
#include "../header/libsocket.h"

void readCallback()
{
	printf("this is readCallback...\n");
}
void closeCallback()
{
	printf("this is closeCallback...\n");
}
void readCallback1(int fd)
{
	int client_fd = accept(fd, NULL, NULL);	
	if(client_fd == -1)
	{
		printf("accept fail!\n");
		return;
	}
	printf("accept fd[%d]\n", client_fd);
	close(client_fd);
	printf("close fd[%d]\n", client_fd);
}
void closeCallback1(int fd)
{
}
int main(int argc, char* argv[])
{
	// 测试handleEvent()
	// Eventloop el;
	// Channel cnl(&el, 1);
	// cnl.setCloseCallback(std::bind(closeCallback));
	// cnl.setReadCallback(std::bind(readCallback));
	// cnl.setRevents(POLLIN);
	// assert(cnl.getFd() == 1);
	// cnl.handleEvent();
	// cnl.setRevents(POLLHUP);
	// assert(cnl.getFd() == 1);
	// cnl.handleEvent();
	
	int fd = init_server("127.0.0.1", 9999);
	if( fd == -1)
	{
		printf("init_server() failed!...\n");
		exit(-1);
	}
	Eventloop el;
	Channel cnl(&el, fd);
	cnl.setReadCallback(std::bind(readCallback1, fd));
	cnl.setRevents(POLLIN);
	cnl.handleEvent();
	return 0;
}
