#include "../TcpConnection.h"
#include "../TcpServer.h"
#include "../Eventloop.h"
#include "../Acceptor.h"
#include <functional>
#include <string.h>
#include <unistd.h>


void connectionCb(int fd)
{
	printf("TcpConnection::connectionCb=server new connection, fd=%d\n", fd);
}
void messageCb(int fd, char *buf, int len)
{
	printf("recv mes[%s]\n", buf);
}
void closeCb(TcpConnection *conn)
{
	printf("socket[%d] closed...\n", conn->getFd());
	close(conn->getFd());
}
void errorCb(TcpConnection *conn)
{
	printf("socket[%d] error...\n", conn->getFd());
	close(conn->getFd());
}
int main(int argc, char* argv[])
{
	Eventloop el;
	TcpServer server(&el, "127.0.0.1", 8888);
	server.setMessageCallback(std::bind(messageCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server.setCloseCallback(std::bind(closeCb, std::placeholders::_1));
	server.setErrorCallback(std::bind(errorCb, std::placeholders::_1));
	server.setConnectionCallback(std::bind(connectionCb, std::placeholders::_1));
	server.init();
	el.loop();

	return 0;
}
