#ifndef ISE_RESULT_QUEUE_H
#define ISE_RESULT_QUEUE_H

struct result
{
    u32 DocumentID;
    u32 QueryIDCount;
    u32 *QueryIDs;
};

#define ISE_RESULT_QUEUE_SIZE 1024

struct result_queue
{
    result Data[1024];
    u32 ReadIndex;
    u32 WriteIndex;
    u32 Count;

#if ISE_MULTI_THREADED
    pthread_mutex_t Mutex;
    pthread_cond_t HasSpace;
    pthread_cond_t HasData;
#endif
};

#endif
