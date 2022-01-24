#define GetHammingTreeIndex(Keyword) ((Keyword)->Length - MIN_KEYWORD_LENGTH)

function void
RegisterQuery(query_tree *Queries, keyword_table *Keywords, keyword_tree *HammingTrees, keyword_tree *EditTree,
              u32 ID, query_type Type, u64 DistanceThreshold, char *String)
{
    char *Words[MAX_KEYWORD_COUNT_PER_QUERY];
    u64 WordCount = 0;

    Words[WordCount++] = String;

    for (char *Character = String;
         *Character;
         ++Character)
    {
        if (*Character == ' ')
        {
            *Character = 0;
            Words[WordCount++] = Character + 1;
        }
    }

    query *Query = InsertIntoQueryTree(Queries, ID, WordCount, Type, DistanceThreshold);

    switch (Type)
    {
        case QueryType_Exact:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(Keywords, Words[Index]);
                ++Keyword->InstanceCount;

                Query->Keywords[Index] = Keyword;
            }
        } break;

        case QueryType_Hamming:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(Keywords, Words[Index]);

                if (!Keyword->HammingInstanceCount)
                {
                    keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Keyword);
                    InsertIntoKeywordTree(Tree, Keyword);
                }

                ++Keyword->InstanceCount;
                ++Keyword->HammingInstanceCount;

                Query->Keywords[Index] = Keyword;
            }
        } break;

        case QueryType_Edit:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(Keywords, Words[Index]);

                if (!Keyword->EditInstanceCount)
                {
                    InsertIntoKeywordTree(EditTree, Keyword);
                }

                ++Keyword->InstanceCount;
                ++Keyword->EditInstanceCount;

                Query->Keywords[Index] = Keyword;
            }
        } break;
    }
}

function void
UnregisterQuery(query_tree *Queries, keyword_tree *HammingTrees, keyword_tree *EditTree, u32 ID)
{
    query *Query = FindQueryInTree(Queries, ID);

    query_type Type = GetType(Query);
    u64 KeywordCount = GetKeywordCount(Query);

    switch (Type)
    {
        case QueryType_Exact:
        {
            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                keyword *Keyword = Query->Keywords[Index];

                --Keyword->InstanceCount;

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;

        case QueryType_Hamming:
        {
            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                keyword *Keyword = Query->Keywords[Index];

                --Keyword->HammingInstanceCount;
                --Keyword->InstanceCount;

                if (!Keyword->HammingInstanceCount)
                {
                    keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Keyword);
                    RemoveFromKeywordTree(Tree, Keyword);
                }

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;

        case QueryType_Edit:
        {
            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                keyword *Keyword = Query->Keywords[Index];

                --Keyword->EditInstanceCount;
                --Keyword->InstanceCount;

                if (!Keyword->EditInstanceCount)
                {
                    RemoveFromKeywordTree(EditTree, Keyword);
                }

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;
    }

    RemoveFromQueryTree(Queries, ID);
}
