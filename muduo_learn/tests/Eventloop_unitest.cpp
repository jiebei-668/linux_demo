#include "../Eventloop.h"
#include "../Channel.h"
#include "../header/libsocket.h"

void clientReadCb(int fd);
void clientCloseCb(Channel *cnl);


// listen fd的read_callback
// 创建新的channel，这个channel是和client进行业务处理的channel
void listenReadCb(Eventloop *loop, int fd)
{
	int client_fd = accept(fd, NULL, NULL);	
	if(client_fd == -1)
	{
		printf("accept fail!\n");
		return;
	}
	printf("accept fd[%d]\n", client_fd);

	Channel *cnl = new Channel(loop, client_fd);
	cnl->setReadCallback(std::bind(clientReadCb, client_fd));
	cnl->setCloseCallback(std::bind(clientCloseCb, cnl));
	cnl->enableReading();
	cnl->enableClose();
	cnl->registerToLoop();
}
// 和client进行业务处理的read cb，接收客户端信息并打印
void clientReadCb(int fd)
{
	char buf[512];
	memset(buf, 0, sizeof buf);
	recv(fd, buf, sizeof buf, 0);
	printf("recv from client[%d]: %s\n", fd, buf);
}
// 和client进行业务处理的close cb
// 从loop中取消channel
// 关闭channel中的fd
// fixme 如果fd没有打开呢？直接close会有问题吗
void clientCloseCb(Channel *cnl)
{
	printf("remove channel from loop, fd=%d\n", cnl->getFd());
	cnl->remove();	
	close(cnl->getFd());
}
// fixme 这里其实没用，listen的fd断开说明服务端停止了
void closeCallback(Channel *cnl, int fd)
{
	printf("remove channel, fd=%d\n", fd);
	cnl->remove();
	close(fd);
	printf("remove channel from Eventloop and close fd=%d\n", fd);
}
int main(int argc, char* argv[])
{
	int fd = init_server("127.0.0.1", 8888);
	if( fd == -1)
	{
		printf("init_server() failed!...\n");
		exit(-1);
	}
	printf("server init success...\n");
	Eventloop el;
	Channel cnl(&el, fd);
	cnl.setReadCallback(std::bind(listenReadCb, &el, fd));
	cnl.setCloseCallback(std::bind(closeCallback, &cnl, fd));
	cnl.enableReading();
	el.loop();
	return 0;
}
