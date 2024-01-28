#ifndef __LIBSOCKET__H__
#define __LIBSOCKET__H__
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>
// listen()的第二个参数
#define MAX_BACKLOG 512
// 成功返回监听socket，失败返回-1
int init_server(const char *ip, const int port);
// 成功返回socket，失败返回-1
int init_client_and_connect(const char *ip, const int port);
// 发送len个字节数据
bool send_nbytes(int socketfd, char *buf, size_t len, int flags=0);
// 接收len个字节数据
bool recv_nbytes(int socketfd, char *buf, size_t len, int flags=0);
// 加了协议头的send，前四个字节存储长度
bool tcp_send(const int socketfd, char *buf, const size_t len, const int flags=0);
// 加了协议头的recv，接收的前四个字节为长度
bool tcp_recv(const int socketfd, char *buf, const size_t buf_len, const int flags=0);	
// 设置为非阻塞
void set_nonblock(const int socketfd);
#endif

