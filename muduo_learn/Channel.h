#ifndef __CHANNEL__H__
#define __CHANNEL__H__
#include <functional>
class Eventloop;
// channel类的作用就是将事件类型和对应的处理函数绑定
class Channel
{
private:
	int m_fd;
	std::function<void()> m_closeCallback;
	std::function<void()> m_readCallback;
	int m_revents;
	int m_event;
	Eventloop *m_loop;
public:
	Channel(Eventloop *loop, int fd);
	~Channel();
	void handleEvent();
	void remove();
	void registerToLoop();
	int getFd()
	{
		return m_fd;
	}
	Eventloop *getLoop()
	{
		return m_loop;
	}
	void setRevents(int revents)
	{
		m_revents = revents;
	}
	void setCloseCallback(std::function<void()> cb)
	{
		m_closeCallback = cb;
	}
	void setReadCallback(std::function<void()> cb)
	{
		m_readCallback = cb;
	}
	// 对应POLLIN事件
	void enableReading();
	// fixme 对应什么事件？
	// 使用pollin recv返回值是否为0来判断对端关闭，所以这个函数暂时没用
	void enableClose();
	int getEvent()
	{
		return m_event;
	}
};
#endif
