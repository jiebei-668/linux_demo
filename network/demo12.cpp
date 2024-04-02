#include "libsocket.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <stdarg.h>

// 各个对象的channel由各个对象来释放
// Connection对象 Accept对象拥有channel对象，EventLoop对象 EchoServer不持有channel对象
// channel中会关闭对应文件
class Acceptor;
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
		m_cb = cb;
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
		close(m_fd);
	}
};
class Connection
{
private:
public:
	EventLoop *m_loop;
	char m_buf[300];
	int m_buf_capacity = 300;
	Channel *m_channel;
	Connection(EventLoop *loop, int fd)
		: m_loop(loop)
		, m_channel(new Channel(loop, fd))
	{
	}
	std::function<void(int, char *, int)> m_OnMsgCallBack;
	std::function<void(int)> m_OnCloseCallBack;
	void setOnMsgCallBack(std::function<void(int, char *, int)> cb)
	{
		m_OnMsgCallBack = cb;
	}
	void setOnCloseCallBack(std::function<void(int)> cb)
	{
		m_OnCloseCallBack = cb;
	}
	void readCallBack()
	{
		int rsize;
		memset(m_buf, 0, m_buf_capacity);
		rsize = ::recv(m_channel->getFd(), m_buf, m_buf_capacity, 0);
		if(rsize > 0)
		{
			m_OnMsgCallBack(m_channel->getFd(), m_buf, rsize);
		}
		else if(rsize == 0)
		{
			m_OnCloseCallBack(m_channel->getFd());
		}
		else
		  // FIXME handler error
		{

		}
	}
	// must call after specialfy m_OnMsgCallBack, m_OnCloseCallBack and m_onerrcallback!!!
	// must call before loop
	void init()
	{
		m_channel->setCallBack(std::bind(&Connection::readCallBack(), this));
	}
	~Connection()
	{
		printf("close client[%d]...\n", m_channel->getFd());
		delete m_channel;
	}
};
// 这个类管理各channel以及进行poll
class EventLoop
{
private:
	std::vector<Channel *> m_activeChannels;
	// m_Channels貌似可以不用
	// std::vector<Channel *> m_Channels;
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
	//void removeChannel(Channel *c)
	//{
	//	for(auto pp = m_Channels.begin(); pp != m_Channels.end(); pp++)
	//	{
	//		if(*pp == c)
	//		{
	//			m_Channels.erase(pp);
	//			m_fdToChannel.erase(c->getFd());
	//			// 从m_fds中删除fd并更新m_maxfd
	//			m_fds[c->getFd()].fd = -1;
	//			for(int ii = c->getFd()-1; ii >= 0; --ii)
	//			{
	//				if(m_fds[ii].fd != -1)
	//				{
	//					m_maxfd = m_fds[ii].fd;
	//				}
	//			}
	//			break;
	//		}
	//	}
	//}
	std::unordered_map<int, Channel *> m_fdToChannel;
	EventLoop();
	void loop();
	void add_channel(Channel *c)
	{
		//m_Channels.push_back(c);
		m_fds[c->getFd()].fd = c->getFd();
		m_fds[c->getFd()].events = POLLIN;
		m_maxfd = m_maxfd > c->getFd() ? m_maxfd : c->getFd();
		m_fdToChannel[c->getFd()] =  c;
	}
	//std::vector<Channel *>& channels()
	//{
	//	return m_Channels;
	//}
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
			m_activeChannels.push_back(m_fdToChannel[fd]);
		}
		for(auto &one: m_activeChannels)
		{
			one->m_cb();
			removeActiveChannel(one);
		}
	}
}

class Acceptor
{
private:
public:
	Channel *m_channel;
	// m_loop->m_fd is listenfd
	EventLoop *m_loop;
	Acceptor(EventLoop *loop, int fd)
		: m_loop(loop)
		, m_channel(new Channel(loop, fd))
	{
	}
	std::function<void(int)> m_cb;
	void setCallBack(std::function<void(int)> cb)
	{
		m_cb = cb;
		m_channel->setCallBack(std::bind(&Acceptor::acceptCallBack, this));
	}
	void acceptCallBack()
	{
		m_cb(m_channel->getFd());
	}
	~Acceptor()
	{
		delete m_channel;
	}
};
// 单进程，单线程
class EchoServer
{
private:
public:
	int m_listenfd;
	// 析构函数中释放所有的connection
	std::vector<Connection *> m_vconns;
	// fd to Connection*
	std::unordered_map<int, Connection *> m_fdToConnection;
	Acceptor *m_acceptor;
	bool init(EventLoop *loop, char *ip="127.0.0.1", int port=9999);	
	// handler for accept
	void accepthandler()
	{
		int connfd = accept(m_listenfd, NULL, NULL);
		set_nonblock(connfd);
		printf("client[%d] connected\n", connfd);
		Connection *cn = new Connection(m_loop, connfd);	
		m_vconns.push_back(cn);
		m_fdToConnection.insert({connfd, cn});
		cn->setOnCloseCallBack(std::bind(&EchoServer::closehandler, this, std::placeholders::_1));
		cn->setOnMsgCallBack(std::bind(&EchoServer::echohandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		cn->init();
		m_vfds.push_back(connfd);
		m_loop->add_channel(cn->m_channel);
	}
	void echohandler(int connfd, char *buf, int rsize)
	{
		send(connfd, buf, rsize, 0);
	}
	void closehandler(int connfd)
	{
		close(connfd);
		Connection *p_conn = m_fdToConnection[connfd];
		delete p_conn;
		m_fdToConnection.erase(connfd);
		printf("client[%d] closed...\n", connfd);
		printf("server-m_vconns.size() = %ld\n", m_vconns.size());
		printf("m_fdToChannel.find(%d)==%p\n", connfd, m_fdToConnection.find(connfd));
		printf("m_fdToChannel.find(%d)==m_fdToChannel::end()=%d\n", connfd, m_fdToConnection.find(connfd)==m_fdToConnection.end());
	}
	EchoServer();
	std::vector<int> m_vfds;
	EventLoop *m_loop;
	~EchoServer()
	{
		delete m_acceptor;
		for(auto &one: m_fdToConnection)
		{
			delete one.second;
		}
	}
};
EchoServer::EchoServer()
	: m_listenfd(-1)
{

}
bool EchoServer::init(EventLoop *loop, char *ip, int port)
{
	m_loop = loop;
	m_listenfd = init_server(ip, port);	
	set_nonblock(m_listenfd);
	set_reuseaddr(m_listenfd);
	m_acceptor = new Acceptor(loop, m_listenfd);
	m_acceptor->setCallBack(std::bind(&EchoServer::accepthandler, this));
	loop->add_channel(m_acceptor->m_channel);
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
