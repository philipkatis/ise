//
// NOTE(philip): Keyword Hash Table
//

function void
InitializeKeywordTable(keyword_table *Table, u64 ExpectedKeywordCount)
{
    Table->SlotCount = ((4 * ExpectedKeywordCount) / 3) + 1;
    Table->KeywordCount = 0;
    Table->Slots = (keyword_table_node **)calloc(1, Table->SlotCount * sizeof(keyword_table_node *));
}

function keyword_table_node *
AllocateNode(char *Word, u64 Length, u32 Hash)
{
    keyword_table_node *Node = (keyword_table_node *)calloc(1, sizeof(keyword_table_node));

    memcpy(Node->Data.Word, Word, Length * sizeof(char));
    Node->Data.Length = Length;
    Node->Data.Hash = Hash;

    return Node;
}

function void
ResizeIfNeeded(keyword_table *Table)
{
    f32 LoadFactor = ((f32)(Table->KeywordCount + 1) / (f32)Table->SlotCount);
    if (LoadFactor > 0.75f)
    {
        u64 NewSlotCount = Table->SlotCount * 2;
        keyword_table_node **NewSlots = (keyword_table_node **)calloc(1, NewSlotCount * sizeof(keyword_table_node *));

        for (u64 Index = 0;
             Index < Table->SlotCount;
             ++Index)
        {
            keyword_table_node *Node = Table->Slots[Index];
            while (Node)
            {
                keyword_table_node *Next = Node->Next;
                u64 NewNodeSlotIndex = Node->Data.Hash % NewSlotCount;

                Node->Next = NewSlots[NewNodeSlotIndex];
                NewSlots[NewNodeSlotIndex] = Node;

                Node = Next;
            }
        }

        free(Table->Slots);

        Table->Slots = NewSlots;
        Table->SlotCount = NewSlotCount;
    }
}

function keyword *
InsertIntoKeywordTable(keyword_table *Table, char *Word)
{
    keyword *Keyword = 0;

    ResizeIfNeeded(Table);

    u64 Length = strlen(Word);
    u32 Hash = StringHash(Word);
    u64 SlotIndex = Hash % Table->SlotCount;

    if (Table->Slots[SlotIndex])
    {
        for (keyword_table_node *Node = Table->Slots[SlotIndex];
             Node;
             Node = Node->Next)
        {
            if (StringCompare(Node->Data.Word, Node->Data.Length, Word, Length))
            {
                Keyword = &Node->Data;
                break;
            }
        }

        if (!Keyword)
        {
            keyword_table_node *Node = AllocateNode(Word, Length, Hash);

            Node->Next = Table->Slots[SlotIndex];
            Table->Slots[SlotIndex] = Node;
            ++Table->KeywordCount;

            Keyword = &Node->Data;
        }
    }
    else
    {
        keyword_table_node *Node = AllocateNode(Word, Length, Hash);

        Table->Slots[SlotIndex] = Node;
        ++Table->KeywordCount;

        Keyword = &Node->Data;
    }

    return Keyword;
}

function keyword *
FindKeywordInTable(keyword_table *Table, keyword *Keyword)
{
    keyword *Result = 0;

    u64 SlotIndex = Keyword->Hash % Table->SlotCount;
    for (keyword_table_node *Node = Table->Slots[SlotIndex];
         Node;
         Node = Node->Next)
    {
        if (StringCompare(Node->Data.Word, Node->Data.Length, Keyword->Word, Keyword->Length))
        {
            Result = &Node->Data;
            break;
        }
    }

    return Result;
}

function void
DestroyKeywordTable(keyword_table *Table)
{
    for (u64 Index = 0;
         Index < Table->SlotCount;
         ++Index)
    {
        keyword_table_node *Node = Table->Slots[Index];
        while (Node)
        {
            keyword_table_node *Next = Node->Next;

            DestroyQueryList(&Node->Data.Queries);
            free(Node);
            Node = Next;
        }
    }

    free(Table->Slots);

    Table->Slots = 0;
    Table->SlotCount = 0;
    Table->KeywordCount = 0;
}

//
// NOTE(philip): Keyword Hash Table Iterator
//

function void
NextKeyword(keyword_table_iterator *Iterator)
{
    if (Iterator->Node)
    {
        Iterator->Node = Iterator->Node->Next;
    }
    else
    {
        keyword_table *Table = Iterator->Table;
        while (Iterator->SlotIndex < Table->SlotCount)
        {
            Iterator->Node = Table->Slots[Iterator->SlotIndex];
            if (Iterator->Node)
            {
                break;
            }

            ++Iterator->SlotIndex;
        }
    }
}

