#ifndef __ACCEPTOR__H__
#define __ACCEPTOR__H__
#include <memory>
#include <functional>
#include <sys/types.h>
#include <netinet/in.h>
#include "Channel.h"
#include "stdio.h"
#include <string>
class Eventloop;
// accept是管理listenfd的那个channel的管理者,作用是监听并接收连接，接收到以后初始化connector
// 使用方法：指定m_newConnectionCallback后，再listen（）
// ps: Acceptor::listen() 一方面是channel->enableread，一方面listen
// fixme 服务器acceptor尚没有做主动关闭或者退出的处理
class Acceptor
{
private:
	int m_listenfd;
	std::string m_ip;
	int m_port;
	Eventloop* m_loop;
	std::unique_ptr<Channel> m_channel;
	// 注：使用Acceptor class时必须指定m_newConnectionCallback
	std::function<void(int, struct sockaddr *, socklen_t *)> m_newConnectionCallback;
	void handleRead();
public:
	Acceptor(Eventloop *loop, const char *ip, const int port);
	~Acceptor();
	void setNewConnectionCallback(std::function<void(int, struct sockaddr *, socklen_t *)> cb)
	{
		m_newConnectionCallback = cb;
	}
	int listen();
};
#endif
