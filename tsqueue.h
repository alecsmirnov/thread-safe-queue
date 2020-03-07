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
    bool set_wait;
    bool get_wait;

    pthread_mutex_t mutex;
    pthread_cond_t set_cond;
    pthread_cond_t get_cond;
} TSQueue;

static inline bool tsQueueIsEmpty(const TSQueue* Q) {
    return Q->head == NULL;
}

static inline bool tsQueueIsFull(const TSQueue* Q) {
    return Q->size == Q->max_size;
}

static inline size_t tsQueueSize(const TSQueue* Q) {
    return Q->size;
}

static inline size_t tsQueueMaxSize(const TSQueue* Q) {
    return Q->max_size;
}

void tsQueueInit(TSQueue** Q, size_t max_size, size_t data_size, 
                 func_ptr free_func, TSQueueWait wait_behavior);

void tsQueueSet(TSQueue* Q, void* data);
void* tsQueueGet(TSQueue* Q);

void tsQueueSetWaitPrepare(TSQueue* Q);
void tsQueueGetWaitPrepare(TSQueue* Q);

void tsQueueSetWaitExit(TSQueue* Q);
void tsQueueGetWaitExit(TSQueue* Q);

void tsQueueClear(TSQueue* Q);
void tsQueueFree(TSQueue** Q);

#endif