function keyword_table_iterator
IterateKeywordTable(keyword_table *Table)
{
    keyword_table_iterator Iterator = { };
    Iterator.Table = Table;

    NextKeyword(&Iterator);

    return Iterator;
}

function b32
IsValid(keyword_table_iterator *Iterator)
{
    b32 Valid = (Iterator->Node != 0);
    return Valid;
}

function void
Advance(keyword_table_iterator *Iterator)
{
    if (!Iterator->Node->Next)
    {
        ++Iterator->SlotIndex;
        Iterator->Node = 0;
    }

    NextKeyword(Iterator);
}

function keyword *
GetValue(keyword_table_iterator *Iterator)
{
    return &Iterator->Node->Data;
}

//
// NOTE(philip): Keyword BK-Tree Node Stack
//

#define NODE_STACK_STORAGE_SIZE 1024

function void
InitializeKeywordTreeNodeStack(keyword_tree_node_stack *Stack)
{
    Stack->Capacity = NODE_STACK_STORAGE_SIZE;
    Stack->Count = 0;
    Stack->Data = (keyword_tree_node **)malloc(Stack->Capacity * sizeof(keyword_tree_node *));
}

function void
PushIntoKeywordTreeNodeStack(keyword_tree_node_stack *Stack, keyword_tree_node *Candidate)
{
    if ((Stack->Count + 1) > Stack->Capacity)
    {
        Stack->Capacity *= 2;
        Stack->Data = (keyword_tree_node **)realloc(Stack->Data, Stack->Capacity * sizeof(keyword_tree_node *));
    }

    Stack->Data[Stack->Count++] = Candidate;
}

function keyword_tree_node *
PopFromKeywordTreeNodeStack(keyword_tree_node_stack *Stack)
{
    keyword_tree_node *Candidate = 0;

    if (Stack->Count)
    {
        Candidate = Stack->Data[--Stack->Count];
    }

    return Candidate;
}

function void
ResetKeywordTreeNodeStack(keyword_tree_node_stack *Stack)
{
    Stack->Count = 0;
}

function void
DestroyKeywordTreeNodeStack(keyword_tree_node_stack *Stack)
{
    free(Stack->Data);

    Stack->Capacity = 0;
    Stack->Count = 0;
    Stack->Data = 0;
}

//
// NOTE(philip): Keyword BK-Tree Match Stack
//

#define MATCH_STACK_STORAGE_SIZE 4096

function void
InitializeKeywordTreeMatchStack(keyword_tree_match_stack *Stack)
{
    Stack->Capacity = MATCH_STACK_STORAGE_SIZE;
    Stack->Count = 0;
    Stack->Data = (keyword_tree_match *)malloc(Stack->Capacity * sizeof(keyword_tree_match));
}

function void
PushIntoKeywordTreeMatchStack(keyword_tree_match_stack *Stack, keyword *Keyword, u64 Distance)
{
    if ((Stack->Count + 1) > Stack->Capacity)
    {
        Stack->Capacity *= 2;
        Stack->Data = (keyword_tree_match *)realloc(Stack->Data, Stack->Capacity * sizeof(keyword_tree_match));
    }

    keyword_tree_match *Match = Stack->Data + Stack->Count++;
    Match->Keyword = Keyword;
    Match->Distance = Distance;
}

function keyword_tree_match *
PopFromKeywordTreeMatchStack(keyword_tree_match_stack *Stack)
{
    keyword_tree_match *Match = 0;

    if (Stack->Count)
    {
        Match = Stack->Data + --Stack->Count;
    }

    return Match;
}

function void
ResetKeywordTreeMatchStack(keyword_tree_match_stack *Stack)
{
    Stack->Count = 0;
}

function void
DestroyKeywordTreeMatchStack(keyword_tree_match_stack *Stack)
{
    free(Stack->Data);

    Stack->Capacity = 0;
    Stack->Count = 0;
    Stack->Data = 0;
}

//
// NOTE(philip): Keyword BK-Tree
//

function void
InitializeKeywordTree(keyword_tree *Tree, keyword_tree_type Type)
{
    Tree->Root = 0;

    switch (Type)
    {
        case KeywordTreeType_Hamming:
        {
            Tree->CalculateDistance = HammingDistance;
        } break;

        case KeywordTreeType_Edit:
        {
            Tree->CalculateDistance = EditDistance;
        } break;
    }
}

