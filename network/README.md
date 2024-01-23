### demo01.c :
- 这是一个网络通信服务端，它接受一个客户端连接后将接受到的来自客户端的消息原封不动发送回去。Ctrl-c或kill可以终止该程序。
- Usage: ./demo01 port
example: ./demo01 8888

### demo02.c 
- 这是一个网络通信客户端，它连接一个服务端后可以由终端向服务端发消息，并接收服务端的消息。Ctrl-c或kill可以终止该程序
- Usage: ./demo02 ip port
example: ./demo02 127.0.0.1 8888


### demo03.cpp :
- 这是一个多线程网络通信服务端，它接受客户端连接后将接受到的来自客户端的消息原封不动发送回去。Ctrl-c或kill可以终止该程序
- Usage: ./demo03 port
Example: ./demo03 8888

### demo04.cpp 
- 这是一个网络通信客户端，它连接一个服务端后不停的向服务端发消息，并接收服务端的消息。Ctrl-c或kill可以终止该程序
- Usage: ./demo04 ip port
Example: ./demo04 127.0.0.1 8888

### client.sh
- 这是一个测试脚本，它启动3000个demo04客户端进程，用于测试demo03是否可以正确完成程序退出时的清理工作
- Usage: ./client.sh 
Example: ./client.sh
