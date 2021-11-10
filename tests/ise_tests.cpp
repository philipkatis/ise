#include <stdlib.h>
#include <stdio.h>

// NOTE(philip): Linux decides to include 'strings.h' by default, preventing us from defining an 'index' type.
#ifdef __USE_MISC
    #undef __USE_MISC
#endif

#include <string.h>


#include "ise_base.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_match.cpp"
#include "ise.cpp"

#include "acutest.h"

function void
Test_IsExactMatch(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Different words. Same length.
    {
        A = "hello";
        B = "world";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words. Different length.
    {
        A = "000x00010f001";
        B = "rgjahrg";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Max string length.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }
}

function void
Test_CalculateHammingDistance(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

function void
Test_CalculateLevenshteinDistance(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Test some regular cases, since this function is more complex than the other keyword matching
    // functions.
    {
        A = "kitten";
        B = "sitten";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sitten";
        B = "sittin";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sittin";
        B = "sitting";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "bravo";
        B = "raven";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 3);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

function void
Test_EntryList(void)
{
    error_code ErrorCode = ErrorCode_Success;

#define KEYWORD_COUNT 100
    char Words[KEYWORD_COUNT][MAX_KEYWORD_LENGTH + 1];
    entry Entries[KEYWORD_COUNT];
    entry_list List = { };

    // NOTE(philip): Initialize the word array.
    for (u64 Index = 0;
         Index < KEYWORD_COUNT;
         ++Index)
    {
        sprintf(Words[Index], "Word%llu", Index + 1);
    }

    // NOTE(philip): List creation.
    {
        ErrorCode = create_entry_list(0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        ErrorCode = create_entry_list(&List);
        TEST_CHECK(ErrorCode == ErrorCode_Success);
        TEST_CHECK(List.Head == 0);
        TEST_CHECK(List.Count == 0);
    }

    // NOTE(philip): Entry creation.
    {
        ErrorCode = create_entry(0, 0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        for (u64 Index = 0;
             Index < KEYWORD_COUNT;
             ++Index)
        {
            ErrorCode = create_entry(Words[Index], &Entries[Index]);
            TEST_CHECK(ErrorCode == ErrorCode_Success);
            TEST_CHECK(Entries[Index] != 0);
            TEST_CHECK(Entries[Index]->Word == Words[Index]);
            TEST_CHECK(Entries[Index]->Next == 0);
        }
    }

    // NOTE(philip): Entry insertion.
    {
        ErrorCode = add_entry(0, 0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        for (u64 Index = 0;
             Index < KEYWORD_COUNT;
             ++Index)
        {
            ErrorCode = add_entry(&List, &Entries[Index]);
            TEST_CHECK(ErrorCode == ErrorCode_Success);
            TEST_CHECK(List.Head == Entries[Index]);
            TEST_CHECK(List.Count == Index + 1);

            if (Index > 0)
            {
                TEST_CHECK(Entries[Index]->Next == Entries[Index - 1]);
            }
        }

        entry TestEntry1 = 0;
        entry TestEntry67 = 0;
        entry TestEntry100 = 0;

        create_entry(Words[0], &TestEntry1);
        create_entry(Words[66], &TestEntry67);
        create_entry(Words[99], &TestEntry100);

        ErrorCode = add_entry(&List, &TestEntry1);
        TEST_CHECK(ErrorCode == ErrorCode_DuplicateEntry);
        TEST_CHECK(TestEntry1 == 0);

        ErrorCode = add_entry(&List, &TestEntry67);
        TEST_CHECK(ErrorCode == ErrorCode_DuplicateEntry);
        TEST_CHECK(TestEntry67 == 0);

        ErrorCode = add_entry(&List, &TestEntry100);
        TEST_CHECK(ErrorCode == ErrorCode_DuplicateEntry);
        TEST_CHECK(TestEntry100 == 0);
    }

    // NOTE(philip): get_first()
    {
        entry *FirstEntry = get_first(0);
        TEST_CHECK(*FirstEntry == 0);

        FirstEntry = get_first(&List);
        TEST_CHECK(*FirstEntry == Entries[99]);
    }

    // NOTE(philip): get_next()
    {
        entry *NextEntry = get_next(0, 0);
        TEST_CHECK(*NextEntry == 0);

        entry PreviousEntry = 0;

        for (entry *Entry = get_first(&List);
             *Entry;
             Entry = get_next(&List, Entry))
        {
            if (PreviousEntry)
            {
                TEST_CHECK(PreviousEntry->Next == *Entry);
            }

            PreviousEntry = *Entry;
        }
    }

    // NOTE(philip): get_number_entries()
    {
        u32 EntryCount = get_number_entries(0);
        TEST_CHECK(EntryCount == 0);

        EntryCount = get_number_entries(&List);
        TEST_CHECK(KEYWORD_COUNT);
    }
#undef KEYWORD_COUNT

    // NOTE(philip): Entry list destruction.
    {
        ErrorCode = destroy_entry_list(0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        ErrorCode = destroy_entry_list(&List);
        TEST_CHECK(ErrorCode == ErrorCode_Success);
        TEST_CHECK(List.Head == 0);
        TEST_CHECK(List.Count == 0);
    }
}

function void
Test_Index(void)
{
    error_code ErrorCode = ErrorCode_Success;

    char *Words[10] =
    {
        "halt", "oouch", "pop", "oops", "felt", "fell", "smell", "shell", "help", "hell"
    };

    entry_list List = { };
    create_entry_list(&List);

    for (u64 Index = 0;
         Index < 10;
         ++Index)
    {
        entry Entry = 0;
        create_entry(Words[Index], &Entry);
        add_entry(&List, &Entry);
    }

    index Index = { };

    // NOTE(philip): BK-Tree creation.
    {
        ErrorCode = build_entry_index(0, 0, 0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        ErrorCode = build_entry_index(&List, MatchType_Levenshtein, &Index);
        TEST_CHECK(ErrorCode == ErrorCode_Success);
        TEST_CHECK(Index.Root != 0);
    }

    // NOTE(philip): BK-Tree search.
    {
        ErrorCode = lookup_entry_index(0, 0, 0, 0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        entry_list Result = { };
        ErrorCode = lookup_entry_index("helt", &Index, 2, &Result);
        TEST_CHECK(ErrorCode == ErrorCode_Success);
        TEST_CHECK(Result.Count == 6);

        entry CurrentEntry = Result.Head;
        TEST_CHECK(CurrentEntry->Word == Words[0]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry->Word == Words[4]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry->Word == Words[5]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry->Word == Words[7]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry->Word == Words[8]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry->Word == Words[9]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry == 0);

        destroy_entry_list(&Result);
    }

    // NOTE(philip): BK-Tree search.
    {
        entry_list Result = { };
        ErrorCode = lookup_entry_index("ops", &Index, 2, &Result);
        TEST_CHECK(ErrorCode == ErrorCode_Success);
        TEST_CHECK(Result.Count == 2);

        entry CurrentEntry = Result.Head;
        TEST_CHECK(CurrentEntry->Word == Words[2]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry->Word == Words[3]);
        CurrentEntry = CurrentEntry->Next;

        TEST_CHECK(CurrentEntry == 0);

        destroy_entry_list(&Result);
    }

    // NOTE(philip): BK-Tree destruction.
    {
        ErrorCode = destroy_entry_index(0);
        TEST_CHECK(ErrorCode == ErrorCode_InvalidParameters);

        ErrorCode = destroy_entry_index(&Index);
        TEST_CHECK(ErrorCode == ErrorCode_Success);
        TEST_CHECK(Index.Root == 0);
    }

    destroy_entry_list(&List);
}

TEST_LIST =
{
    { "Exact Keyword Matching",           Test_IsExactMatch                 },
    { "Hamming Distance Calculation",     Test_CalculateHammingDistance     },
    { "Levenshtein Distance Calculation", Test_CalculateLevenshteinDistance },
    { "Entry List",                       Test_EntryList                    },
    { "Index",                            Test_Index                        },
    { 0, 0 }
};
