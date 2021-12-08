#ifndef ISE_ANSWER_LIST_H
#define ISE_ANSWER_LIST_H

#include "ise_base.h"

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

struct answer_stack
{
    answer_stack_node *Head;
    u64 Count = 0;
};

void AnswerStack_Push(answer_stack *Stack, answer Answer);
answer AnswerStack_Pop(answer_stack *Stack);
void AnswerStack_Destroy(answer_stack *Stack);

#endif
