/********************************************************************
 * program: demo01.cpp
 * author: jiebei
 * 本程序用条件变量和互斥锁实现了消费者生产者模型。向程序发送信号15来生产产品，产品缓冲区有上限由参数设置。程序推荐用信号2（Ctrl-c）终止。
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <vector>

pthread_t producer_id;
// 存放所有消费者线程id的容器
std::vector<pthread_t> v_consumer_id;
// 产品缓冲区中产品个数
int product_num = 0;
// 产品缓冲区大小
int size_product_queue;
// 生产者每次生产产品的数量
int product_num_each_produce;

pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void produce(int);
void *consume(void *);

// 消费者线程退出时的清理函数
void consumer_cleanup(void *);
// 收到信号2时做的善后工作
void exit_fun(int sig);

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		printf("Usage: ./demo01 num_consumer size_product_queue product_num_each_produce\n");
		printf("Example: ./demo01 5 20 5\n");
		return -1;
	}
	int num_consumer = atoi(argv[1]);
	size_product_queue = atoi(argv[2]);
	product_num_each_produce = atoi(argv[3]);

	printf("==============================\n");
	printf("消费者数量：%d\n", num_consumer);
	printf("产品缓冲区大小：%d\n", size_product_queue);
	printf("生产者数量：1\n");
	printf("生产者每次生产%d个产品\n", product_num_each_produce);
	printf("==============================\n");

	signal(2, exit_fun);
	signal(15, produce);

	for(int ii = 0; ii < atoi(argv[1]); ii++)
	{
		pthread_t id;
		pthread_create(&id, NULL, consume, (void *)(long)ii);
		v_consumer_id.push_back(id);
	}
	for(int ii = 0; ii < atoi(argv[1]); ii++)
	{
		pthread_join(v_consumer_id[ii], NULL);
	}

	return 0;
}
void exit_fun(int sig)
{
	for(auto one: v_consumer_id)
	{
		pthread_cancel(one);
	}
	pthread_mutex_unlock(&mutex);
	printf("programmer exit...\n");
	exit(0);
}
void produce(int sig)
{
	pthread_mutex_lock(&mutex);
	// 注意是while防止虚假唤醒
	while(product_num == size_product_queue)
	{
		pthread_cond_wait(&not_full, &mutex);
	}
	int new_num = size_product_queue-product_num >= product_num_each_produce ? product_num_each_produce : size_product_queue-product_num;
	printf("生产者已新生产%d个新产品\n", new_num);
	product_num += new_num;
	printf("产品缓冲区共%d个产品\n", product_num);
	// V操作的顺序：在linux下推荐先V再解锁
	pthread_cond_broadcast(&not_empty);
	pthread_mutex_unlock(&mutex);
}
void consumer_cleanup(void *arg)
{
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}
// @param: 被强转为void *类型的消费者编号0-(num_consumer-1)
void *consume(void *arg)
{
	pthread_cleanup_push(consumer_cleanup, NULL);
	int consumer_id = (int)(long)arg;
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&mutex);
		while(product_num == 0)
		{
			pthread_cond_wait(&not_empty, &mutex);
		}
		product_num--;
		printf("消费者%2d已消耗一个产品，产品缓冲池剩余%d个产品\n", consumer_id, product_num);
		pthread_cond_broadcast(&not_full);
		pthread_mutex_unlock(&mutex);
	}
	pthread_cleanup_pop(1);
	return NULL;
}
