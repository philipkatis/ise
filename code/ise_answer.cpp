function answer_stack
AnswerStack_Create()
{
    answer_stack Stack = { };
    pthread_mutex_init(&Stack.Mutex, 0);
    pthread_cond_init(&Stack.HasData, 0);

    return Stack;
}

function void
AnswerStack_Push(answer_stack *Stack, answer Answer)
{
    pthread_mutex_lock(&Stack->Mutex);

    answer_stack_node *Node = (answer_stack_node *)calloc(1, sizeof(answer_stack_node));
    Node->Answer = Answer;
    Node->Next = Stack->Head;

    Stack->Head = Node;
    ++Stack->Count;

    pthread_cond_signal(&Stack->HasData);

    pthread_mutex_unlock(&Stack->Mutex);
}

function answer
AnswerStack_Pop(answer_stack *Stack)
{
    pthread_mutex_lock(&Stack->Mutex);

    while (!Stack->Head)
    {
        pthread_cond_wait(&Stack->HasData, &Stack->Mutex);
    }

    answer Answer = Stack->Head->Answer;

    answer_stack_node *Next = Stack->Head->Next;
    free(Stack->Head);
    Stack->Head = Next;

    --Stack->Count;

    pthread_mutex_unlock(&Stack->Mutex);

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

    pthread_mutex_destroy(&Stack->Mutex);
    pthread_cond_destroy(&Stack->HasData);

    Stack->Head = 0;
    Stack->Count = 0;
}
