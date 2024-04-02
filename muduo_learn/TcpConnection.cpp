#include "TcpConnection.h"
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>


TcpConnection::TcpConnection(Eventloop *loop, int fd)
	: m_fd(fd)
	, m_loop(loop)
	, m_channel(loop, fd)
{
	m_channel.setReadCallback(std::bind(&TcpConnection::handleRead, this));
	m_channel.setCloseCallback(std::bind(&TcpConnection::handleClose, this));
}
TcpConnection::~TcpConnection()
{
}
void TcpConnection::handleRead()
{
	assert(m_MessageCallback != nullptr);
	memset(m_receiveBuf, 0, sizeof m_receiveBuf);
	int rsize = ::recv(m_fd, m_receiveBuf, sizeof m_receiveBuf, 0);
	if(rsize > 0)
	{
		m_MessageCallback(m_fd, m_receiveBuf, rsize);
	}
	else if(rsize < 0)
	{
		// error handle
	}
	else 
	{
		// close handle
		handleClose();

	}

}
void TcpConnection::handleClose()
{
	assert(m_CloseCallback != nullptr);
	m_CloseCallback(m_fd);
	m_channel.remove();
}
void TcpConnection::init()
{
	assert(m_ConnectionCallback);
	m_channel.enableReading();
	m_channel.registerToLoop();
	m_ConnectionCallback(m_fd);
}
