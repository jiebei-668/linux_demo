#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <vector>

pthread_t producer_id;
std::vector<pthread_t> v_consumer_id;
int product_num = 0;
int size_product_queue;
int product_num_each_produce;
pthread_cond_t not_full;
pthread_cond_t not_empty;
pthread_mutex_t mutex;

void produce(int);
void *consume(void *);
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

	signal(2, produce);
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
	// V操作的顺序时无所谓的可以互换的
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&not_empty);
}
// @param: 被强转为void *类型的消费者编号0-(num_consumer-1)
void *consume(void *arg)
{
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
		pthread_mutex_unlock(&mutex);
		pthread_cond_broadcast(&not_full);
	}
	return NULL;
}
