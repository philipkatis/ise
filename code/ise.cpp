#include <stdlib.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise.h"

#include "ise_base.cpp"

/*

  NOTE(philip): The queries will come from a file. Each query will be on a separate line, with the line number
  acting as the query number/ID. Each query will have up to 5 keywords (based on files from the competition),
  separated by a space. The keywords must follow the regular keyword restrictions.

  NOTE(philip): Deduplication should happen to both the query keywords and the document contents. The most ideal
  way to achieve this has to be a hash table, where we store all the words and only compare them when their hash
  values are the same.

  NOTE(philip): The entry list seems to simply be a list of keywords where each node contains the keyword but also
  additional information. (For example the query ID that that particular keyword is part of.) Based on the
  assignment, this list has to be used in two cases. 1) To supply the query keywords to the BK-tree for
  construction, and 2) as return value for the BK-tree lookup function.

  TODO(philip): What is the final resting point of the query keywords? Are they stored in a keyword list and then
  referenced to by a pointer? Are they stored in the BK-tree for faster accesss?

  TODO(philip): Can the keyword list not be a list??? It makes more sence for it to be a hash table for O(1)
  access, since that will be the most common case. A hash table can also accelerate the deduplication.

  TODO(philip): Investigate how we choose what keyword matching function is used each time.

  TODO(philip): Replace calloc() and free() with custom function that can be instrumented later on.

  NOTE(philip): There are many ways to optimize the keyword matching functions. If there's a need to in the
  future, we should investigate whether the use of intrinsics (SSE) is allowed.

*/

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

function keyword_list_node *
KeywordList_AllocateNode(char *Word)
{
    keyword_list_node *Result = (keyword_list_node *)calloc(1, sizeof(keyword_list_node));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

    return Result;
}

function void
KeywordList_DeallocateNode(keyword_list_node *Node)
{
    free(Node);
}

function keyword_list
KeywordList_Create()
{
    keyword_list Result = { };
    return Result;
}

function void
KeywordList_Insert(keyword_list *List, keyword_list_node *Node)
{
    if (List->Head == 0)
    {
        List->Head = Node;
        List->Tail = Node;
    }
    else
    {
        keyword_list_node *Next = List->Head;
        Next->Previous = Node;

        Node->Next = Next;
        List->Head = Node;
    }
}

function void
KeywordList_Destroy(keyword_list *List)
{
    keyword_list_node *Node = List->Head;
    while (Node)
    {
        keyword_list_node *Next = Node->Next;
        KeywordList_DeallocateNode(Node);
        Node = Next;
    }

    List->Head = 0;
    List->Tail = 0;
    List->Count = 0;
}

function void
KeywordList_Visualize(keyword_list *List)
{
    keyword_list_node *Node = List->Head;
    u64 Index = 0;

    while (Node)
    {
        printf("Index: %llu, Word: %s\n", Index, Node->Word);
        Node = Node->Next;
        Index++;
    }
}

function bk_tree_node *
BKTree_AllocateNode(keyword_list_node *Keyword)
{
    bk_tree_node *Result = (bk_tree_node *)calloc(1, sizeof(bk_tree_node));
    Result->Keyword = Keyword;

    return Result;
}

function bk_tree_node *
BKTree_Create(keyword_list_node *RootKeyword)
{
    return BKTree_AllocateNode(RootKeyword);
}

function bk_tree_node *
BKTree_Insert(bk_tree_node *Tree, keyword_list_node *Keyword)
{
    bk_tree_node *Result = 0;

    bk_tree_node *Node = Tree;
    b32 Complete = false;

    while (!Complete)
    {
        u32 Distance = CalculateLevenshteinDistance(Node->Keyword->Word, Keyword->Word);
        if (Distance > 0)
        {
            if (Node->Children[Distance - 1])
            {
                Node = Node->Children[Distance - 1];
            }
            else
            {
                Result = BKTree_AllocateNode(Keyword);
                Node->Children[Distance - 1] = Result;

                Complete = true;
            }
        }
        else
        {
            // NOTE(philip): If the keyword is already in the BK-tree, do not insert it.
            Complete = true;
        }
    }

    return Result;
}

function void
BKTree_Destroy(bk_tree_node *Node)
{
    for (u64 Index = 0;
         Index < MAX_KEYWORD_LENGTH - 1;
         ++Index)
    {
        if (Node->Children[Index])
        {
            BKTree_Destroy(Node->Children[Index]);
        }
    }

    free(Node);
}

