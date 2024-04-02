#ifndef __TCPCONNECTION__H__
#define __TCPCONNECTION__H__
#include <functional>
#include <memory>
#include "Channel.h"
// 注：使用方法 指定三个回调m_ConnectionCallback, m_MessageCallback, m_closeCallback后再调用init
// ps：init方法主要是channel enableread enableclose和注册channel到loop
class TcpConnection
{
private:
	int m_fd;
	Eventloop *m_loop;
	char m_receiveBuf[1000];
	char m_sendBuf[1000];
	Channel m_channel;
	// 注：使用TcpConnection时必须指定m_ConnectionCallback, m_MessageCallback, m_CloseCallback
	// m_ConnectionCallback在accept到一个新连接后调用
	std::function<void(int)> m_ConnectionCallback;
	// m_MessageCallback在接收到消息时调用
	std::function<void(int, char *, int)> m_MessageCallback;
	// m_CloseCallback在对端关闭socket后调用
	std::function<void(int)> m_CloseCallback;
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
	void setCloseCallback(std::function<void(int)> cb)
	{
		m_CloseCallback = cb;
	}
	// 设置m_channel的readCallback
	void handleRead();
	// 设置m_channel的closeCallback
	// 注：本函数负责调用m_closeCallback(int fd)和将m_channel从loop中开除,并不负责关闭fd，这个职责应该由用户调用负责，这属于业务，不属于框架
	void handleClose();
	// 设置m_channel enableread enableclose并注册m_channel到loop,最后调用m_ConnectionCallback
	void init();
};
#endif
