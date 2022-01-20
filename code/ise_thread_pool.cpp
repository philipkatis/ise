//
// NOTE(philip): Initializes a work queue.
//

function void
InitializeWorkQueue(work_queue *Queue)
{
    Assert(Queue);

    // NOTE(philip): Initialize the synchronization objects.
    pthread_mutex_init(&Queue->Mutex, 0);
    pthread_cond_init(&Queue->HasSpace, 0);
    pthread_cond_init(&Queue->HasData, 0);
}

//
// NOTE(philip): Pushes work into the work queue. Blocks if the queue is full and signals thraeds when there is'
// data in the queue.
//

function void
PushWork(work_queue *Queue, work_type Type, u8 *Data)
{
    pthread_mutex_lock(&Queue->Mutex);

    // NOTE(philip): If the queue is full, we block until there is space.
    while ((Queue->Count + 1) == ISE_WORK_QUEUE_SIZE)
    {
        pthread_cond_wait(&Queue->HasSpace, &Queue->Mutex);
    }

    // NOTE(philip): Copy the work data into the queue.
    work *Work = Queue->Data + Queue->WriteIndex++;
    Work->Type = Type;
    Work->Data = Data;

    // NOTE(philip): Update the queue indices.
    Queue->WriteIndex = ((Queue->WriteIndex < ISE_WORK_QUEUE_SIZE) ? Queue->WriteIndex : 0);
    ++Queue->Count;

    // NOTE(philip): There is definitelly work in the queue, so we signal all the consumer threads.
    pthread_cond_broadcast(&Queue->HasData);

    pthread_mutex_unlock(&Queue->Mutex);
}

//
// NOTE(philip): Pops work from the work queue. Blocks if the queue is empty and signals the main thread when
// there is space in the queue.
//

function work
PopWork(work_queue *Queue)
{
    pthread_mutex_lock(&Queue->Mutex);

    // NOTE(philip): If the queue is empty, we block until there is work.
    while (Queue->Count == 0)
    {
        pthread_cond_wait(&Queue->HasData, &Queue->Mutex);
    }

    // NOTE(philip): Make a copy of the work.
    work Work = Queue->Data[Queue->ReadIndex++];

    // NOTE(philip): Update the queue indices.
    Queue->ReadIndex = ((Queue->ReadIndex < ISE_WORK_QUEUE_SIZE) ? Queue->ReadIndex : 0);
    --Queue->Count;

    // NOTE(philip): There is definitelly space in the queue, so we signal the producer.
    pthread_cond_signal(&Queue->HasSpace);

    pthread_mutex_unlock(&Queue->Mutex);

    return Work;
}

//
// NOTE(philip): Destroys a work queue.
//

function void
DestroyWorkQueue(work_queue *Queue)
{
    Assert(Queue);

    // NOTE(philip): Destroy the synchronization primitives.
    pthread_mutex_destroy(&Queue->Mutex);
    pthread_cond_destroy(&Queue->HasSpace);
    pthread_cond_destroy(&Queue->HasData);
}

//
// NOTE(philip): Thread Pool
//

// TODO(philip): Move this.
function void *
ThreadFn(void *Arguments)
{
    thread_pool_memory *Memory = (thread_pool_memory *)Arguments;

    b32 Running = true;
    while (Running)
    {
        work Work = PopWork(&Memory->Queue);

        switch (Work.Type)
        {
            case WorkType_MatchDocument:
            {
                u32 ID = *(u32 *)Work.Data;
                char *Document = (char *)(Work.Data + sizeof(u32));

                FindDocumentAnswer(Memory->Results, Memory->Keywords, Memory->HammingTrees, Memory->EditTree,
                                   ID, Document);

                // NOTE(philip): Free the work data.
                free(Work.Data);
            } break;

            case WorkType_Exit:
            {
                Running = false;
            } break;

            default:
            {
                Assert(false);
            } break;
        }
    }

    return 0;
}

function thread_pool
ThreadPool_Create(u32 ThreadCount, keyword_table *Keywords, bk_tree *HammingTrees, bk_tree *EditTree,
                  result_queue *Results)
{
    thread_pool Pool = { };
    Pool.ThreadCount = ThreadCount;
    Pool.Threads = (pthread_t *)calloc(1, Pool.ThreadCount * sizeof(pthread_t));
    Pool.Memory = (thread_pool_memory *)calloc(1, sizeof(thread_pool_memory));

    InitializeWorkQueue(&Pool.Memory->Queue);
    Pool.Memory->Keywords = Keywords;
    Pool.Memory->HammingTrees = HammingTrees;
    Pool.Memory->EditTree = EditTree;
    Pool.Memory->Results = Results;

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

    // NOTE(philip): Submit one work unit for each thread, informing them to stop execution.
    for (u32 Index = 0;
         Index < Pool->ThreadCount;
         ++Index)
    {
        PushWork(&Pool->Memory->Queue, WorkType_Exit, 0);
    }

    // NOTE(philip): Wait for each thread to exit.
    for (u32 Index = 0;
         Index < Pool->ThreadCount;
         ++Index)
    {
        pthread_join(Pool->Threads[Index], 0);
    }

    DestroyWorkQueue(&Pool->Memory->Queue);

    free(Pool->Threads);
    free(Pool->Memory);

    Pool->ThreadCount = 0;
    Pool->Threads = 0;
    Pool->Memory = 0;
}
