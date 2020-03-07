#include "tsqueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define bitCheck(val, bit_pos) \
    (val & (1 << bit_pos))

typedef enum TSQueueWaitBit {
    TS_WAIT_SET_BIT = 0,
    TS_WAIT_GET_BIT = 1
} TSQueueWaitBit;

void tsQueueInit(TSQueue** Q, size_t max_size, size_t data_size, 
                 func_ptr free_func, TSQueueWait wait_behavior) {
    (*Q) = (TSQueue*)malloc(sizeof(TSQueue));
    if ((*Q) == NULL) 
        throwErr("Error: out of memmory!");

    (*Q)->head = (*Q)->tail = NULL;

    (*Q)->size = 0;
    (*Q)->max_size = max_size;

    (*Q)->data_size = data_size;
    (*Q)->free_func = free_func;

    (*Q)->wait_behavior = wait_behavior;
    (*Q)->set_wait = true;
    (*Q)->get_wait = true;

    int err = 0;
    err = pthread_mutex_init(&(*Q)->mutex, NULL);
    if (err != 0)
        throwErr("Error: cannot initialize push mutex!");

    err = pthread_cond_init(&(*Q)->set_cond, NULL); 
    if (err != 0)
        throwErr("Error: cannot initialize set cond variable!");

    err = pthread_cond_init(&(*Q)->get_cond, NULL); 
    if (err != 0)
        throwErr("Error: cannot initialize get cond variable!");
}

static void tsQueueLock(TSQueue* Q) {
    int err = pthread_mutex_lock(&Q->mutex);
    if (err != 0)
        throwErr("Error: mutex lock!");
}

static void tsQueueUnlock(TSQueue* Q) {
    int err = pthread_mutex_unlock(&Q->mutex);
    if (err != 0)
        throwErr("Error: mutex unlock!");
}

void tsQueueSet(TSQueue* Q, void* data) {
    tsQueueLock(Q);

    if (bitCheck(Q->wait_behavior, TS_WAIT_SET_BIT))
        while (tsQueueIsFull(Q) && Q->set_wait) {
            int err = pthread_cond_wait(&Q->set_cond, &Q->mutex);
            if (err != 0)
                throwErr("Error: cannot wait on set cond variable!");
        }

    if (!tsQueueIsFull(Q) && Q->set_wait) {
        TSQueueNode* new_node = (TSQueueNode*)malloc(sizeof(TSQueueNode));
        if (new_node == NULL)
            throwErr("Error: new node out of memmory!");

        new_node->data = malloc(Q->data_size);
        if (new_node == NULL)
            throwErr("Error: new node data out of memmory!");

        memcpy(new_node->data, data, Q->data_size);
        new_node->next = NULL;

        if (!tsQueueIsEmpty(Q)) {
            Q->tail->next = new_node;
            Q->tail = new_node;
        }
        else
            Q->head = Q->tail = new_node;

        ++Q->size;

        if (bitCheck(Q->wait_behavior, TS_WAIT_GET_BIT))
            pthread_cond_signal(&Q->get_cond);
    }

    tsQueueUnlock(Q);
}

void* tsQueueGet(TSQueue* Q) {
    void* data = NULL;

    tsQueueLock(Q);

    if (bitCheck(Q->wait_behavior, TS_WAIT_GET_BIT))
        while (tsQueueIsEmpty(Q) && Q->get_wait) {
            int err = pthread_cond_wait(&Q->get_cond, &Q->mutex);
            if (err != 0)
                throwErr("Error: cannot wait on get cond variable!");
        }

    if (!tsQueueIsEmpty(Q) && Q->get_wait) {
        data = Q->head->data;
        TSQueueNode* delete_node = Q->head;

        if (Q->head != Q->tail)
            Q->head = Q->head->next;
        else
            Q->head = Q->tail = NULL;

        if (Q->free_func)
            Q->free_func(delete_node->data);
        
        free(delete_node);

        --Q->size;

        if (bitCheck(Q->wait_behavior, TS_WAIT_SET_BIT))
            pthread_cond_signal(&Q->set_cond);
    }

    tsQueueUnlock(Q);

    return data;
}

void tsQueueSetWaitPrepare(TSQueue* Q) {
    tsQueueLock(Q);

    Q->set_wait = true;
    
    tsQueueUnlock(Q);
}

void tsQueueGetWaitPrepare(TSQueue* Q) {
    tsQueueLock(Q);

    Q->get_wait = true;
    
    tsQueueUnlock(Q);
}

void tsQueueSetWaitExit(TSQueue* Q) {
    tsQueueLock(Q);

    if (bitCheck(Q->wait_behavior, TS_WAIT_SET_BIT)) {
        Q->set_wait = false;
        pthread_cond_broadcast(&Q->set_cond);
    }
    
    tsQueueUnlock(Q);
}

void tsQueueGetWaitExit(TSQueue* Q) {
    tsQueueLock(Q);

    if (bitCheck(Q->wait_behavior, TS_WAIT_GET_BIT)) {
        Q->get_wait = false;
        pthread_cond_broadcast(&Q->get_cond);
    }

    tsQueueUnlock(Q);
}

static void tsQueueClearData(TSQueue* Q) {
    if (!tsQueueIsEmpty(Q)) {
        TSQueueNode* iter = Q->head;

        do {
            TSQueueNode* delete_node = iter;

            iter = iter->next;

            if (Q->free_func)
                Q->free_func(delete_node->data);

            free(delete_node);
        } while (iter);

        Q->head = Q->tail = NULL;
        Q->size = 0;
    }
}

void tsQueueClear(TSQueue* Q) {
    tsQueueLock(Q);

    tsQueueClearData(Q);
    if (bitCheck(Q->wait_behavior, TS_WAIT_SET_BIT))
        pthread_cond_broadcast(&Q->set_cond);

    tsQueueUnlock(Q);
}

void tsQueueFree(TSQueue** Q) {
    tsQueueClearData((*Q));

    pthread_mutex_destroy(&(*Q)->mutex);
    pthread_cond_destroy(&(*Q)->set_cond);
    pthread_cond_destroy(&(*Q)->get_cond);

    free((*Q));
    (*Q) = NULL;
}