#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

        Result = IsExactMatch(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = IsExactMatch(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Different words. Same length.
    {
        A = "hello";
        B = "world";

        Result = IsExactMatch(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words. Different length.
    {
        A = "000x00010f001";
        B = "rgjahrg";

        Result = IsExactMatch(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Max string length.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = IsExactMatch(A, StringLength(A), B, StringLength(B));
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

        Result = CalculateHammingDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateHammingDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateHammingDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateHammingDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateHammingDistance(A, StringLength(A), B, StringLength(B));
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

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Test some regular cases, since this function is more complex than the other keyword matching
    // functions.
    {
        A = "kitten";
        B = "sitten";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 1);

        A = "sitten";
        B = "sittin";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 1);

        A = "sittin";
        B = "sitting";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 1);

        A = "bravo";
        B = "raven";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 3);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
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

TEST_LIST =
{
    { "Exact Keyword Matching",           Test_IsExactMatch                 },
    { "Hamming Distance Calculation",     Test_CalculateHammingDistance     },
    { "Levenshtein Distance Calculation", Test_CalculateLevenshteinDistance },
    { "Entry List",                       Test_EntryList                    },
    { 0, 0 }
};