function void
InsertIntoKeywordTree(keyword_tree *Tree, keyword *Keyword)
{
    keyword_tree_node *Subtree = Tree->Root;
    keyword_tree_node *Parent = 0;
    u64 DistanceFromParent = 0;
    b32 Exists = false;

    while (Subtree)
    {
        u64 Distance = Tree->CalculateDistance(Subtree->Data->Word, Subtree->Data->Length,
                                               Keyword->Word, Keyword->Length);

        if (Distance == 0)
        {
            Subtree->IsActive = true;
            Exists = true;

            break;
        }

        Parent = Subtree;
        DistanceFromParent = Distance;

        Subtree = Subtree->FirstChild;
        while (Subtree && (Subtree->DistanceFromParent != DistanceFromParent))
        {
            Subtree = Subtree->NextSibling;
        }
    }

    if (!Exists)
    {
        keyword_tree_node *Node = (keyword_tree_node *)calloc(1, sizeof(keyword_tree_node));
        Node->Data = Keyword;
        Node->DistanceFromParent = DistanceFromParent;
        Node->IsActive = true;

        if (Parent)
        {
            Node->NextSibling = Parent->FirstChild;
            Parent->FirstChild = Node;
        }
        else
        {
            Tree->Root = Node;
        }
    }
}

function void
FindMatchesInKeywordTree(keyword_tree *Tree, keyword_tree_node_stack *Candidates, keyword_tree_match_stack *Matches,
                         keyword *Keyword)
{
    ResetKeywordTreeNodeStack(Candidates);
    ResetKeywordTreeMatchStack(Matches);

    for (keyword_tree_node *Subtree = Tree->Root;
         Subtree;
         Subtree = PopFromKeywordTreeNodeStack(Candidates))
    {
        u64 Distance = Tree->CalculateDistance(Subtree->Data->Word, Subtree->Data->Length,
                                               Keyword->Word, Keyword->Length);

        if (Subtree->IsActive && (Distance <= MAX_DISTANCE_THRESHOLD))
        {
            PushIntoKeywordTreeMatchStack(Matches, Subtree->Data, Distance);
        }

        u64 SearchRangeStart = (((s64)(Distance - MAX_DISTANCE_THRESHOLD) > 0) ? (Distance - MAX_DISTANCE_THRESHOLD) : 1);
        u64 SearchRangeEnd = Distance + MAX_DISTANCE_THRESHOLD;

        for (keyword_tree_node *Child = Subtree->FirstChild;
             Child;
             Child = Child->NextSibling)
        {
            if ((Child->DistanceFromParent >= SearchRangeStart) && (Child->DistanceFromParent <= SearchRangeEnd))
            {
                PushIntoKeywordTreeNodeStack(Candidates, Child);
            }
        }
    }
}

function void
RemoveFromKeywordTree(keyword_tree *Tree, keyword_tree_node_stack *Candidates, keyword *Keyword)
{
    ResetKeywordTreeNodeStack(Candidates);

    for (keyword_tree_node *Subtree = Tree->Root;
         Subtree;
         Subtree = PopFromKeywordTreeNodeStack(Candidates))
    {
        u64 Distance = Tree->CalculateDistance(Subtree->Data->Word, Subtree->Data->Length,
                                               Keyword->Word, Keyword->Length);

        if (!Distance)
        {
            Subtree->IsActive = false;
            break;
        }
        else
        {
            for (keyword_tree_node *Child = Subtree->FirstChild;
                 Child;
                 Child = Child->NextSibling)
            {
                if (Child->DistanceFromParent == Distance)
                {
                    PushIntoKeywordTreeNodeStack(Candidates, Child);
                    break;
                }
            }
        }
    }
}

function void
DestroySubtree(keyword_tree_node *Subtree)
{
    if (Subtree->FirstChild)
    {
        DestroySubtree(Subtree->FirstChild);
    }

    if (Subtree->NextSibling)
    {
        DestroySubtree(Subtree->NextSibling);
    }

    free(Subtree);
}

function void
DestroyKeywordTree(keyword_tree *Tree)
{
    if (Tree->Root)
    {
        DestroySubtree(Tree->Root);
    }

    Tree->Root = 0;
    Tree->CalculateDistance = 0;
}
