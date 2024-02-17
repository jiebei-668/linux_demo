/********************************************************************************************
 * program: demo10.cpp
 * author: jiebei
 * 本程序是反向代理程序的外网端程序，接通内网代理程序和通过外网反向代理程序中转的外网客户端程序。与内网代理程序通信的端口由参数指定,目前暂时写死外网的代理端口和内网的端口以及被代理服务程序的地址，且暂时只代理一个，后续由配置文件指定，可指定多个代理。
 * 程序流程：
 * 1 外网代理程序初始化和内网代理程序通信的socket（监听socket）
 * 2 内网代理程序向外网代理程序发起连接，外网代理程序接受内网代理程序的连接建立控制通道
 * 3 外网代理程序解析被代理服务程序的参数（文件），逐个建立和外网客户端程序通信的socket（监听socket）
 * 3 循环执行 4
 * 4 当外网代理程序收到外网客户端程序连接请求后，向内网代理程序通过控制通道发送外网客户端程序请求的内网被代理程序的信息。内网代理程序接收外网代理程序通过控制通道发来的被代理程序的信息向被代理服务程序和外网代理程序发起连接，并且将这两端socket对接。外网代理程序接受内网代理程序的连接后同样地将内网代理程序的socket和外网客户端socket对接。这样建立了被代理程序和外网客户程序的连接后，外网客户端程序就可以访问被代理程序。
 * Usage: ./demo10 out_port
 * Example: ./demo10 8866
 *******************************************************************************************/