function void
PrintSpaces(u64 Count)
{
    for (u64 Index = 0;
         Index < Count;
         ++Index)
    {
        printf(" ");
    }
}

function void
BKTree_Visualize(bk_tree_node *Node, u64 Depth = 0)
{
    PrintSpaces(Depth * 4);
    printf("{\n");

    PrintSpaces(Depth * 4 + 4);
    printf("Word: %s\n", Node->Keyword->Word);

    for (u64 Index = 0;
         Index < MAX_KEYWORD_LENGTH - 1;
         ++Index)
    {
        if (Node->Children[Index])
        {
            printf("\n");
            PrintSpaces(Depth * 4 + 4);
            printf("Distance: %llu\n", Index + 1);

            BKTree_Visualize(Node->Children[Index], Depth + 1);
        }
    }

    PrintSpaces(Depth * 4);
    printf("}\n");
}

function b32
LoadTextFile(char *Path, buffer *Buffer)
{
    b32 Result = true;

    FILE *File = fopen(Path, "rt");
    if (File)
    {
        fseek(File, 0, SEEK_END);
        u64 Length = ftell(File);

        *Buffer = AllocateBuffer(Length * sizeof(char));

        fseek(File, 0, SEEK_SET);
        fread(Buffer->Data, 1, Length, File);

        fclose(File);
    }
    else
    {
        Result = false;
    }

    return Result;
}

function entry *
Allocate_entry(char* Word)
{
    entry* Result = (entry*)calloc(1, sizeof(entry));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

    return Result;
}

function void
Deallocate_entry(entry *Entry)
{
    free(Entry);
}

function entry *
Create_entry(char *Word)
{
    entry *Entry = Allocate_entry(Word);

    /*

    NOTE(Alex): Initialize payload here too.

    */

    return Entry;
}

/*

  NOTE(philip): Since all the data that the application needs to process come from a file, we read the entire
  file contents and store them in a contiguous buffer. That buffer is then directly modified to separate the
  different data from one another, and then pointers into the buffer are used to reference each of them.

  TODO(philip): We should investigate the performance characteristics of this approach. While this requires the
  least number of string copies (which is an expensive operation), there are two other problems that I can think
  of right now.

  1) Cache misses. Since the actual data is separated from the BK-tree (the structure that we will need to
  traverse the most), the keyword strings will be loaded/unloaded to/from the cache all the time.

  2) Data duplication. We can certainly use separate data-structures to cull duplicate keywords from being
  processed, but the data will always be loaded in memory.

  NOTE(philip): Right now we assume that there will stricly be a single command per line.

*/

function b32
EndOfCommand(parse_context *Context)
{
    b32 Result = (!(*Context->Pointer) || (*Context->Pointer == '\n'));
    return Result;
}

// NOTE(philip): This function is what modifies the file data, essentially replacing whitespace with 0, except
// new line characters. That replacement occurs at ParseCommandFile().
function void
NullifyWhitespace(parse_context *Context)
{
    while (!EndOfCommand(Context) && IsWhitespace(*Context->Pointer))
    {
        *Context->Pointer = 0;
        ++Context->Pointer;
    }
}

