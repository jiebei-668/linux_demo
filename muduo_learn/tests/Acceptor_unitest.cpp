// 需要配合一个客户端来测试，客户端连发起连接，测试Acceptor能否正确运行
#include "../Acceptor.h"
#include "../Eventloop.h"
#include <functional>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void onConnect(int connfd, struct sockaddr *client, socklen_t *len)
{
	printf("test Aceptor ok...\n");
	::close(connfd);
}
int main(int argc, char* argv[])
{

	Eventloop el;
	Acceptor apt(&el, "127.0.0.1", 8888);
	apt.setNewConnectionCallback(std::bind(onConnect, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	apt.listen();
	el.loop();

	return 0;
}
