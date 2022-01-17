#include "acutest.h"

#include "ise.h"

#include "ise_base.cpp"
#include "ise_query_tree.cpp"
#include "ise_query_list.cpp"
#include "ise_keyword_table.cpp"
#include "ise_keyword_list.cpp"
#include "ise_bk_tree.cpp"
#include "ise_answer.cpp"

#include "tests_query_tree.cpp"
#include "tests_keyword_table.cpp"
#include "tests_bk_tree.cpp"

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
Test_CalculateEditDistance(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Test some regular cases, since this function is more complex than the other keyword matching
    // functions.
    {
        A = "kitten";
        B = "sitten";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sitten";
        B = "sittin";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sittin";
        B = "sitting";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "bravo";
        B = "raven";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 3);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

TEST_LIST =
{
    { "Exact Keyword Matching",       Test_IsExactMatch             },
    { "Hamming Distance Calculation", Test_CalculateHammingDistance },
    { "Edit Distance Calculation",    Test_CalculateEditDistance    },
    { "Query Tree",                   QueryTree                     },
    { "Keyword Table",                KeywordTable                  },
    { "BK Tree",                      BKTree                        },
    { 0, 0 }
};
