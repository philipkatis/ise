#include "ise_keyword_table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// NOTE(philip): DJB2 Hash Function
function u64
HashKeyword(char *Word)
{
    u64 Result = 5381;

    for (char *Character = Word;
         *Character;
         ++Character)
    {
        Result = ((Result << 5) + Result) + *Character;
    }

    return Result;
}

keyword_table_node *
KeywordTable_Find(keyword_table *Table, char *Word, u64 Hash)
{
    keyword_table_node *Result = 0;

    u64 Index = Hash % KEYWORD_TABLE_BUCKET_COUNT;

    for (keyword_table_node *Node = Table->Buckets[Index];
         Node;
         Node = Node->Next)
    {
        if ((Hash == Node->Hash) && (strcmp(Word, Node->Word) == 0))
        {
            Result = Node;
            break;
        }
    }

    return Result;
}

keyword_table_node *
KeywordTable_Find(keyword_table *Table, char *Word)
{
    u64 Hash = HashKeyword(Word);
    keyword_table_node *Result = KeywordTable_Find(Table, Word, Hash);
    return Result;
}

keyword_table_node *
KeywordTable_Insert(keyword_table *Table, char *Word)
{
    u64 Hash = HashKeyword(Word);
    u64 Index = Hash % KEYWORD_TABLE_BUCKET_COUNT;

    keyword_table_node *Node = (keyword_table_node *)calloc(1, sizeof(keyword_table_node));
    strcpy(Node->Word, Word);
    Node->Hash = Hash;

    Node->Next = Table->Buckets[Index];
    Table->Buckets[Index] = Node;

    return Node;
}

void KeywordTable_Destroy(keyword_table *Table)
{
    for (u64 Index = 0;
         Index < KEYWORD_TABLE_BUCKET_COUNT;
         ++Index)
    {
        keyword_table_node *Node = Table->Buckets[Index];
        while (Node)
        {
            keyword_table_node *Next = Node->Next;

            QueryList_Destroy(&Node->Queries);
            free(Node);

            Node = Next;
        }
    }
}


#if ISE_DEBUG
    void
    KeywordTable_Visualize_(keyword_table *Table)
    {
        for (u64 BucketIndex = 0;
             BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
             ++BucketIndex)
        {
            if (Table->Buckets[BucketIndex])
            {
                printf("%llu : { ", BucketIndex);

                u64 NodeIndex = 0;
                for (keyword_table_node *Node = Table->Buckets[BucketIndex];
                     Node;
                     Node = Node->Next)
                {
                    if (NodeIndex > 0)
                    {
                        printf("} -> { ");
                    }

                    printf("%llu, %s (", NodeIndex, Node->Word);

                    for (query *Query = Node->Queries.Head;
                         Query;
                         Query = Query->Next)
                    {
                        printf("%d, ", Query->ID);
                    }

                    printf(") ");

                    ++NodeIndex;
                }

                printf("}\n");
            }
        }
    }
#endif
