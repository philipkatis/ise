#include <stdlib.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_match.cpp"

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

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateLevenshteinDistance(A, StringLength(A), B, StringLength(B));
        TEST_CHECK(Result == 1);
    }
}

TEST_LIST =
{
    { "IsExactMatch",                 Test_IsExactMatch                 },
    { "CalculateHammingDistance",     Test_CalculateHammingDistance     },
    { "CalculateLevenshteinDistance", Test_CalculateLevenshteinDistance },
    { 0, 0 }
};
