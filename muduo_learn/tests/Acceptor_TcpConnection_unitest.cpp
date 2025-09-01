#include "../header/libsocket.h"
#include "../Acceptor.h"
#include "../TcpConnection.h"
#include "../Eventloop.h"

// 这是一个基本的服务器，使用Acceptor监听并接受连接，使用TcpConnection处理业务
// 刚链接上acceptor打印 Accept accept connection; tcpconnection打印 "TcpConnection::connectionCb=server new connection, fd=%d\n"
// 有消息时打印出消息内容
// 对端关闭时打印 socket[fd] closed 并close 
// 错误时打印 socket[fd] error 并close
std::vector<TcpConnection *> conns;
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
void myAccept(Eventloop *el, int fd, struct sockaddr *addr, socklen_t *addr_len)
{
	printf("Accept accept connection..\n");
	TcpConnection *conn = new TcpConnection(el, fd);
	conns.emplace_back(conn);
	conn->setConnectionCallback(std::bind(connectionCb, std::placeholders::_1));
	conn->setMessageCallback(std::bind(messageCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	conn->setErrorCallback(std::bind(errorCb, conn));
	conn->setCloseCallback(std::bind(closeCb, conn));
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
