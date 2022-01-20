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

#if 0
struct answer_stack_node
{
    answer Answer;
    answer_stack_node *Next;
};

// TODO(philip): Replace with circular buffer.
struct answer_stack
{
    answer_stack_node *Head;
    u64 Count;
};

function void AnswerStack_Push(answer_stack *Stack, answer Answer);
function answer AnswerStack_Pop(answer_stack *Stack);
function void AnswerStack_Destroy(answer_stack *Stack);
#endif

#endif
