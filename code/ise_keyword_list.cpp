function keyword_list
KeywordList_Create()
{
    keyword_list Result = { };
    return Result;
}

function keyword *
KeywordList_AllocateKeyword(char *Word)
{
    keyword *Result = (keyword *)calloc(1, sizeof(keyword));
    Result->Word = Word;

    return Result;
}

function void
KeywordList_DeallocateKeyword(keyword *Keyword)
{
    free(Keyword);
}

function keyword *
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

function keyword *
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

function keyword *
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

function void
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
    function void
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
