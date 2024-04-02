#include "../TcpConnection.h"
#include "../TcpServer.h"
#include "../Eventloop.h"
#include "../Acceptor.h"
#include <functional>
#include <string.h>
#include <unistd.h>

void onConnection(int fd)
{
	printf("Client[%d] connect to server...\n", fd);
}
void onMessage(int fd, char *buf, int len)
{
	char tmp[len+1];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, buf, len);
	printf("recv from client[%d]==%s==\n", fd, tmp);
}
void onClose(int fd)
{
	printf("client[%d] closed...\n", fd);
	::close(fd);
}
int main(int argc, char* argv[])
{
	Eventloop el;
	TcpServer server(&el, "127.0.0.1", 8888);
	server.setMessageCallback(std::bind(onMessage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server.setCloseCallback(std::bind(onClose, std::placeholders::_1));
	server.setConnectionCallback(std::bind(onConnection, std::placeholders::_1));
	server.init();
	el.loop();

	return 0;
}
