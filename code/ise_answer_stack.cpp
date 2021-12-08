#include "ise_answer_stack.h"

#include <stdlib.h>

void
AnswerStack_Push(answer_stack *Stack, answer Answer)
{
    answer_stack_node *Node = (answer_stack_node *)calloc(1, sizeof(answer_stack_node));
    Node->Answer = Answer;
    Node->Next = Stack->Head;

    Stack->Head = Node;
    ++Stack->Count;
}

answer
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

void
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
