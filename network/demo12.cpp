#include "libsocket.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <stdarg.h>

class EventLoop;
class Connection;
class Channel;
class Channel
{
private:
public:
	int m_fd;
	EventLoop *m_loop;
	std::function<void()> m_cb;
	// 只设置可读事件的回调
	void setCallBack(std::function<void()> cb)
	{
		m_cb = std::move(cb);
	}
	EventLoop *getLoop()
	{
		return m_loop;
	}
	int getFd()
	{
		return m_fd;
	}
	Channel(EventLoop *loop, int fd)
		: m_loop(loop)
		, m_fd(fd)
	{
	}
	~Channel()
	{
	}
};
class Connection
{
private:
public:
	int m_fd;
	EventLoop *m_loop;
	char m_buf[300];
	int m_buf_capacity = 300;
	Channel *m_channel;
	Connection(EventLoop *loop, int fd)
		: m_fd(fd)
		, m_loop(loop)
		, m_channel(new Channel(loop, fd))
	{
	}
	std::function<void(int, char *, int)> m_cb;
	void setCallBack(std::function<void(int, char *, int)> cb)
	{
		m_cb = cb;
		m_channel->setCallBack(std::bind(&Connection::readCallBack, this));
	}
	void readCallBack()
	{
		m_cb(m_fd, m_buf, m_buf_capacity);
	}
	~Connection()
	{
		delete m_channel;
		printf("close client[%d]...\n", m_fd);
		close(m_fd);
	}
};
class EventLoop
{
private:
	std::vector<Channel *> m_activeChannels;
	std::vector<Channel *> m_Channels;
	// int m_poll_fd;
	static const int FDNUM = 1024;
	struct pollfd m_fds[FDNUM];
	int m_maxfd;
public:
	void removeActiveChannel(Channel *c)
	{
		for(auto pp = m_activeChannels.begin(); pp != m_activeChannels.end(); pp++)
		{
			if(*pp == c)
			{
				m_activeChannels.erase(pp);
				break;
			}
		}
	}
	void removeChannel(Channel *c)
	{
		for(auto pp = m_Channels.begin(); pp != m_Channels.end(); pp++)
		{
			if(*pp == c)
			{
				m_Channels.erase(pp);
				m_um.erase(c->getFd());
				// 从m_fds中删除fd并更新m_maxfd
				m_fds[c->getFd()].fd = -1;
				for(int ii = c->getFd()-1; ii >= 0; --ii)
				{
					if(m_fds[ii].fd != -1)
					{
						m_maxfd = m_fds[ii].fd;
					}
				}
				break;
			}
		}
	}
	std::unordered_map<int, Channel *> m_um;
	EventLoop();
	void loop();
	void add_channel(Channel *c)
	{
		m_Channels.push_back(c);
		m_fds[c->getFd()].fd = c->getFd();
		m_fds[c->getFd()].events = POLLIN;
		m_maxfd = m_maxfd > c->getFd() ? m_maxfd : c->getFd();
		m_um[c->getFd()] =  c;
	}
	std::vector<Channel *>& channels()
	{
		return m_Channels;
	}
	std::vector<Channel *>& activeChannels()
	{
		return m_activeChannels;
	}
};
EventLoop::EventLoop()
	: m_maxfd(-1)
{
	for(int ii = 0; ii < EventLoop::FDNUM; ii++)
	{
		m_fds[ii].fd = -1;	
	}
}
void EventLoop::loop()
{
	while(true)
	{
		int eventnums = poll(m_fds, m_maxfd+1, -1);		
		if(eventnums == 0)
		{
			// timeout
			continue;
		}
		if(eventnums < 0)
		{
			perror("poll() failed...");
			exit(-1);
		}
		for(int fd = 0; fd <= m_maxfd; fd++)
		{
			if(m_fds[fd].fd == -1 || (m_fds[fd].revents&POLLIN) == 0)
			{
				continue;
			}
			m_activeChannels.push_back(m_um[fd]);
		}
		for(auto &one: m_activeChannels)
		{
			one->m_cb();
			removeActiveChannel(one);
		}
	}
}

// 单进程，单线程
class EchoServer
{
private:
public:
	int m_listenfd;
	std::vector<Connection *> m_vconns;
	// fd to Connection*
	std::unordered_map<int, Connection *> m_um;
	bool init(EventLoop *loop, char *ip="127.0.0.1", int port=9999);	
	// handler for accept
	void accepthandler()
	{
		int connfd = accept(m_listenfd, NULL, NULL);
		set_nonblock(connfd);
		printf("client[%d] connected\n", connfd);
		Connection *cn = new Connection(m_loop, connfd);	
		m_vconns.push_back(cn);
		m_um.insert({connfd, cn});
		cn->setCallBack(std::bind(&EchoServer::echohandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_vfds.push_back(connfd);
		m_loop->add_channel(cn->m_channel);
	}
	void echohandler(int connfd, char *buf, int buf_capacity)
	{
		int rsize;
		rsize = recv(connfd, buf, buf_capacity, 0);
		// opposite closed
		if(rsize == 0)
		{
			delete m_um[connfd];
			m_um.erase(connfd);
		}
		// error handler
		else if(rsize < 0)
		{

		}
		else
		{
			send(connfd, buf, rsize, 0);
		}
	}
	Channel *acceptChannel;
	EchoServer();
	std::vector<int> m_vfds;
	EventLoop *m_loop;
	~EchoServer()
	{
		delete acceptChannel;
	}
};
EchoServer::EchoServer()
	: acceptChannel(nullptr)
	, m_listenfd(-1)
{

}
bool EchoServer::init(EventLoop *loop, char *ip, int port)
{
	m_loop = loop;
	m_listenfd = init_server(ip, port);	
	set_nonblock(m_listenfd);
	set_reuseaddr(m_listenfd);
	acceptChannel = new Channel(loop, m_listenfd);
	acceptChannel->setCallBack(std::bind(&EchoServer::accepthandler, this));
	loop->add_channel(acceptChannel);
	return true;
}
EventLoop loop;
EchoServer server;
int main(int argc, char* argv[])
{
	signal(SIGPIPE, SIG_IGN);
	server.init(&loop);
	loop.loop();
	return 0;
}
