#ifndef ISE_WORK_H
#define ISE_WORK_H

struct document_answer
{
    u32 ID;
    u32 *Queries;
    u64 QueryCount;
};

struct document_answer_stack
{
    u64 Capacity;
    u64 Count;
    document_answer *Data;
};

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)

struct context
{
    query_tree Queries;

    keyword_table Keywords;
    keyword_tree HammingTrees[HAMMING_TREE_COUNT];
    keyword_tree EditTree;

    document_answer_stack DocumentAnswers;
};

global context GlobalContext = { };

#endif
