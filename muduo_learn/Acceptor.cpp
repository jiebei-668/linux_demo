#include "Acceptor.h"
#include "Channel.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

Acceptor::Acceptor(Eventloop *loop, const char *ip, const int port)
	: m_listenfd(-1)
	, m_loop(loop)
	, m_port(port)
	, m_ip(ip)
	, m_channel(nullptr)
{
	m_listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_listenfd == -1)
	{
		// FIXME
		// error handle
		exit(-1);
	}
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(m_ip.c_str());
	server.sin_port = htons(m_port);
	int opt = 1;
	setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
	if(bind(m_listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		close(m_listenfd);
		// FIXME
		// error handle
		exit(-1);
	}
	// FIXME 使用非阻塞socket
	m_channel.reset(new Channel(m_loop, m_listenfd));
	m_channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
	m_channel->enableReading();
	
	
}
Acceptor::~Acceptor()
{
}
void Acceptor::handleRead()
{
	struct sockaddr_in client;
	socklen_t len = sizeof(struct sockaddr_in);
	int connfd = ::accept(m_listenfd, (struct sockaddr *)&client, &len);
	if(connfd >= 0)
	{
		if(m_newConnectionCallback)
		{
			m_newConnectionCallback(connfd, (struct sockaddr *)&client, &len);
		}
		else
		{
			printf("Acceptor::handler m_newConnectionCallback is null...\n");
			::close(connfd);
		}
	}
	else
	{
		printf("Acceptor::handleRead() accept() failed...\nerrno: %d[%s]\n", errno, strerror(errno));
	}
}
int Acceptor::listen()
{
	m_channel->enableReading();
	return ::listen(m_listenfd, 10);
}
