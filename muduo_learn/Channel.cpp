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
	// socket编程中，走不到这个条件，当对端处于发送fin又收到ack处于FIN_WAIT_2时，本端有recv返回值为0,就会关闭socket，不会等四次握手后受到pollhup
	if(m_revents & POLLHUP && !(m_revents & POLLIN))
	{
		assert(m_closeCallback);
		m_closeCallback();
		return;
	}
	// 编程得当的话pollnval是不会出现的
	if(m_revents & (POLLERR | POLLNVAL))
	{
		assert(m_errorCallback);
		m_errorCallback();
		return;
	}
	 if((POLLIN | POLLPRI) & m_revents)
	 {
	 	assert(m_readCallback);
		// NOTE 上层如tcpconnection指定channel的readcallback时应该特别处理recv返回值为0的情况！
	 	m_readCallback();
		return;
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
