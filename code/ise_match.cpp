/*

  NOTE(philip): This is the first of the keyword matching functions. It returns 0 if the two words do not match and
  it only returns 1 when the two words match exactly.

*/

function u32
IsExactMatch(char *A, u64 LengthA, char *B, u64 LengthB)
{
    u32 Result = 1;

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                Result = 0;
                break;
            }
        }
    }
    else
    {
        Result = 0;
    }

    return Result;
}

/*

  NOTE(philip): This is the second keyword matching function. It returns the Hamming distance between the two
  keywords. This means that this function only operates on keywords of the same length. The function itself does
  not check for that condition, so the client using it must be aware. If and only if the words are exactly the
  same, will the function return 0.

*/

function u32
CalculateHammingDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    s32 Result = 0;
    Assert(LengthA == LengthB);

    for (u64 Index = 0;
         Index < LengthA;
         ++Index)
    {
        if (A[Index] != B[Index])
        {
            ++Result;
        }
    }

    return Result;
}

/*

  NOTE(philip): This is the third and last keyword matching function. It returns the Levenshtein distance between
  two words. It only returns 0 if the words are exactly the same. There are several algorithm implementations for
  calculating the Levenshtein distance between two strings, but here we are using the one with the matrix cache.
  The regular implementation uses 3 recursive function calls. Appart from the fact that function calls are
  expensive for a function called so many times, that specific implementation recalculates the length between two
  substrings multiple times, thus wasting CPU time. The version used here is not recursive and uses a 2D matrix to
  cache distance calculation results that might need to be used in the future.

  One small implementation detail is that this matrix is stored as a 1D array in memory. This should be more
  cache friendly. Additionally, the matrix is a global variable and it is reused on every function call.

  Implementation Reference: https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm

*/

#define LEVENSHTEIN_CACHE_MATRIX_SIZE (MAX_KEYWORD_LENGTH + 1)

global u32 LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * LEVENSHTEIN_CACHE_MATRIX_SIZE];

function u32
CalculateLevenshteinDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    ZeroMemory(LevenshteinDistanceCache, LEVENSHTEIN_CACHE_MATRIX_SIZE * LEVENSHTEIN_CACHE_MATRIX_SIZE * sizeof(u32));

    for (u64 Index = 1;
         Index <= LengthA;
         ++Index)
    {
        LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * 0 + Index] = Index;
    }

    for (u64 Index = 1;
         Index <= LengthB;
         ++Index)
    {
        LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * Index + 0] = Index;
    }

    for (u64 IndexB = 1;
         IndexB <= LengthB;
         ++IndexB)
    {
        for (u64 IndexA = 1;
             IndexA <= LengthA;
             ++IndexA)
        {
            u32 DeletionDistance  = LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * IndexB + (IndexA - 1)] + 1;
            u32 InsertionDistance = LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * (IndexB - 1) + IndexA] + 1;

            b32 RequiresSubstitution = (A[IndexA - 1] != B[IndexB - 1]);
            u32 SubstitutionDistance = LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * (IndexB - 1) + (IndexA - 1)] +
                RequiresSubstitution;

            u32 *Distance = &LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * IndexB + IndexA];
            *Distance = Min(Min(DeletionDistance, InsertionDistance), SubstitutionDistance);
        }
    }

    return LevenshteinDistanceCache[LEVENSHTEIN_CACHE_MATRIX_SIZE * LengthB + LengthA];
}

// NOTE(philip): This array stores all the different keyword matching functions in such a way that the match_type
// can be used to index into it.
global match_function MatchFunctions[3] = { IsExactMatch, CalculateHammingDistance, CalculateLevenshteinDistance };
