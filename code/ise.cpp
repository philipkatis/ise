/*

  NOTE(philip): This function is used only for debugging. It prints the specified number of tabs to the
  command-line. It is mostly used for visualization.

*/

#if ISE_DEBUG

function void
PrintTabs(u64 Count)
{
    for (u64 Index = 0;
         Index < Count;
         ++Index)
    {
        printf("    ");
    }
}

#endif

/*

  NOTE(philip): This function creates a keyword list with zero nodes.

*/

function keyword_list
KeywordList_Create()
{
    keyword_list Result = { };
    return Result;
}

/*

  NOTE(philip): This function allocates a new keyword and sets the data it stores.

*/

function keyword *
KeywordList_AllocateKeyword(char *Word)
{
    keyword *Result = (keyword *)calloc(1, sizeof(keyword));
    Result->Word = Word;

    return Result;
}

/*

  NOTE(philip): This function deallocates a keyword.

*/

function void
KeywordList_DeallocateKeyword(keyword *Keyword)
{
    free(Keyword);
}

/*

  NOTE(philip): This function creates a new keyword and inserts it the keyword list. The new word will be at the
  head of the list. This is done to improve insertion speed. This function does not ensure that there are no
  duplicates. It is up to the client to use KeywordList_Find() to implement that functionality.

*/

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

/*

  NOTE(philip): This function inserts a keyword to the keyword list. The new word will be at the head of the list.
  This is done to improve insertion speed. This function does not ensure that there are no duplicates. It is up to
  the client to use KeywordList_Find() to implement that functionality.

*/

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

/*

  NOTE(philip): This function finds a word in the keyword list and returns it. If the word could not be found, this
  function returns zero.

*/

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

/*

  NOTE(philip): This function is used only for debugging. It is a tool for visualizing the contents of a keyword
  list, by dumping them into the command-line.

*/

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

#define KeywordList_Visualize(List) _KeywordList_Visualize((List))

#else

#define KeywordList_Visualize(List)

#endif

/*

  NOTE(philip): This function destroys a keyword list, deallocating all the memory allocated by all it's nodes.
  This does not deallocate the memory of the words referenced by all the nodes.

*/

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

/*

  NOTE(philip): This function creates a BK-Tree with zero nodes.

*/

function bk_tree
BKTree_Create(match_type MatchType)
{
    bk_tree Result = { };
    Result.MatchType = MatchType;

    return Result;
}

/*

  NOTE(philip): This function allocates the memory and sets the data for a new BK-Tree node.

*/

function bk_tree_node *
BKTree_AllocateNode(keyword *Keyword, u64 DistanceFromParent)
{
    bk_tree_node *Result = (bk_tree_node *)calloc(1, sizeof(bk_tree_node));
    Result->Keyword = Keyword;
    Result->DistanceFromParent = DistanceFromParent;

    return Result;
}

/*

  NOTE(philip): This functions inserts a new keyword into the BK-Tree at the correct location using the Levenshtein
  distance calculation algorithm. In case the keyword we try to insert is already in the BK-Tree, this function will
  return a reference to it.

*/

function bk_tree_node *
BKTree_Insert(bk_tree *Tree, keyword *Keyword)
{
    bk_tree_node *Node = 0;

    if (Tree->Root)
    {
        bk_tree_node *CurrentNode = Tree->Root;
        while (true)
        {
            u64 DistanceFromCurrentNode = CalculateLevenshteinDistance(CurrentNode->Keyword->Word,
                                                                       strlen(CurrentNode->Keyword->Word),
                                                                       Keyword->Word, strlen(Keyword->Word));
            b32 FoundNodeWithSameDistance = false;

            for (bk_tree_node *ChildNode = CurrentNode->FirstChild;
                 ChildNode;
                 ChildNode = ChildNode->NextSibling)
            {
                if (ChildNode->DistanceFromParent == DistanceFromCurrentNode)
                {
                    CurrentNode = ChildNode;
                    FoundNodeWithSameDistance = true;

                    break;
                }
            }

            if (!FoundNodeWithSameDistance)
            {
                Node = BKTree_AllocateNode(Keyword, DistanceFromCurrentNode);
                Node->NextSibling = CurrentNode->FirstChild;

                CurrentNode->FirstChild = Node;

                break;
            }
        }
    }
    else
    {
        Node = BKTree_AllocateNode(Keyword, 0);
        Tree->Root = Node;
    }

    return Node;
}

