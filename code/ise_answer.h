#ifndef ISE_ANSWER_LIST_H
#define ISE_ANSWER_LIST_H

struct answer
{
    u32 DocumentID;
    u32 QueryIDCount;
    u32 *QueryIDs;
};

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
