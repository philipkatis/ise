//
// NOTE(philip): Work Queue
//

function void
WorkQueue_Initialize(work_queue *Queue)
{
    Assert(Queue);
    *Queue = { };

    pthread_mutex_init(&Queue->Mutex, 0);
    pthread_cond_init(&Queue->HasSpace, 0);
    pthread_cond_init(&Queue->IsEmpty, 0);
}

function void
WorkQueue_Push(work_queue *Queue, work_type Type, u32 Data)
{
    pthread_mutex_lock(&Queue->Mutex);

    // NOTE(philip): If the queue is full, we block until there is space.
    while ((Queue->PendingWork + 1) == ISE_WORK_QUEUE_SIZE)
    {
        pthread_cond_wait(&Queue->HasSpace, &Queue->Mutex);
    }

    work *Work = Queue->Data + Queue->WriteIndex;
    Work->Type = Type;
    Work->Data = Data;

    Queue->WriteIndex = (((Queue->WriteIndex + 1) < ISE_WORK_QUEUE_SIZE) ? (Queue->WriteIndex + 1) : 0);
    ++Queue->PendingWork;

    // NOTE(philip): There is definitelly work in the queue, so we signal all the consumer threads.
    pthread_cond_broadcast(&Queue->IsEmpty);

    pthread_mutex_unlock(&Queue->Mutex);
}

function work
WorkQueue_Pop(work_queue *Queue)
{
    pthread_mutex_lock(&Queue->Mutex);

    // NOTE(philip): If the queue is empty, we block until there is work.
    while (Queue->PendingWork == 0)
    {
        pthread_cond_wait(&Queue->IsEmpty, &Queue->Mutex);
    }

    work Work = Queue->Data[Queue->ReadIndex];

    Queue->ReadIndex = (((Queue->ReadIndex + 1) < ISE_WORK_QUEUE_SIZE) ? (Queue->ReadIndex + 1) : 0);
    --Queue->PendingWork;

    // NOTE(philip): There is definitelly space in the queue, so we signal the producer.
    pthread_cond_signal(&Queue->HasSpace);

    pthread_mutex_unlock(&Queue->Mutex);

    return Work;
}

function void
WorkQueue_Destroy(work_queue *Queue)
{
    Assert(Queue);

    pthread_mutex_destroy(&Queue->Mutex);
    pthread_cond_destroy(&Queue->HasSpace);
    pthread_cond_destroy(&Queue->IsEmpty);
}

//
// NOTE(philip): Thread Pool
//

// TODO(philip): Remove this.
#include <unistd.h>

// TODO(philip): Move this.
function void *
ThreadFn(void *Arguments)
{
    thread_pool_memory *Memory = (thread_pool_memory *)Arguments;
    pthread_t ThreadHandle = pthread_self();

    for (;;)
    {
        work Work = WorkQueue_Pop(&Memory->Queue);

        if (Work.Type == WorkType_Exit)
        {
            break;
        }
        else if (Work.Type == WorkType_Actual)
        {
            // NOTE(philip): Simulate work.
            usleep(10000);
            //printf("[%lu]: Working on %d with %d...\n", ThreadHandle, Work.Type, Work.Data);
        }
        else
        {
            printf("hello\n");
        }
    }

    return 0;
}

function thread_pool
ThreadPool_Create(u32 ThreadCount)
{
    thread_pool Pool = { };
    Pool.ThreadCount = ThreadCount;
    Pool.Threads = (pthread_t *)calloc(1, Pool.ThreadCount * sizeof(pthread_t));
    Pool.Memory = (thread_pool_memory *)calloc(1, sizeof(thread_pool_memory));

    WorkQueue_Initialize(&Pool.Memory->Queue);

    for (u32 Index = 0;
         Index < Pool.ThreadCount;
         ++Index)
    {
        pthread_create(Pool.Threads + Index, 0, ThreadFn, Pool.Memory);
    }

    return Pool;
}

function void
ThreadPool_Destroy(thread_pool *Pool)
{
    Assert(Pool);

    for (u32 Index = 0;
         Index < Pool->ThreadCount;
         ++Index)
    {
        // NOTE(philip): Submit one work unit for each thread, informing them to stop execution.
        WorkQueue_Push(&Pool->Memory->Queue, WorkType_Exit, 0);
    }

    for (u32 Index = 0;
         Index < Pool->ThreadCount;
         ++Index)
    {
        pthread_join(Pool->Threads[Index], 0);
    }

    WorkQueue_Destroy(&Pool->Memory->Queue);

    free(Pool->Threads);
    free(Pool->Memory);

    Pool->ThreadCount = 0;
    Pool->Threads = 0;
    Pool->Memory = 0;
}
