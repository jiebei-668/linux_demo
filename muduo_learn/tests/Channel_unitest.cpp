#include "../Channel.h"
#include "../Eventloop.h"
#include <assert.h>
#include <sys/poll.h>

void readCallback()
{
	printf("this is readCallback...\n");
}
void closeCallback()
{
	printf("this is closeCallback...\n");
}
int main(int argc, char* argv[])
{
	Eventloop el;
	Channel cnl(&el, 1);
	cnl.setCloseCallback(std::bind(closeCallback));
	cnl.setReadCallback(std::bind(readCallback));
	cnl.setRevents(POLLIN);
	assert(cnl.getFd() == 1);
	cnl.handleEvent();
	cnl.setRevents(POLLHUP);
	assert(cnl.getFd() == 1);
	cnl.handleEvent();
	return 0;
}
