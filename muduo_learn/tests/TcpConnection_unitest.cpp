#include "../TcpConnection.h"
#include "../Eventloop.h"
#include "../Acceptor.h"
#include <functional>
#include <string.h>
#include <unistd.h>

TcpConnection *tcpConnection;
Eventloop el;
void onConnection(int fd)
{
	printf("Connection Callback...\n");
}
void onMessage(int fd, char *buf, int len)
{
	char tmp[len+1];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, buf, len);
	printf("recv==%s==\n", tmp);
}
// 在TcpConnection class设计中，会先调用用户CloseCallback在将channel开除
void onClose(int fd)
{
	printf("client[%d] disconnected...\n", fd);
	close(fd);
}
void newConnection(int fd, struct sockaddr *client, socklen_t *len)
{
	tcpConnection = new TcpConnection(&el, fd);
	tcpConnection->setConnectionCallback(std::bind(onConnection, std::placeholders::_1));
	tcpConnection->setCloseCallback(std::bind(onClose, std::placeholders::_1));
	tcpConnection->setMessageCallback(std::bind(onMessage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	tcpConnection->init();
}
int main(int argc, char* argv[])
{
	Acceptor acceptor(&el, "127.0.0.1", 8888);
	acceptor.setNewConnectionCallback(std::bind(newConnection, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	acceptor.listen();
	el.loop();

	return 0;
}
