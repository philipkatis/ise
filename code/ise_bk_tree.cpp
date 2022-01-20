function bk_tree
BKTree_Create(bk_tree_type Type)
{
    bk_tree Tree = { };

    InitializeMemoryArena(&Tree.Arena, MB(1));

    switch (Type)
    {
        case BKTree_Type_Hamming:
        {
            Tree.MatchFunction = HammingDistance;
        } break;

        case BKTree_Type_Edit:
        {
            Tree.MatchFunction = EditDistance;
        } break;

        default:
        {
            Assert(false);
        } break;
    }

    return Tree;
}

function bk_tree_node *
AllocateNode(memory_arena *Arena, keyword *Keyword, u64 DistanceFromParent = 0, bk_tree_node *NextSibling = 0)
{
    bk_tree_node *Node = PushStruct(Arena, bk_tree_node);
    Node->Keyword = Keyword;
    Node->NextSibling = NextSibling;
    Node->DistanceFromParent = DistanceFromParent;

    return Node;
}

function void
BKTree_Insert(bk_tree *Tree, keyword *Keyword)
{
    if (Tree->Root)
    {
        bk_tree_node *CurrentNode = Tree->Root;
        while (true)
        {
            keyword *CurrentKeyword = CurrentNode->Keyword;

            u8 Distance = Tree->MatchFunction(CurrentKeyword->Word, CurrentKeyword->Length,
                                              Keyword->Word, Keyword->Length);
            b32 DistanceChildExists = false;

            for (bk_tree_node *Child = CurrentNode->FirstChild;
                 Child;
                 Child = Child->NextSibling)
            {
                if (Child->DistanceFromParent == Distance)
                {
                    CurrentNode = Child;
                    DistanceChildExists = true;

                    break;
                }
            }

            if (!DistanceChildExists)
            {
                CurrentNode->FirstChild = AllocateNode(&Tree->Arena, Keyword, Distance, CurrentNode->FirstChild);
                break;
            }
        }
    }
    else
    {
        Tree->Root = AllocateNode(&Tree->Arena, Keyword);
    }
}

function void
PushCandidate(candidate_stack *Stack, bk_tree_node *Node)
{
    if ((Stack->Count + 1) > Stack->Capacity)
    {
        u64 NewCapacity = (Stack->Capacity ? (2 * Stack->Capacity) : 1024);

        void *Memory = malloc(NewCapacity * sizeof(bk_tree_node *));
        memcpy(Memory, Stack->Data, Stack->Count * sizeof(bk_tree_node *));
        free(Stack->Data);

        Stack->Data = (bk_tree_node **)Memory;
        Stack->Capacity = NewCapacity;
    }

    Stack->Data[Stack->Count] = Node;
    ++Stack->Count;
}

function bk_tree_node *
PopCandidate(candidate_stack *Stack)
{
    bk_tree_node *Result = 0;

    if (Stack->Count)
    {
        Result = Stack->Data[Stack->Count - 1];
        --Stack->Count;
    }

    return Result;
}

// TODO(philip): This function is currently the bottleneck. 74% of cycles are spent calculating the edit distance.
// Waiting for the 3rd assignment to choose a proper solution. Maybe calling this function once per match check (we
// currently do it 3 times per keyword for all the possible distances).

function keyword_list
BKTree_FindMatches(bk_tree *Tree, keyword *Keyword, s32 DistanceThreshold)
{
    keyword_list Result = { };

    // TODO(philip): Move to bk tree structure.
    candidate_stack Candidates = { };

    for (bk_tree_node *Candidate = Tree->Root;
         Candidate;
         Candidate = PopCandidate(&Candidates))
    {
        u8 Distance = Tree->MatchFunction(Candidate->Keyword->Word, Candidate->Keyword->Length,
                                          Keyword->Word, Keyword->Length);
        if (Distance <= DistanceThreshold)
        {
            KeywordList_Insert(&Result, Candidate->Keyword);
        }

        s32 ChildrenRangeStart = Distance - DistanceThreshold;
        ChildrenRangeStart = (ChildrenRangeStart < 0) ? 1 : ChildrenRangeStart;
        s32 ChildrenRangeEnd = Distance + DistanceThreshold;

        for (bk_tree_node *Child = Candidate->FirstChild;
             Child;
             Child = Child->NextSibling)
        {
            if (((u8)Child->DistanceFromParent >= ChildrenRangeStart) &&
                ((u8)Child->DistanceFromParent <= ChildrenRangeEnd))
            {
                PushCandidate(&Candidates, Child);
            }
        }
    }

    free(Candidates.Data);

    return Result;
}

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

function void
BKTree_Visualize(bk_tree *Tree)
{
    if (Tree->Root)
    {
        BKTree_VisualizeNode(Tree->Root, 0);
    }
}

function void
BKTree_Destroy(bk_tree *Tree)
{
    DestroyMemoryArena(&Tree->Arena);

    Tree->Root = 0;
    Tree->MatchFunction = 0;
}
