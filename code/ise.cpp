#include <stdio.h>

#include "ise_base.h"
#include "ise.h"

#include "ise_base.cpp"

function b32
IsExactMatch(char *A, char *B)
{
    b32 Result = true;

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                Result = false;
                break;
            }
        }
    }
    else
    {
        Result = false;
    }

    return Result;
}

// TODO(philip): What if we give this strings of different lengths??? Is that ever a case??? If so, does this
// function handle it??? For now a special value of -1 is returned.
function s32
CalculateHammingDistance(char *A, char *B)
{
    s32 Result = 0;

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                ++Result;
            }
        }
    }
    else
    {
        Result = -1;
    }

    return Result;
}

/*

  NOTE(philip): There are several algorithm implementations for calculating the Levenshtein distance between
  two strings, but here we are using the one with the matrix cache. The regular implementation uses 3 recursive
  function calls. Appart from the fact that function calls are expensive for a function called so many times,
  that specific implementation recalculates the length between two substrings multiple times, thus wasting CPU
  time. The version used here is not recursive and uses a 2D matrix to cache distance calculation results that
  might need to be used in the future.

  One small implementation detail is that this matrix is stored as a 1D array in memory. This should be more
  cache friendly. Additionally, the matrix is a global variable and it is reused on every function call.

  Implementation Reference: https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm

*/

global u32 LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * MAX_KEYWORD_LENGTH];

function u32
CalculateLevenshteinDistance(char *A, char *B)
{
    ZeroMemory(LevenshteinDistanceCache, MAX_KEYWORD_LENGTH * MAX_KEYWORD_LENGTH * sizeof(u32));

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    for (u64 Index = 1;
         Index <= LengthA;
         ++Index)
    {
        LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * 0 + Index] = Index;
    }

    for (u64 Index = 1;
         Index <= LengthB;
         ++Index)
    {
        LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * Index + 0] = Index;
    }

    for (u64 IndexB = 1;
         IndexB <= LengthB;
         ++IndexB)
    {
        for (u64 IndexA = 1;
             IndexA <= LengthA;
             ++IndexA)
        {
            u32 DeletionDistance  = LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * IndexB + (IndexA - 1)] + 1;
            u32 InsertionDistance = LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * (IndexB - 1) + IndexA] + 1;

            b32 RequiresSubstitution = (A[IndexA - 1] != B[IndexB - 1]);
            u32 SubstitutionDistance = LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * (IndexB - 1) + (IndexA - 1)] +
                                       RequiresSubstitution;

            u32 *Distance = &LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * IndexB + IndexA];
            *Distance = Min(Min(DeletionDistance, InsertionDistance), SubstitutionDistance);
        }
    }

    return LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * LengthB + LengthA];
}

s32 main()
{
    u32 Result = CalculateLevenshteinDistance("hell", "help");
    printf("%d\n", Result);

    Result = CalculateLevenshteinDistance("hell", "fall");
    printf("%d\n", Result);

    Result = CalculateLevenshteinDistance("hell", "small");
    printf("%d\n", Result);

    Result = CalculateLevenshteinDistance("help", "fell");
    printf("%d\n", Result);

    Result = CalculateLevenshteinDistance("fall", "felt");
    printf("%d\n", Result);

    Result = CalculateLevenshteinDistance("fall", "melt");
    printf("%d\n", Result);

    return 0;
}
