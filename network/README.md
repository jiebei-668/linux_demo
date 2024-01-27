### demo01.c :
- 这是一个网络通信服务端，它接受一个客户端连接后将接受到的来自客户端的消息原封不动发送回去。Ctrl-c或kill可以终止该程序。
- Usage: ./demo01 port
example: ./demo01 8888

### demo02.c 
- 这是一个网络通信客户端，它连接一个服务端后可以由终端向服务端发消息，并接收服务端的消息。Ctrl-c或kill可以终止该程序
- Usage: ./demo02 ip port
example: ./demo02 127.0.0.1 8888


### demo03.cpp :
- 这是一个多线程网络通信服务端，它接受客户端连接后将接受到的来自客户端的消息原封不动发送回去。信号2（Ctrl-c）或信号5可以终止该程序
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

### demo05.cpp 
- 这是一个多进程网络通信服务端，它接受客户端连接后将接受到的来自客户端的消息原封不动发送回去。信号2（Ctrl-c）或信号15可以终止该程序
- Usage: ./demo05 port
Example: ./demo05 8888

### demo06.cpp 
- 这是一个使用select的网络通信服务端程序，它接收客户端的消息并原封不动返回。使用信号2（Ctrl-c）或信号15可以终止信号。
- Usage: ./demo06 port
Example: ./demo06 8888

### demo07.cpp 
- 这是一个使用poll的网络通信服务端程序，它接收客户端的消息并原封不动返回。使用信号2（Ctrl-c）或信号15可以终止信号。
- Usage: ./demo07 port
Example: ./demo07 8888

### demo08.cpp 
- 本程序使用poll实现正向代理，先暂时固定用127.0.0.1 8881代理127.0.0.1-8883，用127.0.0.1-8882代理127.0.0.1-8884
- 使用信号2（Ctrl-c)或信号15可以终止程序
- Usage: ./demo08 configfile Example: ./demo08 ./forwardproxy.config
- 注：还暂时没有用配置文件来配置代理的服务器的地址，后续更新
