#ifndef __TCPCONNECTION__H__
#define __TCPCONNECTION__H__
#include <functional>
#include <memory>
#include "Channel.h"
#include "stdio.h"
// connector 和对端进行通信，处理四个事情，刚连接时的处理，接收到消息时的处理，错误的处理以及当对端关闭时的处理
// 注：使用方法 指定四个回调m_ConnectionCallback, m_MessageCallback, m_closeCallback, m_errorCallback后再调用init
// ps：init方法主要是channel enableread enableclose enableerror和注册channel到loop
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
private:
	int m_fd;
	Eventloop *m_loop;
	char m_receiveBuf[1000];
	char m_sendBuf[1000];
	Channel m_channel;
	// 注：使用TcpConnection时必须指定m_ConnectionCallback, m_MessageCallback, m_CloseCallback
	// m_ConnectionCallback在刚创建连接时调用
	std::function<void(int)> m_ConnectionCallback;
	// m_MessageCallback在接收到消息时调用
	std::function<void(int, char *, int)> m_MessageCallback;
	// m_CloseCallback在对端关闭socket后调用
	std::function<void(TcpConnection *)> m_CloseCallback;
	// POLLERR | POLLNVAL时候调用
	std::function<void(TcpConnection *)> m_ErrorCallback;
public:
	TcpConnection(Eventloop *loop, int fd);
	~TcpConnection();
	void setConnectionCallback(std::function<void(int)> cb)
	{
		m_ConnectionCallback = cb;
	}
	void setMessageCallback(std::function<void(int, char *, int)> cb)
	{
		m_MessageCallback = cb;
	}
	void setCloseCallback(std::function<void(TcpConnection *)> cb)
	{
		m_CloseCallback = cb;
	}
	void setErrorCallback(std::function<void(TcpConnection *)> cb)
	{
		m_ErrorCallback = cb;
	}
	// 处理接收消息，以及处理对端关闭即recv返回值为0的情况
	void handleRead();
	// 设置m_channel的closeCallback
	// 注：本函数负责调用m_closeCallback(int fd)和将m_channel从loop中开除,并不负责关闭fd，这个职责应该由用户调用负责，这属于业务，不属于框架
	void handleClose();
	void handleError();
	// 设置m_channel enableread enableclose并注册m_channel到loop,最后调用m_ConnectionCallback
	void init();
	int getFd(){return m_channel.getFd();}
};
#endif
