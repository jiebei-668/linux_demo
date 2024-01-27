#include "libsocket.h"

int init_server(const char *ip, const int port)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listenfd == -1)
	{
		return -1;
	}
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons(port);
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
	if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		close(listenfd);
		return -1;
	}
	if(listen(listenfd, MAX_BACKLOG) == -1)
	{
		close(listenfd);
		return -1;
	}
	return listenfd;
}
int init_client(const char *ip, const int port)
{
	int clientfd = socket(AF_INET, SOL_SOCKET, IPPROTO_TCP);
	if(clientfd == -1)
	{
		return -1;
	}
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);
	if(connect(clientfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		close(clientfd);
		return -1;
	}
	return clientfd;
}
bool send_nbytes(int socketfd, char *buf, size_t len, int flags)
{
	// 还需发送的字节数
	ssize_t rest_len = len;
	// 调用一次send()发送的字节数
	ssize_t send_size;
	while(rest_len > 0)
	{
		send_size = send(socketfd, buf, rest_len, flags);
		if(send_size <= 0)
		{
			return false;
		}
		rest_len -= send_size;
	}
	return true;
}
bool recv_nbytes(int socketfd, char *buf, size_t len, int flags)
{
	// 还需接收的字节数
	ssize_t rest_len = len;
	// 调用一次recv()接收的字节数
	ssize_t recv_size;
	while(rest_len > 0)
	{
		recv_size = recv(socketfd, buf, rest_len, flags);
		if(recv_size <= 0)
		{
			return false;
		}
		rest_len -= recv_size;
	}
	return true;
}
