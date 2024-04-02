#ifndef __CHANNEL__H__
#define __CHANNEL__H__
#include <functional>
class Eventloop;
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
	void enableReading();
	void enableClose();
	int getEvent()
	{
		return m_event;
	}
};
#endif
