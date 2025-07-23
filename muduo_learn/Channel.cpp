#include <iostream>
#include <poll.h>
#include <sys/poll.h>
#include <unistd.h>
#include "Channel.h"
#include "Eventloop.h"
#include <assert.h>
Channel::Channel(Eventloop *loop, int fd)
	: m_loop(loop)
	, m_event(0)
	, m_fd(fd)
	, m_revents(0)
{
}
Channel::~Channel()
{
	remove();
	close(m_fd);
}
void Channel::handleEvent()
{
	 // channel对事件的处理由上层调用如acceptor或者coonnector
	 if(POLLIN & m_revents)
	 {
	 	assert(m_readCallback);
	 	m_readCallback();
	 }
	 if(POLLRDHUP & m_revents || POLLHUP & m_revents)
	 {
	 	assert(m_closeCallback);
	 	m_closeCallback();
	 }
}
void Channel::remove()
{
	m_loop->removeChannel(this);
}
void Channel::registerToLoop()
{
	m_loop->registerChannel(this);
}
void Channel::enableReading()
{
	m_event |= POLLIN;
	registerToLoop();
}
void Channel::enableClose()
{
	m_event |= POLLRDHUP;
	m_event |= POLLHUP;
	registerToLoop();
}
