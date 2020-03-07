#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define TSQUEUE_SIZE_MAX SIZE_MAX
#define NO_FREE_FUNC     NULL

typedef void (*func_ptr)(void*);

typedef enum TSQueueWait {
    TS_NO_WAIT  = 0,
    TS_WAIT_SET = 1,
    TS_WAIT_GET = 2,
    TS_WAIT_ALL = 3
} TSQueueWait;

typedef struct TSQueueNode {
    void* data;

    struct TSQueueNode* next;
} TSQueueNode;

typedef struct TSQueue {
    struct TSQueueNode* head;
    struct TSQueueNode* tail;

    size_t size;
    size_t max_size;
    size_t data_size;

    func_ptr free_func;

    TSQueueWait wait_behavior;

    pthread_mutex_t mutex;
    pthread_cond_t set_cond;
    pthread_cond_t get_cond;
} TSQueue;

static inline bool tsQueueIsEmpty(const TSQueue* Q) {
<<<<<<< HEAD
    return Q->head == NULL;
=======
	return Q->head == NULL;
>>>>>>> 6f93a32f9193544716b5ff1da75bfb5dedd679d4
}

void tsQueueInit(TSQueue** Q, size_t max_size, size_t data_size, 
                 func_ptr free_func, TSQueueWait wait_behavior);

void tsQueueSet(TSQueue* Q, void* data);
void* tsQueueGet(TSQueue* Q);

void tsQueueClear(TSQueue* Q);
void tsQueueFree(TSQueue** Q);

#endif