/*

  NOTE(philip): This function searches through the BK-Tree in an optimized manner, in order to find the keywords
  that match the specified word with the maximum threshold. It returns a keyword list containing the matches.

*/

function keyword_list
BKTree_FindMatches(bk_tree *Tree, char *Word, u64 DistanceThreshold)
{
    keyword_list Matches = KeywordList_Create();

    // TODO(philip): Implement this using a stack.
    bk_tree_node *Candidates[1024];
    u64 CandidateCount = 0;

    // TODO(philip): Change this to a push operation.
    Candidates[CandidateCount] = Tree->Root;
    ++CandidateCount;

    while (CandidateCount)
    {
        // TODO(philip): Change this to a pop operation.
        bk_tree_node *CurrentCandidate = Candidates[CandidateCount - 1];
        --CandidateCount;

        u64 DistanceFromCurrentCandidate = CalculateLevenshteinDistance(CurrentCandidate->Keyword->Word,
                                                                        strlen(CurrentCandidate->Keyword->Word),
                                                                        Word, strlen(Word));

        if (DistanceFromCurrentCandidate <= DistanceThreshold)
        {
            KeywordList_Insert(&Matches, CurrentCandidate->Keyword->Word);
        }

        s32 ChildrenRangeStart = DistanceFromCurrentCandidate - DistanceThreshold;
        if (ChildrenRangeStart)
        {
            ChildrenRangeStart = 1;
        }

        s32 ChildrenRangeEnd = DistanceFromCurrentCandidate + DistanceThreshold;

        for (bk_tree_node *Child = CurrentCandidate->FirstChild;
             Child;
             Child = Child->NextSibling)
        {
            if (Child->DistanceFromParent >= ChildrenRangeStart && Child->DistanceFromParent <= ChildrenRangeEnd)
            {
                Candidates[CandidateCount] = Child;
                ++CandidateCount;
            }
        }
    }

    return Matches;
}

/*

  NOTE(philip): This function is used only for debugging. It is a tool for visualizing the contents of a BK-Tree
  node, by dumping them into the command-line. This is a recursive function that also prints all of the node's
  siblings and children.

*/

#if ISE_DEBUG

function void
BKTree_VisualizeNode(bk_tree_node *Node, u64 Depth)
{
    PrintTabs(Depth);
    printf("Word: %s, DistanceFromParent: %llu, Depth: %llu\n", Node->Keyword->Word, Node->DistanceFromParent,
           Depth);

    if (Node->FirstChild)
    {
        PrintTabs(Depth);
        printf("{\n");

        BKTree_VisualizeNode(Node->FirstChild, Depth + 1);

        PrintTabs(Depth);
        printf("}\n");
    }

    if (Node->NextSibling)
    {
        printf("\n");
        BKTree_VisualizeNode(Node->NextSibling, Depth);
    }
}

#endif


/*

  NOTE(philip): This function is used only for debugging. It is a tool for visualizing the contents of a BK-Tree,
  by dumping them into the command-line.

*/

#if ISE_DEBUG

function void
_BKTree_Visualize(bk_tree *Tree)
{
    if (Tree->Root)
    {
        BKTree_VisualizeNode(Tree->Root, 0);
    }
}

#define BKTree_Visualize(Tree) _BKTree_Visualize((Tree))

#else

#define BKTree_Visualize(Tree)

#endif


/*

  NOTE(philip): This function destroys a BK-Tree node and all of it's siblings and children. This is a recursive
  function that also destroys all of the node's siblings and children.. This does not deallocate the memory of the
  keywords referenced by all the nodes.

*/

