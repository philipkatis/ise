#include "ise_keyword_list.h"

#include <stdlib.h>
#include <string.h>

keyword_list
KeywordList_Create()
{
    keyword_list Result = { };
    return Result;
}

keyword *
KeywordList_AllocateKeyword(char *Word)
{
    keyword *Result = (keyword *)calloc(1, sizeof(keyword));
    Result->Word = Word;

    return Result;
}

void
KeywordList_DeallocateKeyword(keyword *Keyword)
{
    free(Keyword);
}

keyword *
KeywordList_Insert(keyword_list *List, char *Word)
{
    keyword *Keyword = KeywordList_AllocateKeyword(Word);

    if (List->Head)
    {
        Keyword->Next = List->Head;
    }

    List->Head = Keyword;
    ++List->Count;

    return Keyword;
}

keyword *
KeywordList_Insert(keyword_list *List, keyword *Keyword)
{
    if (List->Head)
    {
        Keyword->Next = List->Head;
    }

    List->Head = Keyword;
    ++List->Count;

    return Keyword;
}

keyword *
KeywordList_Find(keyword_list *List, char *Word)
{
    keyword *Result = 0;

    for (keyword *Keyword = List->Head;
         Keyword;
         Keyword = Keyword->Next)
    {
        if (strcmp(Keyword->Word, Word) == 0)
        {
            Result = Keyword;
            break;
        }
    }

    return Result;
}

void
KeywordList_Destroy(keyword_list *List)
{
    keyword *Keyword = List->Head;
    while (Keyword)
    {
        keyword *NextKeyword = Keyword->Next;
        KeywordList_DeallocateKeyword(Keyword);
        Keyword = NextKeyword;
    }

    List->Head = 0;
    List->Count = 0;
}


#if ISE_DEBUG
    void
    _KeywordList_Visualize(keyword_list *List)
    {
        u64 Index = 0;

        for (keyword *Keyword = List->Head;
             Keyword;
             Keyword = Keyword->Next)
        {
            printf("%llu: %s\n", Index, Keyword->Word);
            ++Index;
        }
    }
#endif
