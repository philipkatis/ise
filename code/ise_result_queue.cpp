function void
InitializeResultQueue(result_queue *Queue)
{
#if ISE_MULTI_THREADED
    pthread_mutex_init(&Queue->Mutex, 0);
    pthread_cond_init(&Queue->HasSpace, 0);
    pthread_cond_init(&Queue->HasData, 0);
#endif
}

function void
PushResult(result_queue *Queue, result Result)
{
#if ISE_MULTI_THREADED
    pthread_mutex_lock(&Queue->Mutex);

    while (Queue->Count == ISE_RESULT_QUEUE_SIZE)
    {
        pthread_cond_wait(&Queue->HasSpace, &Queue->Mutex);
    }
#else
    Assert(Queue->Count < ISE_RESULT_QUEUE_SIZE);
#endif

    Queue->Data[Queue->WriteIndex++] = Result;
    Queue->WriteIndex = ((Queue->WriteIndex < ISE_RESULT_QUEUE_SIZE) ? Queue->WriteIndex : 0);
    ++Queue->Count;

#if ISE_MULTI_THREADED
    pthread_cond_signal(&Queue->HasData);

    pthread_mutex_unlock(&Queue->Mutex);
#endif
}

function result
PopResult(result_queue *Queue)
{
#if ISE_MULTI_THREADED
    pthread_mutex_lock(&Queue->Mutex);

    while (Queue->Count == 0)
    {
        pthread_cond_wait(&Queue->HasData, &Queue->Mutex);
    }
#else
    Assert(Queue->Count > 0);
#endif

    result Result = Queue->Data[Queue->ReadIndex++];
    Queue->ReadIndex = ((Queue->ReadIndex < ISE_RESULT_QUEUE_SIZE) ? Queue->ReadIndex : 0);
    --Queue->Count;

#if ISE_MULTI_THREADED
    pthread_cond_broadcast(&Queue->HasSpace);

    pthread_mutex_unlock(&Queue->Mutex);
#endif

    return Result;
}

function void
DestroyResultQueue(result_queue *Queue)
{
#if ISE_MULTI_THREADED
    pthread_mutex_destroy(&Queue->Mutex);
    pthread_cond_destroy(&Queue->HasSpace);
    pthread_cond_destroy(&Queue->HasData);
#endif
}
