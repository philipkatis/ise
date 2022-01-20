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

#if 0

function answer_stack
AnswerStack_Create()
{
    answer_stack Stack = { };
    return Stack;
}

function void
AnswerStack_Push(answer_stack *Stack, answer Answer)
{
    answer_stack_node *Node = (answer_stack_node *)calloc(1, sizeof(answer_stack_node));
    Node->Answer = Answer;
    Node->Next = Stack->Head;

    Stack->Head = Node;
    ++Stack->Count;
}

function answer
AnswerStack_Pop(answer_stack *Stack)
{
    answer Answer = { };

    if (Stack->Head)
    {
        Answer = Stack->Head->Answer;

        answer_stack_node *Next = Stack->Head->Next;
        free(Stack->Head);
        Stack->Head = Next;

        --Stack->Count;
    }

    return Answer;
}

function void
AnswerStack_Destroy(answer_stack *Stack)
{
    answer_stack_node *Node = Stack->Head;
    while (Node)
    {
        answer_stack_node *Next = Node->Next;
        free(Node);
        Node = Next;
    }

    Stack->Head = 0;
    Stack->Count = 0;
}

#endif
