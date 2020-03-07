#include "tsqueue.h"

int main(int argc, char* argv[]) {
    int val = 0;

    TSQueue* Q = NULL;
    tsQueueInit(&Q, TSQUEUE_SIZE_MAX, sizeof(int), NO_FREE_FUNC, TS_NO_WAIT);

    // Will be blocked at empty queue get
    //tsQueueInit(&Q, TSQUEUE_SIZE_MAX, sizeof(int), NO_FREE_FUNC, TS_GET_WAIT);
    
    val = 1;
    tsQueueSet(Q, (void*)&val);
    val = 2;
    tsQueueSet(Q, (void*)&val);
    val = 3;
    tsQueueSet(Q, (void*)&val);
    val = 4;
    tsQueueSet(Q, (void*)&val);

    printf("%d\n", *(int*)tsQueueGet(Q));
    printf("%d\n", *(int*)tsQueueGet(Q));
    printf("%d\n", *(int*)tsQueueGet(Q));
    printf("%d\n", *(int*)tsQueueGet(Q));

    if ((int*)tsQueueGet(Q) == NULL)
        printf("Empty\n");

    printf("%zu\n", Q->size);
    return 0;
}