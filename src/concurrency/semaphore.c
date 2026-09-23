#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


#define CONSUME_THREAD  4
#define PRODUCE_THREAD  2

sem_t semaphore1, semaphore2;
int counter = 0;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


                                                            
                                                            
//        HEAD       TAIL                                         
//          │          │                                          
// BUFER    │          │                                          
//          ▼          ▼                                          
//        ┌───┬───┬───┬───┬───┬───┐                            
//        │ 15│17 │56 │   │   │   │                            
//        └───┴───┴───┴───┴───┴───┘                            
//          1   2   3   4   5   6                              
                                                            
//            TAIL HEAD                                    
//             │    │                                      
//        ┌───┬▼──┬─▼─┬───┬───┬───┐                            
//        │ 23│   │56 │ 44│ 32│ 21│                            
//        └───┴───┴───┴───┴───┴───┘                            
//          1   2   3   4   5   6                              
                                                            
                                                            
typedef struct Buffer {
    int head, tail;
	int size;
	int *buf;
} buf;

bool is_full(buf *b){
	if (b->head == (b->tail + 1) % b->size){
		return true;
	}
	return false;
}

bool is_empty(buf *b) {
	if (b->head == b->tail){
		return true;
	}
	return 0;
}

bool add_item(buf *b, int i) {
	if (is_full(b)){
		printf("buffer full!\n");
		return false;
	}
	b->buf[b->tail] = i;
	b->tail = (b->tail + 1) % b->size;
	return true;
}

bool pop_item(buf *b, int *r) {
	if (is_empty(b)){
		printf("buffer empty!\n");
		return false;
	}
	*r = b->buf[b->head];
	b->head = (b->head + 1) % b->size;
	return true;
}

void *child()
{
    for (int i=0; i < 3; i++) {
		pthread_mutex_lock(&mutex1);
        int tmp = counter;
	    sleep(1);
	    counter = tmp + 1;
		pthread_mutex_unlock(&mutex1);
	    printf("Counter=%d\n", counter);	
	}
	pthread_exit(NULL);
}

void *consume(void *args){
	buf *b = (buf *)args;
    for (int i = 0; i < 8; i++) {
        sem_wait(&semaphore1);
		printf("cosume 1 job\n");
		int r = -1;
		pthread_mutex_lock(&mutex1);
		pop_item(b, &r);
		pthread_mutex_unlock(&mutex1);
		printf("get %d from queue\n", r);
		sem_post(&semaphore2);
	}
	pthread_exit(NULL);
}

void *produce(void * args){
	while(1) {
		buf *b = (buf *)args;
		sem_wait(&semaphore2);
		printf("produce 1 job\n");
		srand(time(NULL));
		int r = rand();
		pthread_mutex_lock(&mutex1);
		add_item(b, r);
		pthread_mutex_unlock(&mutex1);
		sem_post(&semaphore1);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv) 
{
    pthread_t t1, t2;
	pthread_create(&t1, NULL, child, NULL);
	pthread_create(&t2, NULL, child, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	printf("done\n");
	pthread_t c, p;
	sem_init(&semaphore1, 0, 0);
	// work queue initial size is 5, leave 1 empty
	sem_init(&semaphore2, 0, 4);
	buf queue = {.size=5, .head=0, .tail=0};
	queue.buf = malloc((queue.size)*sizeof(int));
	pthread_create(&c, NULL, consume, (void*)&queue);
    pthread_create(&p, NULL, produce, (void*)&queue);
	pthread_join(c, NULL);
	free(queue.buf);
	pthread_mutex_destroy(&mutex1);
	return 0;
}
