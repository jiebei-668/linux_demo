#include "../header/libsocket.h"
#include "../Acceptor.h"
#include "../TcpConnection.h"
#include "../Eventloop.h"

// 这是一个基本的服务器，使用Acceptor监听并接受连接，使用TcpConnection处理业务
std::vector<TcpConnection *> conns;
void connectionCb(int fd)
{
	printf("TcpConnection::connectionCb=server new connection, fd=%d\n", fd);
}
void messageCb(int fd, char *buf, int len)
{
	printf("recv mes[%s]\n", buf);
}
void closeCb(int fd)
{
	printf("socket[%d] closed...\n", fd);
	close(fd);
}
void myAccept(Eventloop *el, int fd, struct sockaddr *addr, socklen_t *addr_len)
{
	printf("Accept accept connection..\n");
	TcpConnection *conn = new TcpConnection(el, fd);
	conns.emplace_back(conn);
	conn->setConnectionCallback(std::bind(connectionCb, std::placeholders::_1));
	conn->setMessageCallback(std::bind(messageCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	conn->setCloseCallback(std::bind(closeCb, std::placeholders::_1));
	conn->init();
}
int main(int argc, char* argv[])
{
	Eventloop el;
	Acceptor acceptor = Acceptor(&el, "127.0.0.1", 8888);
	acceptor.setNewConnectionCallback(std::bind(myAccept, &el, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	acceptor.listen();
	el.loop();


	for(auto &conn: conns)
	{
		conn->~TcpConnection();
	}
	return 0;
}
