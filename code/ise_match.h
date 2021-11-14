#ifndef ISE_MATCH_H
#define ISE_MATCH_H

/*

  NOTE(philip): This is the first of the keyword matching functions. It returns 0 if the two words do not match and
  it only returns 1 when the two words match exactly.

*/

function u32 IsExactMatch(char *A, u64 LengthA, char *B, u64 LengthB);

/*

  NOTE(philip): This is the second keyword matching function. It returns the Hamming distance between the two
  keywords. This means that this function only operates on keywords of the same length. The function itself does
  not check for that condition, so the client using it must be aware. If and only if the words are exactly the
  same, will the function return 0.

*/

function u32 CalculateHammingDistance(char *A, u64 LengthA, char *B, u64 LengthB);

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

function u32 CalculateLevenshteinDistance(char *A, u64 LengthA, char *B, u64 LengthB);

#endif
