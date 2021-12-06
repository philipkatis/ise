#include "ise_keyword_table.h"

#include <stdlib.h>
#include <string.h>

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
AllocateNode(char *Word, u32 Length, u32 Hash, keyword_table_node *Next)
{
    keyword_table_node *Node = (keyword_table_node *)calloc(1, sizeof(keyword_table_node));

    memcpy(Node->Data.Word, Word, Length * sizeof(char));
    Node->Data.Length = Length;
    Node->Data.Hash = Hash;
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

            u64 Index = Node->Data.Hash % BucketCount;
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

    u32 Length = strlen(Word);
    u32 Hash = DJB2(Word);
    u64 BucketIndex = Hash % Table->BucketCount;

    if (Table->Buckets[BucketIndex])
    {
        for (keyword_table_node *Node = Table->Buckets[BucketIndex];
             Node;
             Node = Node->Next)
        {
            if ((Node->Data.Hash == Hash) && (Node->Data.Length == Length) &&
                (memcmp(Node->Data.Word, Word, Length * sizeof(char)) == 0))
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

            Table->Buckets[BucketIndex] = AllocateNode(Word, Length, Hash, Table->Buckets[BucketIndex]);
            Result.Keyword = &Table->Buckets[BucketIndex]->Data;

            ++Table->ElementCount;
        }
    }
    else
    {
        Table->Buckets[BucketIndex] = AllocateNode(Word, Length, Hash, 0);
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

            QueryList_Destroy(&Node->Data.Queries);
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

function void
FindNextKeyword(keyword_iterator *Iterator)
{
    if (Iterator->Node)
    {
        Iterator->Node = Iterator->Node->Next;
    }
    else
    {
        while (Iterator->BucketIndex < Iterator->Table->BucketCount)
        {
            Iterator->Node = Iterator->Table->Buckets[Iterator->BucketIndex];
            if (Iterator->Node)
            {
                break;
            }

            ++Iterator->BucketIndex;
        }
    }
}

keyword_iterator
IterateAllKeywords(keyword_table *Table)
{
    keyword_iterator Iterator = { };
    Iterator.Table = Table;
    FindNextKeyword(&Iterator);

    return Iterator;
}

b32
IsValid(keyword_iterator *Iterator)
{
    return Iterator->Node != 0;
}

void Advance(keyword_iterator *Iterator)
{
    Assert(Iterator->Node);

    if (!Iterator->Node->Next)
    {
        ++Iterator->BucketIndex;
        Iterator->Node = 0;
    }

    FindNextKeyword(Iterator);
}

keyword *
GetValue(keyword_iterator *Iterator)
{
    Assert(Iterator->Node);
    return &Iterator->Node->Data;
}
