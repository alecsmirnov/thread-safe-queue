#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "tsqueue.h"

#define THREADS_COUNT 4

void* getThread(void* arg) {
    TSQueue* Q = (TSQueue*)arg;

    void* data = NULL;
    do {
        data = tsQueueGet(Q);
    } while (Q->get_wait && data);

    printf("Closed\n");
    pthread_exit(NULL);
}

void* closeThread(void* arg) {
    TSQueue* Q = (TSQueue*)arg;

    while (!tsQueueIsEmpty(Q))
        sleep(1);

    // Close waiting gets
    tsQueueGetWaitExit(Q);
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    TSQueue* Q = NULL;

    // Shutdown when queue is empty/full
    //tsQueueInit(&Q, TSQUEUE_SIZE_MAX, sizeof(int), NO_FREE_FUNC, TS_NO_WAIT);

    // Will wait for get until it receives a exit signal
    tsQueueInit(&Q, TSQUEUE_SIZE_MAX, sizeof(int), NO_FREE_FUNC, TS_WAIT_GET);

    for (int i = 0; i != THREADS_COUNT - 1; ++i)
        tsQueueSet(Q, (void*)&i);

    pthread_t threads[THREADS_COUNT];
    for (uint8_t i = 0; i != THREADS_COUNT - 1; ++i)
        pthread_create(&threads[i], NULL, getThread, (void*)Q);

    pthread_create(&threads[THREADS_COUNT - 1], NULL, closeThread, (void*)Q);

    for (uint8_t i = 0; i != THREADS_COUNT; ++i)
        pthread_join(threads[i], NULL);

    tsQueueFree(&Q);

    return 0;
}