#include "libsocket.h"
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>
#define MAX_FD_NUM 1024
struct pollfd sockets[MAX_FD_NUM];
// 被代理程序的参数结构体
typedef struct proxyarg
{
	// 外网代理程序用于和外网客户端程序通信的端口
	int outport;
	// 内网被代理服务程序的端口
	char ip[100];
	// 内网被代理服务程序的地址
	int inport;
	// 外网代理程序用于监听外网客户端程序socket
	int listenfd;
}proxyarg_t;
// 记录所有的被代理程序的参数
std::vector<proxyarg_t> proxyargs;
int maxfd;
// 记录对端socket的结构，如果没有对端socket为-1
// 如果ii的对端为jj，那么opposite_socket[ii]=jj opposite_socket[jj]=ii
int opposite_socket[MAX_FD_NUM];
// 退出函数，清理资源，关socket
void exit_fun(int sig);
// 更新maxfd的值
int update_maxfd();
// 加载被代理程序和外网代理程序端口的参数
bool load_args(const char* initfile);
// 初始化和外网客户端程序通信的各socket
bool init_proxy();
// 判断是否为外网客户端发起的连接请求
bool is_client(int fd);
int listenfd = -1;
int cmdsocket = -1;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo10 out_port\n");
		printf("Example: ./demo10 8866\n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		sockets[ii].fd = -1;
		opposite_socket[ii] = -1;
	}
	// 加载被代理程序的参数
	if(load_args("none") == false)
	{
		printf("外网代理程序错误[0]-load_args() failed!\n");
		exit_fun(-1);
	}
	listenfd = init_server("127.0.0.1", atoi(argv[1]));
	if(listenfd == -1)
	{
		printf("init_server() failed...\n");
		exit_fun(-1);
	}
	// 设置可重用端口
	set_reuseaddr(listenfd);
	if((cmdsocket = accept(listenfd, NULL, NULL)) == -1) 
	{
		printf("外网代理程序错误[1]-accept() failed!\n");
		exit_fun(-1);
	}
	// 设置非阻塞
	set_nonblock(cmdsocket);
	// 更新pollfd
	sockets[cmdsocket].fd = cmdsocket;
	sockets[cmdsocket].events = POLLIN;
	// 更新maxfd
	maxfd = cmdsocket;
	if(init_proxy() == false)
	{
		printf("外网代理程序错误[2]-init_proxy() failed!\n");
		exit_fun(-1);
	}
	
	while(true)
	{
		int event_num = poll(sockets, maxfd+1, -1);
		if(event_num == -1)
		{
			// 处理错误并退出
			printf("poll() failed...\n");
			printf("errno[%d] info[%s]\n", errno, strerror(errno));
			exit_fun(-1);
		}
		if(event_num == 0)
		{
			// 处理超时
			continue;
		}
		for(int fd = 0; fd <= maxfd; fd++)
		{
			if(sockets[fd].fd == -1 || (sockets[fd].revents&POLLIN) == 0)
			{
				continue;
			}
			// 处理外网客户端的连接，向内网代理程序发代理参数，连通内网代理程序和外网客户端程序
			if(is_client(fd))
			{
				int out_connfd = accept(sockets[fd].fd, NULL, NULL);
				// 如果接受外网客户端失败，则继续下一个循环
				if(out_connfd == -1)
				{
					continue;
				}
				char cmdbuf[100];
				memset(cmdbuf, 0, sizeof(cmdbuf));
				// 找到外网客户端程序请求的被代理服务程序参数在proxyargs中的位置，把被代理程序参数通过控制通道发给内网代理程序
				int argpos = 0;
				for(argpos = 0; argpos < proxyargs.size(); argpos++)
				{
					if(proxyargs[argpos].listenfd == sockets[fd].fd)
					{
						break;
					}
				}
				if(argpos == proxyargs.size())
				{
					close(out_connfd);
					continue;
				}
				// 通过控制通道向内网代理程序发送被代理程序参数
				snprintf(cmdbuf, sizeof(cmdbuf)-1, "%s<sep>%d", proxyargs[argpos].ip, proxyargs[argpos].inport);
				send(cmdsocket, cmdbuf, strlen(cmdbuf), 0);
				// 接受内网代理程序的连接，对接内网代理程序和外网客户端程序
				int in_connfd = accept(listenfd, NULL, NULL);
				if(in_connfd == -1)
				{
					close(out_connfd);
					continue;
				}
				// 设置非阻塞
				set_nonblock(in_connfd);
				set_nonblock(out_connfd);
				// 对接对端socket
				sockets[in_connfd].fd = in_connfd;
				sockets[in_connfd].events = POLLIN;
				maxfd = maxfd > in_connfd ? maxfd : in_connfd;
				sockets[out_connfd].fd = out_connfd;
				sockets[out_connfd].events = POLLIN;
				maxfd = maxfd > out_connfd ? maxfd : out_connfd;
				opposite_socket[in_connfd] = out_connfd;
				opposite_socket[out_connfd] = in_connfd;
				continue;
			}
			// 如果不是客户端连接请求，将数据发给对端
			char buf[1600];
			memset(buf, 0, sizeof(buf));
			int rsize = recv(fd, buf, sizeof(buf), 0);
			if(rsize <= 0)
			{
				close(fd);
				close(opposite_socket[fd]);
				sockets[fd].fd = -1;
				sockets[opposite_socket[fd]].fd = -1;
				maxfd = update_maxfd();
			}
			else
			{
				send(opposite_socket[fd], buf, rsize, 0);
			}
		}
	}

	return 0;
}
void exit_fun(int sig)
{
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);
	if(listenfd != -1)
	{
		close(listenfd);
		listenfd = -1;
	}
	for(int ii = 0; ii < MAX_FD_NUM; ii++)
	{
		if(~sockets[ii].fd)
		{
			close(sockets[ii].fd);
			sockets[ii].fd = -1;
		}
	}
	exit(-1);
}
int update_maxfd()
{
	int ii = MAX_FD_NUM;
	for( ii = MAX_FD_NUM-1; ii >= 0; ii--)
	{
		if(~sockets[ii].fd)
		{
			break;
		}
	}
	return sockets[ii].fd;
}
bool load_args(const char* initfile)
{
	proxyarg_t arg = {10000, "127.0.0.1", 6666, -1};
	proxyargs.push_back(arg);
	return true;
}
bool init_proxy()
{
	for(auto &proxy: proxyargs)
	{
		int fd = init_server("127.0.0.1", proxy.outport);
		if(fd == -1)
		{
			return false;
		}
		// 设置非阻塞
		set_nonblock(fd);
		// 设置地址重用
		set_reuseaddr(fd);
		proxy.listenfd = fd;
		sockets[fd].fd = fd;
		sockets[fd].events = POLLIN;
		maxfd = maxfd > fd ? maxfd : fd;
	}
	return true;
}
bool is_client(int fd)
{
	for(auto &proxy: proxyargs)
	{
		if(fd == proxy.listenfd)
		{
			return true;
		}
	}
	return false;
}
