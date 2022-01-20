#ifndef ISE_THREAD_POOL_H
#define ISE_THREAD_POOL_H

#define ISE_WORK_QUEUE_SIZE 1024

typedef u32 work_type;
enum
{
    WorkType_None              = 0,
    WorkType_MatchDocument     = 1,
    WorkType_Exit              = 2
};

struct work
{
    work_type Type;
    u8 *Data;
};

struct work_queue
{
    work Data[ISE_WORK_QUEUE_SIZE];
    u32 WriteIndex;
    u32 ReadIndex;
    u32 Count;

    pthread_mutex_t Mutex;
    pthread_cond_t HasSpace;
    pthread_cond_t HasData;
};

struct thread_pool_memory
{
    work_queue Queue;
    keyword_table *Keywords;
    bk_tree *HammingTrees;
    bk_tree *EditTree;
    result_queue *Results;
};

struct thread_pool
{
    u32 ThreadCount;
    pthread_t *Threads;

    thread_pool_memory *Memory;
};

#endif
