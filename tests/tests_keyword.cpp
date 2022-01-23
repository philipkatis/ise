global keyword_tree_match KeywordTreeMatches[KEYWORD_TREE_MATCH_STORAGE_SIZE];
global u64 KeywordTreeMatchCount = 0;

function void
KeywordTree(void)
{
    // NOTE(philip): Hamming Tree.
    {
        char *Words[10] =
        {
            "halt", "oouc", "pops", "oops", "felt", "fell", "smel", "shel", "help", "hell"
        };

        keyword_table Keywords = KeywordTable_Create(64);

        for (u64 Index = 0;
             Index < ArrayCount(Words);
             ++Index)
        {
            KeywordTable_Insert(&Keywords, Words[Index]);
        }

        keyword_tree Tree;
        InitializeKeywordTree(&Tree, KeywordTreeType_Hamming);

        TEST_CHECK(Tree.Root == 0);

        for (keyword_iterator Iterator = IterateAllKeywords(&Keywords);
             IsValid(&Iterator);
             Advance(&Iterator))
        {
            keyword *Keyword = GetValue(&Iterator);
            InsertIntoKeywordTree(&Tree, Keyword);
        }

        {
            char *Solutions[4] =
            {
                Words[0], Words[4], Words[8], Words[9]
            };

            keyword_table SearchValues = KeywordTable_Create(1);
            keyword_table_insert_result InsertResult = KeywordTable_Insert(&SearchValues, "helt");

            FindMatchesInKeywordTree(&Tree, InsertResult.Keyword, &KeywordTreeMatchCount, KeywordTreeMatches);
            KeywordTable_Destroy(&SearchValues);

            for (u64 SolutionIndex = 0;
                 SolutionIndex < ArrayCount(Solutions);
                 ++SolutionIndex)
            {
                b32 Found = false;

                for (u64 Index = 0;
                     Index < KeywordTreeMatchCount;
                     ++Index)
                {
                    keyword_tree_match *Match = KeywordTreeMatches + Index;
                    if (strcmp(Match->Keyword->Word, Solutions[SolutionIndex]) == 0)
                    {
                        Found = true;
                        break;
                    }
                }

                TEST_CHECK(Found == true);
            }
        }

        {
            keyword_table SearchValues = KeywordTable_Create(1);
            keyword_table_insert_result InsertResult = KeywordTable_Insert(&SearchValues, "opsy");

            FindMatchesInKeywordTree(&Tree, InsertResult.Keyword, &KeywordTreeMatchCount, KeywordTreeMatches);
            KeywordTable_Destroy(&SearchValues);
        }

        DestroyKeywordTree(&Tree);
        TEST_CHECK(Tree.Root == 0);

        KeywordTable_Destroy(&Keywords);
    }

    // NOTE(philip): Edit Tree.
    {
        char *Words[10] =
        {
            "halt", "oouch", "pop", "oops", "felt", "fell", "smell", "shell", "help", "hell"
        };

        keyword_table Keywords = KeywordTable_Create(64);

        for (u64 WordIndex = 0;
             WordIndex < ArrayCount(Words);
             ++WordIndex)
        {
            KeywordTable_Insert(&Keywords, Words[WordIndex]);
        }

        keyword_tree Tree;
        InitializeKeywordTree(&Tree, KeywordTreeType_Edit);

        TEST_CHECK(Tree.Root == 0);

        for (keyword_iterator Iterator = IterateAllKeywords(&Keywords);
             IsValid(&Iterator);
             Advance(&Iterator))
        {
            keyword *Keyword = GetValue(&Iterator);
            InsertIntoKeywordTree(&Tree, Keyword);
        }

        {
            char *Solutions[6] =
            {
                Words[0], Words[4], Words[5], Words[7], Words[8], Words[9]
            };

            keyword_table SearchValues = KeywordTable_Create(1);
            keyword_table_insert_result InsertResult = KeywordTable_Insert(&SearchValues, "helt");

            FindMatchesInKeywordTree(&Tree, InsertResult.Keyword, &KeywordTreeMatchCount, KeywordTreeMatches);
            KeywordTable_Destroy(&SearchValues);

            for (u64 SolutionIndex = 0;
                 SolutionIndex < ArrayCount(Solutions);
                 ++SolutionIndex)
            {
                b32 Found = false;

                for (u64 Index = 0;
                     Index < KeywordTreeMatchCount;
                     ++Index)
                {
                    keyword_tree_match *Match = KeywordTreeMatches + Index;
                    if (strcmp(Match->Keyword->Word, Solutions[SolutionIndex]) == 0)
                    {
                        Found = true;
                        break;
                    }
                }

                TEST_CHECK(Found == true);
            }
        }

        {
            char *Solutions[2] =
            {
                Words[2], Words[3]
            };

            keyword_table SearchValues = KeywordTable_Create(1);
            keyword_table_insert_result InsertResult = KeywordTable_Insert(&SearchValues, "ops");

            FindMatchesInKeywordTree(&Tree, InsertResult.Keyword, &KeywordTreeMatchCount, KeywordTreeMatches);
            KeywordTable_Destroy(&SearchValues);

            for (u64 SolutionIndex = 0;
                 SolutionIndex < ArrayCount(Solutions);
                 ++SolutionIndex)
            {
                b32 Found = false;

                for (u64 Index = 0;
                     Index < KeywordTreeMatchCount;
                     ++Index)
                {
                    keyword_tree_match *Match = KeywordTreeMatches + Index;
                    if (strcmp(Match->Keyword->Word, Solutions[SolutionIndex]) == 0)
                    {
                        Found = true;
                        break;
                    }
                }

                TEST_CHECK(Found == true);
            }
        }

        DestroyKeywordTree(&Tree);
        KeywordTable_Destroy(&Keywords);
    }
}
