#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define TSQUEUE_SIZE_MAX SIZE_MAX
#define NO_FREE_FUNC     NULL

typedef void (*func_ptr)(void*);

typedef int TSQueueWaitType;

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

    TSQueueWaitType wait_behavior;

    pthread_mutex_t mutex;
    pthread_cond_t set_cond;
    pthread_cond_t get_cond;
} TSQueue;

static inline bool tsQueueIsEmpty(const TSQueue* Q) {
	return Q->head == NULL;
}

void tsQueueInit(TSQueue** Q, size_t max_size, size_t data_size, 
                 func_ptr free_func, TSQueueWaitType wait_behavior);

void tsQueueSet(TSQueue* Q, void* data);
void* tsQueueGet(TSQueue* Q);

void tsQueueClear(TSQueue* Q);
void tsQueueFree(TSQueue** Q);

#endif