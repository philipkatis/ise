function bk_tree
BKTree_Create(bk_tree_type Type)
{
    bk_tree Result = { };

    switch (Type)
    {
        case BKTree_Type_Hamming:
        {
            Result.MatchFunction = CalculateHammingDistance;
        } break;

        case BKTree_Type_Edit:
        {
            Result.MatchFunction = CalculateEditDistance;
        } break;

        default:
        {
            Assert(false);
        } break;
    }

    return Result;
}

function bk_tree_node *
AllocateNode(keyword *Keyword, s32 DistanceFromParent, bk_tree_node *NextSibling)
{
    bk_tree_node *Result = (bk_tree_node *)calloc(1, sizeof(bk_tree_node));
    Result->Keyword = Keyword;
    Result->NextSibling = NextSibling;
    Result->DistanceFromParent = DistanceFromParent;

    return Result;
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

            s32 Distance = Tree->MatchFunction(CurrentKeyword->Word, CurrentKeyword->Length,
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
                CurrentNode->FirstChild = AllocateNode(Keyword, Distance, CurrentNode->FirstChild);
                break;
            }
        }
    }
    else
    {
        Tree->Root = AllocateNode(Keyword, 0, 0);
    }
}

// TODO(philip): If this data structure becomes a problem, switch to an array. The array could either double in
// size or grow based on the tree height and max child count.

struct candidate_stack_node
{
    bk_tree_node *Data;
    candidate_stack_node *Next;
};

struct candidate_stack
{
    candidate_stack_node *Head;
};

function void
PushCandidate(candidate_stack *Stack, bk_tree_node *Data)
{
    candidate_stack_node *Node = (candidate_stack_node *)calloc(1, sizeof(candidate_stack_node));
    Node->Data = Data;
    Node->Next = Stack->Head;

    Stack->Head = Node;
}

function bk_tree_node *
PopCandidate(candidate_stack *Stack)
{
    bk_tree_node *Data = 0;

    if (Stack->Head)
    {
        Data = Stack->Head->Data;

        candidate_stack_node *Next = Stack->Head->Next;
        free(Stack->Head);
        Stack->Head = Next;
    }

    return Data;
}


// TODO(philip): This function is currently the bottleneck. 74% of cycles are spent calculating the edit distance.
// Waiting for the 3rd assignment to choose a proper solution. Maybe calling this function once per match check (we
// currently do it 3 times per keyword for all the possible distances).

function keyword_list
BKTree_FindMatches(bk_tree *Tree, keyword *Keyword, s32 DistanceThreshold)
{
    keyword_list Result = { };
    candidate_stack Candidates = { };

    for (bk_tree_node *Candidate = Tree->Root;
         Candidate;
         Candidate = PopCandidate(&Candidates))
    {
        s32 Distance = Tree->MatchFunction(Candidate->Keyword->Word, Candidate->Keyword->Length,
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
            if ((Child->DistanceFromParent >= ChildrenRangeStart) && (Child->DistanceFromParent <= ChildrenRangeEnd))
            {
                PushCandidate(&Candidates, Child);
            }
        }
    }

    return Result;
}

function void
DestroyNode(bk_tree_node *Node)
{
    if (Node->FirstChild)
    {
        DestroyNode(Node->FirstChild);
    }

    if (Node->NextSibling)
    {
        DestroyNode(Node->NextSibling);
    }

    free(Node);
}

function void
BKTree_VisualizeNode(bk_tree_node *Node, u64 Depth)
{
    PrintTabs(Depth);
    printf("Word: %s, DistanceFromParent: %d, Depth: %llu\n", Node->Keyword->Word, Node->DistanceFromParent,
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
    if (Tree->Root)
    {
        DestroyNode(Tree->Root);
    }

    Tree->Root = 0;
    Tree->MatchFunction = 0;
}
