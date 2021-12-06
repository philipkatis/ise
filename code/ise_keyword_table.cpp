#include "ise_keyword_table.h"

#include <stdlib.h>
#include <string.h>

// TODO(philip): Should we sort the words in some way in the linked list. Maybe sort by hash?? Probably won't be
// a problem though.

// TODO(philip): Currently using the DJB2 hash algorithm. If this is not good enough switch to murmur3.
function u32
DJB2(char *String)
{
    u32 Result = 5381;

    for (char *Character = String;
         *Character;
         ++Character)
    {
        Result = ((Result << 5) + Result) + *Character;
    }

    return Result;
}

function keyword_table_node *
AllocateNode(char *Word, keyword_table_node *Next)
{
    keyword_table_node *Node = (keyword_table_node *)calloc(1, sizeof(keyword_table_node));

    // TODO(philip): Switch to memcpy if we decide to store the word length.
    strcpy(Node->Data.Word, Word);
    Node->Next = Next;

    return Node;
}

keyword_table
KeywordTable_Create(u64 InitialBucketCount)
{
    keyword_table Table = { };
    Table.Buckets = (keyword_table_node **)calloc(1, InitialBucketCount * sizeof(keyword_table_node *));
    Table.BucketCount = InitialBucketCount;

    return Table;
}

function void
Rehash(keyword_table *Table)
{
    // TODO(philip): The new bucket count should probably be a prime number.
    u64 BucketCount = Table->BucketCount * 2;
    keyword_table_node **Buckets = (keyword_table_node **)calloc(1, BucketCount * sizeof(keyword_table_node *));

    for (u64 BucketIndex = 0;
         BucketIndex < Table->BucketCount;
         ++BucketIndex)
    {
        keyword_table_node *Node = Table->Buckets[BucketIndex];
        while (Node)
        {
            keyword_table_node *Next = Node->Next;

            // TODO(philip): Do not recalculate the hash.
            u32 Hash = DJB2(Node->Data.Word);
            u64 Index = Hash % BucketCount;

            Node->Next = Buckets[Index];
            Buckets[Index] = Node;

            Node = Next;
        }
    }

    free(Table->Buckets);

    Table->Buckets = Buckets;
    Table->BucketCount = BucketCount;
}

keyword_table_insert_result
KeywordTable_Insert(keyword_table *Table, char *Word)
{
    keyword_table_insert_result Result = { };

    u32 Hash = DJB2(Word);
    u64 BucketIndex = Hash % Table->BucketCount;

    if (Table->Buckets[BucketIndex])
    {
        for (keyword_table_node *Node = Table->Buckets[BucketIndex];
             Node;
             Node = Node->Next)
        {
            // TODO(philip): If we decide to store more data in the keyword, first compare length and hash and then
            // the word itself. Also switch to memcmp.
            if (strcmp(Node->Data.Word, Word) == 0)
            {
                Result.Keyword = &Node->Data;
                Result.Exists = true;

                break;
            }
        }

        if (!Result.Exists)
        {
            // TODO(philip): Investigate what load factor works.
            f32 LoadFactor = (f32)(Table->ElementCount + 1 ) / (f32)Table->BucketCount;
            if (LoadFactor > 0.85f)
            {
                Rehash(Table);
            }

            Table->Buckets[BucketIndex] = AllocateNode(Word, Table->Buckets[BucketIndex]);
            Result.Keyword = &Table->Buckets[BucketIndex]->Data;

            ++Table->ElementCount;
        }
    }
    else
    {
        Table->Buckets[BucketIndex] = AllocateNode(Word, 0);
        Result.Keyword = &Table->Buckets[BucketIndex]->Data;

        ++Table->ElementCount;
    }

    return Result;
}

void
KeywordTable_Destroy(keyword_table *Table)
{
    for (u64 BucketIndex = 0;
         BucketIndex < Table->BucketCount;
         ++BucketIndex)
    {
        keyword_table_node *Node = Table->Buckets[BucketIndex];
        while (Node)
        {
            keyword_table_node *Next = Node->Next;
            free(Node);
            Node = Next;
        }
    }

    free(Table->Buckets);

    Table->Buckets = 0;
    Table->BucketCount = 0;
    Table->ElementCount = 0;
}

#if ISE_DEBUG
    void
    KeywordTable_Visualize_(keyword_table *Table)
    {
        for (u64 BucketIndex = 0;
             BucketIndex < Table->BucketCount;
             ++BucketIndex)
        {
            keyword_table_node *BucketHead = Table->Buckets[BucketIndex];
            if (BucketHead)
            {
                printf("%llu: ", BucketIndex);

                for (keyword_table_node *Node = BucketHead;
                     Node;
                     Node = Node->Next)
                {
                    printf("%s", Node->Data.Word);

                    if (Node->Next)
                    {
                        printf(" -> ");
                    }
                }

                printf("\n");
            }
        }

        printf("\nBucket Count: %llu\n", Table->BucketCount);
        printf("Element Count: %llu\n", Table->ElementCount);
        printf("Load Factor: %f\n", (f32)Table->ElementCount / (f32)Table->BucketCount);
    }
#endif
