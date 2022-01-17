#ifndef ISE_THREAD_POOL_H
#define ISE_THREAD_POOL_H

#define ISE_WORK_QUEUE_SIZE 1024

typedef u32 work_type;
enum
{
    WorkType_None = 0,
    WorkType_Exit = 1,
    WorkType_Actual = 2
};

struct work
{
    work_type Type;
    u32 Data;
};

struct work_queue
{
    work Data[ISE_WORK_QUEUE_SIZE];
    u32 WriteIndex;
    u32 ReadIndex;
    u32 PendingWork;

    pthread_mutex_t Mutex;
    pthread_cond_t HasSpace;
    pthread_cond_t IsEmpty;
};

struct thread_pool_memory
{
    work_queue Queue;
};

struct thread_pool
{
    u32 ThreadCount;
    pthread_t *Threads;

    thread_pool_memory *Memory;
};

#endif
