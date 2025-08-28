#ifndef __CHANNEL__H__
#define __CHANNEL__H__
#include <functional>
class Eventloop;
// channel类的作用就是将事件类型和对应的处理函数绑定
class Channel
{
private:
	int m_fd;
	std::function<void()> m_closeCallback;		// 关闭事件的回调 
	std::function<void()> m_readCallback;		// 读事件的回调
	int m_revents;          // 返回的事件
	int m_event;			// 关注的事件  
	Eventloop *m_loop;		// 管理本channel的loop指针	
public:
	Channel(Eventloop *loop, int fd);
	~Channel();
	// 处理事件的函数
	void handleEvent();
	// 将本channel从m_loop的管理中解除
	void remove();
	// 将本channel交给m_loop管理
	// NOTE 该函数应该在注册好所有类型事件的回调函数后调用，即在 `setXxxCallback()` 和 `enableXxx()` 后调用
	void registerToLoop();
	int getFd()
	{
		return m_fd;
	}
	Eventloop *getLoop()
	{
		return m_loop;
	}
	// 设置关注的事件
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
