### demo01(.cpp)
- 本程序用条件变量和互斥锁实现了消费者生产者模型。向程序发送信号15来生产产品，产品缓冲区有上限由参数设置。程序推荐用信号2（Ctrl-c）终止。
- Usage: ./demo01 num_consumer size_product_queue product_num_each_produce
- Example: ./demo01 5 20 5
### demo02(.cpp)
- 本程序用信号量实现了消费者生产者模型。向程序发送信号15来生产产品，产品缓冲区有上限由参数设置。程序推荐用信号2（Ctrl-c）终止。
- Usage: ./demo02 num_consumer size_product_queue product_num_each_produce
- Example: ./demo02 5 20 5
### demo03(.cpp)
- 功能：使用管道实现父子进程通信，父进程通过管道向子进程发送100条消息，子进程读取后打印出来。
- Usage: ./demo03
