/***************************************************************************
 * program: demo11(.cpp)
 * author: jiebei
 * 这是一个使用epoll的网络通信服务端程序，它接收客户端的消息并原封不动返回。使用信号2（Ctrl-c）或信号15可以终止信号。
 **************************************************************************/
#include "libsocket.h"
#define MAX_FDNUM 1024
#define RETURN_EVENT_NUM 10

int listenfd;
bool fd_busy[MAX_FDNUM];
struct epoll_event ret_ev[RETURN_EVENT_NUM];
void exit_fun(int sig);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./demo11 port\n");
		printf("Example: ./demo11 8888\n");
		exit(-1);
	}
	signal(2, exit_fun);
	signal(15, exit_fun);
	listenfd = init_server("127.0.0.1", 8888);
	if(listenfd == -1)
	{
		printf("init_server() failed...\n");
		exit(-1);
	}
	memset(fd_busy, 0, sizeof(fd_busy));
	fd_busy[listenfd] = true; 
	set_nonblock(listenfd);
	int epfd = epoll_create(1000);
	if(epfd == -1)
	{
		printf("epoll_create() failed...\n");
		exit_fun(-1);
	}
	struct epoll_event ev;
	memset(&ev, 0, sizeof(struct epoll_event));
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);	
	while(true)
	{
		int event_num = epoll_wait(epfd, ret_ev,  RETURN_EVENT_NUM, -1);
		if(event_num == -1)
		{
			printf("epoll_wait() failed...\n");
			exit_fun(-1);
		}
		for(int ii = 0; ii < event_num; ii++)
		{
			if(ret_ev[ii].data.fd == listenfd)
			{
				int connfd = accept(listenfd, NULL, NULL);
				if(connfd == -1)
				{
					// 连接失败的错误处理，这里是忽略
					continue;
				}
				set_nonblock(connfd);
				struct epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = connfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev); 
				continue;
			}
			char buf[512];
			memset(buf, 0, sizeof(buf));
			int rrecv = recv(ret_ev[ii].data.fd, buf, 512, 0);
			if(rrecv <= 0)
			{
				printf("client[%d] disconnect...\n", ret_ev[ii].data.fd);
				close(ret_ev[ii].data.fd);
				epoll_ctl(epfd, EPOLL_CTL_DEL, ret_ev[ii].data.fd, NULL);
			}
			else
			{
				send(ret_ev[ii].data.fd, buf, rrecv, 0);
			}
		}
	}
	return 0;
}
void exit_fun(int sig)
{
	signal(2, SIG_IGN);
	signal(15, SIG_IGN);
	for(int i = 0; i < MAX_FDNUM; i++)
	{
		if(fd_busy[i])
		{
			close(i);
			fd_busy[i] = false;
		}
	}
	printf("server exit...\n");
	exit(-1);

}
