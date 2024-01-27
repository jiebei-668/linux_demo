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
// listen()的第二个参数
#define MAX_BACKLOG 512
// 成功返回监听socket，失败返回-1
int init_server(const char *ip, const int port);
int init_client(const char *ip, const int port);
#endif