function void
ParseQueryEntry(parse_context *Context)
{
#define MAX_TOKEN_COUNT (4 + MAX_KEYWORD_COUNT_PER_QUERY)

    char *Tokens[MAX_TOKEN_COUNT];
    u32 TokenCount = 0;

    // NOTE(philip): Get rid of any whitespace before the first token.
    NullifyWhitespace(Context);

    if (!EndOfCommand(Context))
    {
        while (!EndOfCommand(Context) && (TokenCount <= MAX_TOKEN_COUNT))
        {
            ++TokenCount;
            Tokens[TokenCount - 1] = Context->Pointer;

            ++Context->Pointer;

            // NOTE(philip): Go over all the token characters.
            while (!EndOfCommand(Context) && !IsWhitespace(*Context->Pointer))
            {
                ++Context->Pointer;
            }

            // NOTE(philip): Get rid of any whitespace before the next token.
            NullifyWhitespace(Context);
        }

        // NOTE(philip): If there are more tokens that the maximum allowable number, skip them.
        if (TokenCount > MAX_TOKEN_COUNT)
        {
            while (!EndOfCommand(Context))
            {
                ++Context->Pointer;
            }
        }
    }

    // NOTE(philip): After parsing is complete, the last character should always be a new line character.
    Assert(*Context->Pointer == '\n');
    *Context->Pointer = 0;

    // NOTE(philip): Ensure that the data we read is all valid.

    if (TokenCount < 4 || TokenCount > MAX_TOKEN_COUNT)
    {
        printf("[Warning]: Incorrect argument count for query insertion! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[0]))
    {
        printf("[Warning]: Found query with non-integer ID! Skipping... (Line: %llu)\n", Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[1]))
    {
        printf("[Warning]: Found query with non-integer match type! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[2]))
    {
        printf("[Warning]: Found query with non-integer match distance! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[3]))
    {
        printf("[Warning]: Found query with non-integer keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    u32 ID = atoi(Tokens[0]);
    u32 MatchType = atoi(Tokens[1]);
    u32 MatchDistance = atoi(Tokens[2]);
    u32 KeywordCount = atoi(Tokens[3]);

    // TODO(philip): Check for duplicate IDs.

    if (MatchType > 2)
    {
        printf("[Warning]: Found query with out-of-bounds match type! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    u32 MinMatchDistance = ((MatchType == 0) ? 0 : 1);
    u32 MaxMatchDistance = ((MatchType == 0) ? 0 : MAX_KEYWORD_LENGTH);

    if (MatchDistance < MinMatchDistance || MatchDistance > MaxMatchDistance)
    {
        printf("[Warning]: Found query with out-of-bounds match distance! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (KeywordCount <= 0 || KeywordCount > MAX_KEYWORD_COUNT_PER_QUERY)
    {
        printf("[Warning]: Found query with out-of-bounds keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (KeywordCount != TokenCount - 4)
    {
        printf("[Warning]: Found query with different keyword count and real keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    printf("Query %d:\n{\n", ID);

    for (u32 Index = 4;
         Index < TokenCount;
         ++Index)
    {
        printf("    %s\n", Tokens[Index]);
    }

    printf("}\n\n");

#undef MAX_TOKEN_COUNT
}

function void
ParseCommandFile(buffer Contents)
{
    parse_context Context = { };
    Context.Pointer = (char *)Contents.Data;
    Context.LineNumber = 1;

    while (*Context.Pointer)
    {
        if (*Context.Pointer == 's')
        {
            // NOTE(philip): This command adds a new query to the active set. The format of this command is known.
            // s <Query ID> <> <> <Keyword Count> keyword1 ... ... ... ...

            // NOTE(philip): This is the command signature.
            *Context.Pointer = 0;
            ++Context.Pointer;

            ParseQueryEntry(&Context);
        }
        else
        {
            printf("[Warning]: Skipping unknown command! (Command: '%c', Line: %llu)\n", *Context.Pointer,
                   Context.LineNumber);
        }

        // NOTE(philip): This should always be zero (previously the new line character). Advance to
        // the next command.
        ++Context.Pointer;

        ++Context.LineNumber;
    }
}

s32 main()
{
    char *CommandFilePath = "data/commands.txt";

    buffer CommandFileContents = { };
    if (LoadTextFile(CommandFilePath, &CommandFileContents))
    {
        ParseCommandFile(CommandFileContents);
        DeallocateBuffer(CommandFileContents);
    }
    else
    {
        printf("[Error]: Could not open command file! (Path: \"%s\")\n", CommandFilePath);
    }

#if 0
    char *Words[] = { "hell", "help", "fall", "felt", "fell", "small", "melt" };

    keyword_list List = KeywordList_Create();
    for (u64 Index = 0;
         Index < ArrayCount(Words);
         ++Index)
    {
        keyword_list_node *Node = KeywordList_AllocateNode(Words[Index]);
        KeywordList_Insert(&List, Node);
    }

    KeywordList_Visualize(&List);

    printf("\n\n");

    bk_tree_node *Tree = BKTree_Create(List.Tail);

    for (keyword_list_node *Node = List.Tail->Previous;
         Node;
         Node = Node->Previous)
    {
        BKTree_Insert(Tree, Node);
    }

    BKTree_Visualize(Tree);

    BKTree_Destroy(Tree);
    KeywordList_Destroy(&List);
#endif

    return 0;
}
