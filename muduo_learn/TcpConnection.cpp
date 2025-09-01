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
	m_channel.setErrorCallback(std::bind(&TcpConnection::handleError, this));
}
TcpConnection::~TcpConnection()
{
	// m_channel.~Channel();
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
	else if(rsize == 0)
	{
		// 对端出错就关闭本端socket并退出
		handleClose();
	}
	else 
	{
		// close handle
		handleError();

	}

}
void TcpConnection::handleClose()
{
	assert(m_CloseCallback != nullptr);
	printf("TcpConnection::handleClose connection close...\n");
	m_channel.remove();
	m_CloseCallback(this);
}
void TcpConnection::handleError()
{
	assert(m_ErrorCallback != nullptr);
	printf("TcpConnection::handleError connection close...\n");
	m_channel.remove();
	m_ErrorCallback(this);
}
void TcpConnection::init()
{
	assert(m_ConnectionCallback);
	m_channel.enableReading();
	m_channel.registerToLoop();
	// fixme: 这个连接回调在任何线程都可以运行
	m_ConnectionCallback(m_fd);
}
