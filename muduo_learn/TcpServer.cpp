#include "TcpServer.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include <cassert>
TcpServer::TcpServer(Eventloop *loop, const char *ip, const int port)
	: m_loop(loop)
	, m_acceptor(nullptr)
{
	m_acceptor.reset(new Acceptor(loop, ip, port));
	// FIXME 
	m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
TcpServer::~TcpServer()
{
}
void TcpServer::init()
{
	assert(m_acceptor != nullptr);
	m_acceptor->listen();
}
void TcpServer::newConnection(int fd, struct sockaddr *client, socklen_t *len)
{
	assert(m_connectionCallback != nullptr);
	// FIXME： m_loop 和 Tcpserver构造函数的loop参数是两个loop
	TcpConnection *tcpConnection = new TcpConnection(m_loop, fd);
	m_connectionMap[tcpConnection] = tcpConnection;
	tcpConnection->setConnectionCallback(m_connectionCallback);
	tcpConnection->setCloseCallback(m_closeCallback);
	tcpConnection->setMessageCallback(m_messageCallback);
	tcpConnection->init();
}