function void
BKTree_DestroyNode(bk_tree_node *Node)
{
    if (Node->FirstChild)
    {
        BKTree_DestroyNode(Node->FirstChild);
    }

    if (Node->NextSibling)
    {
        BKTree_DestroyNode(Node->NextSibling);
    }

    free(Node);
}

/*

  NOTE(philip): This function destroys a BK-Tree, deallocating all the memory allocated by all it's nodes.
  This does not deallocate the memory of the keywords referenced by all the nodes.

*/

function void
BKTree_Destroy(bk_tree *Tree)
{
    if (Tree->Root)
    {
        BKTree_DestroyNode(Tree->Root);
    }

    Tree->Root = 0;
}

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

#if 0

function entry *
Entry_Allocate(char* Word)
{
    entry* Result = (entry*)calloc(1, sizeof(entry));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

    return Result;
}

function void
Entry_Destroy(entry *Entry)
{
    free(Entry);
}

function entry *
Entry_Create(char *Word)
{
    entry *Entry = Entry_Allocate(Word);

    /*

    NOTE(Alex): If the said payload is actually implemented, we need to search for queries in which the word
    exists and insert their ID numbers in the matrix.

    */

    return Entry;
}

function entry_list_node*
EntryList_AllocateNode(entry* Entry)
{
    entry_list_node* Result = (entry_list_node*)calloc(1, sizeof(entry_list_node));
    Result->Entry = Entry;
    Result->Previous = NULL;
    Result->Next = NULL;

    return Result;
}

function void
EntryList_DeallocateNode(entry_list_node* Node)
{
    free(Node);
}

function entry_list
EntryList_Create()
{
    entry_list Result = { };
    return Result;
}

function void
EntryList_AddEntry(entry_list* List, entry_list_node* Node)
{
    if (List->Head == 0)
    {
        List->Head = Node;
        List->Tail = Node;
    }
    else
    {
        entry_list_node* Next = List->Head;
        Next->Previous = Node;

        Node->Next = Next;
        List->Head = Node;
    }

    ++List->Count;
}

function void
EntryList_Visualize(entry_list* List)
{
    entry_list_node* Node = List->Head;
    u64 Index = 0;

    while (Node)
    {
        printf("Index: %llu, Word: %s\n", Index, Node->Entry->Word);
        Node = Node->Next;
        Index++;
    }
}

function entry_list_node*
EntryList_GetFirst(entry_list* List)
{
    return List->Head;
}

function entry_list_node*
EntryList_GetNext(entry_list_node* Node)
{
    return Node->Next;
}

function u32
EntryList_GetNumberEntries(entry_list* List)
{
    return List->Count;
}

function void
EntryList_Destroy(entry_list* List)
{
    entry_list_node* Node = List->Head;
    while (Node)
    {
        entry_list_node* Next = Node->Next;
        EntryList_DeallocateNode(Node);
        Node = Next;
    }

    List->Head = 0;
    List->Tail = 0;
    List->Count = 0;
}

s32 main()
{
    keyword_list List = KeywordList_Create();

    KeywordList_Insert(&List, "halt");
    KeywordList_Insert(&List, "oouch");
    KeywordList_Insert(&List, "pop");
    KeywordList_Insert(&List, "oops");
    KeywordList_Insert(&List, "felt");
    KeywordList_Insert(&List, "fell");
    KeywordList_Insert(&List, "smell");
    KeywordList_Insert(&List, "shell");
    KeywordList_Insert(&List, "help");
    KeywordList_Insert(&List, "hell");

    bk_tree Tree = BKTree_Create();

    for (keyword *Keyword = List.Head;
         Keyword;
         Keyword = Keyword->Next)
    {
        BKTree_Insert(&Tree, Keyword);
    }

    keyword_list Oops = BKTree_FindMatches(&Tree, "helt", 2);
    KeywordList_Visualize(&Oops);

    BKTree_Destroy(&Tree);
    KeywordList_Destroy(&List);

    return 0;
}

#endif
