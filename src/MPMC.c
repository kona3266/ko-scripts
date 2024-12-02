/* multi producer mutlti consumer problem 
                                
P1---->                 +---> C1
         +-------------+|       
P2---->  |   QUEUE     ++---> C2
         +-------------+|       
P3---->                 +---> C3
                                

*/
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PRODUCER_NUM 2
#define CONSUMER_NUM 4

typedef struct Task {
    int seq;
} Task;
pthread_mutex_t mutexQ;
pthread_cond_t condQ;
pthread_cond_t condP;
int executeTask(int s) {
    int fn[s];
    fn[0] = 1;
    fn[1] = 1;
    if (s < 2) return fn[s];
    for(int i = 2; i <= s; i++) {
        fn[i] = fn[i-1] + fn[i-2];
    }
    printf("fn[%d] is %d\n", s, fn[s]);
    return fn[s];
}

typedef struct TaskQueue{
    Task *t;
    int head;
    int tail;
    int size;
} TaskQueue;

int is_full(TaskQueue *t){
    if (t->head == (t->tail + 1) % t->size) return 1;
    return 0;
}

int is_empty(TaskQueue *t){
    if (t->head == t->tail) return 1;
    return 0;
}

int add_item(TaskQueue *tq, Task *tk){
    if (is_full(tq)) {return -1;}
    tq->t[tq->tail] = *tk;
    tq->tail++;
    return 0;
}

int pop_item(TaskQueue *tq, Task *tk) {
    if (is_empty(tq)) {return -1;}
    tk->seq = (tq->t[tq->head]).seq;
    tq->head++;
    return 0;
}

void *produce_task(void *args){
    TaskQueue *tq = (TaskQueue *)args;
    int i = 0;
    pid_t tid = syscall(SYS_gettid);
    int s = rand() % 10;

    while(i < 25){
        Task task;
        task.seq = s;
        printf("thread id %d produce %d\n", tid, task.seq);
        pthread_mutex_lock(&mutexQ);
        while (is_full(tq) == 1) {
            pthread_cond_wait(&condP, &mutexQ);
        }

        int r = add_item(tq, &task);
        if (r == 0) {
            // condition_signal
            pthread_cond_signal(&condQ);
        }
        pthread_mutex_unlock(&mutexQ);
        i++;
        s++;
    }
    pthread_exit(NULL);
}

void *consume_task(void *args) {
    TaskQueue *tq = (TaskQueue *)args;
    while(1){
        Task task;
        pthread_mutex_lock(&mutexQ);
        while (is_empty(tq) == 1) {
            pthread_cond_wait(&condQ, &mutexQ);
        }
        int r = pop_item(tq, &task);
        if (r == 0) {
            pthread_cond_signal(&condP);
        }
        pthread_mutex_unlock(&mutexQ);
        executeTask(task.seq);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    pthread_t c[CONSUMER_NUM];
    pthread_t p[PRODUCER_NUM];
    pthread_mutex_init(&mutexQ, NULL);
    pthread_cond_init(&condQ, NULL);
    srand(time(NULL));
    TaskQueue tq = {.size = 5, .head=0, .tail=0}; 
    tq.t = malloc(sizeof(Task)*tq.size);
    for (int i = 0; i < PRODUCER_NUM; i++) {
        pthread_create(&p[i], NULL, produce_task, &tq);
    }

    for (int i = 0; i < CONSUMER_NUM; i++) {
        pthread_create(&c[i], NULL, consume_task, &tq);
    }

    for (int i = 0; i < CONSUMER_NUM; i++) {
        pthread_join(c[i], NULL);
    }

    for (int i = 0; i < PRODUCER_NUM; i++) {
        pthread_join(p[i], NULL);
    }

    pthread_mutex_destroy(&mutexQ);
    pthread_cond_destroy(&condP);
    pthread_cond_destroy(&condQ);
    return 0;
}