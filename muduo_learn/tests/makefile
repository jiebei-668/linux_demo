all: Channel_unitest Eventloop_unitest Acceptor_unitest TcpConnection_unitest TcpServer_unitest Acceptor_TcpConnection_unitest
Channel_unitest: Channel_unitest.cpp ../Channel.h ../Channel.cpp ../Eventloop.h ../Eventloop.cpp
	g++ -g -o Channel_unitest Channel_unitest.cpp ../Channel.cpp ../Eventloop.cpp -L../lib/ -lsocket
Eventloop_unitest: Eventloop_unitest.cpp ../Channel.h ../Channel.cpp ../Eventloop.h ../Eventloop.cpp
	g++ -g -o Eventloop_unitest Eventloop_unitest.cpp ../Channel.cpp ../Eventloop.cpp -L../lib/ -lsocket
Acceptor_unitest: Acceptor_unitest.cpp ../Channel.h ../Channel.cpp ../Eventloop.h ../Eventloop.cpp ../Acceptor.h ../Acceptor.cpp
	g++ -g -o Acceptor_unitest Acceptor_unitest.cpp ../Channel.cpp ../Eventloop.cpp ../Acceptor.cpp
TcpConnection_unitest: TcpConnection_unitest.cpp ../Channel.h ../Channel.cpp ../Eventloop.h ../Eventloop.cpp ../Acceptor.h ../Acceptor.cpp ../TcpConnection.h ../TcpConnection.cpp
	g++ -g -o TcpConnection_unitest TcpConnection_unitest.cpp ../Channel.cpp ../Eventloop.cpp ../Acceptor.cpp ../TcpConnection.cpp
TcpServer_unitest: TcpServer_unitest.cpp ../Channel.h ../Channel.cpp ../Eventloop.h ../Eventloop.cpp ../Acceptor.h ../Acceptor.cpp ../TcpConnection.h ../TcpConnection.cpp ../TcpServer.h ../TcpServer.cpp
	g++ -g -o TcpServer_unitest TcpServer_unitest.cpp ../Channel.cpp ../Eventloop.cpp ../Acceptor.cpp ../TcpConnection.cpp ../TcpServer.cpp
Acceptor_TcpConnection_unitest: Acceptor_TcpConnection_unitest.cpp ../Channel.h ../Channel.cpp ../Eventloop.h ../Eventloop.cpp ../Acceptor.h ../Acceptor.cpp ../TcpConnection.h ../TcpConnection.cpp
	g++ -g -o Acceptor_TcpConnection_unitest Acceptor_TcpConnection_unitest.cpp ../Channel.cpp ../Eventloop.cpp ../Acceptor.cpp ../TcpConnection.cpp
clean:
	rm -rf Channel_unitest Eventloop_unitest Acceptor_unitest TcpConnection_unitest TcpServer_unitest Acceptor_TcpConnection_unitest
