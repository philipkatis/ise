/*

  NOTE(philip): These functions implement the required interface from the assignment. They are just a wrapper of
  the above functions so refer to those for implementation details.

*/

function error_code
create_entry(char *Word, entry *Entry)
{
    if (Entry)
    {
        *Entry = KeywordList_AllocateKeyword(Word);

        if (*Entry)
        {
            return ErrorCode_Success;
        }
        else
        {
            return ErrorCode_FailedAllocation;
        }
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function error_code
destroy_entry(entry *Entry)
{
    if (Entry)
    {
        KeywordList_DeallocateKeyword(*Entry);
        *Entry = 0;

        return ErrorCode_Success;
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function error_code
create_entry_list(entry_list *List)
{
    if (List)
    {
        *List = KeywordList_Create();
        return ErrorCode_Success;
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function error_code
add_entry(entry_list *List, entry *Entry)
{
    if (List && Entry && *Entry)
    {
        if (!KeywordList_Find(List, (*Entry)->Word))
        {
            KeywordList_Insert(List, *Entry);
            return ErrorCode_Success;
        }
        else
        {
            KeywordList_DeallocateKeyword(*Entry);
            *Entry = 0;

            return ErrorCode_DuplicateEntry;
        }
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function entry *
get_first(entry_list *List)
{
    // NOTE(philip): That's what is needed to get this API to work. LUL.
    static entry Storage;

    Storage = 0;
    if (List)
    {
        Storage = List->Head;
    }

    return &Storage;
}

function entry *
get_next(entry_list *List, entry *Entry)
{
    static entry Storage;

    Storage = 0;
    if (List && Entry && *Entry)
    {
        Storage = (*Entry)->Next;
    }

    return &Storage;
}

function u32
get_number_entries(entry_list *List)
{
    u32 Result = 0;

    if (List)
    {
        Result = List->Count;
    }

    return Result;
}

function error_code
destroy_entry_list(entry_list *List)
{
    if (List)
    {
        KeywordList_Destroy(List);
        return ErrorCode_Success;
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function error_code
build_entry_index(entry_list *List, match_type MatchType, index *Index)
{
    if (List && Index)
    {
        *Index = BKTree_Create(MatchType);

        for (entry Entry = List->Head;
             Entry;
             Entry = Entry->Next)
        {
            BKTree_Insert(Index, Entry);
        }

        return ErrorCode_Success;
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function error_code
lookup_entry_index(char *Word, index *Index, u32 Threshold, entry_list *Result)
{
    if (Word && Index && Result)
    {
        *Result = BKTree_FindMatches(Index, Word, Threshold);
        return ErrorCode_Success;
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}

function error_code
destroy_entry_index(index *Index)
{
    if (Index)
    {
        BKTree_Destroy(Index);
        return ErrorCode_Success;
    }
    else
    {
        return ErrorCode_InvalidParameters;
    }
}
