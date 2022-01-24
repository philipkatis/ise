#ifndef ISE_WORK_H
#define ISE_WORK_H

struct document_answer
{
    u32 DocumentID;
    u32 *Queries;
    u64 QueryCount;
};

struct document_answer_stack
{
    u64 Capacity;
    u64 Count;
    document_answer *Data;
};

#endif
