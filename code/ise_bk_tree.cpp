function bk_tree
BKTree_Create(match_type Type)
{
    bk_tree Tree = {0};
    InitializeMemoryArena(&Tree.Arena, MB(1));

    switch (Type)
    {
        case MatchType_Hamming:
        {
            Tree.MatchFn = HammingDistance;
        } break;

        case MatchType_Edit:
        {
            Tree.MatchFn = EditDistance;
        } break;

        default:
        {
            Assert(false);
        } break;
    }

    return Tree;
}

function bk_tree_node *
AllocateNode(memory_arena *Arena, keyword *Keyword, u8 DistanceFromParent = 0, bk_tree_node *NextSibling = 0)
{
    bk_tree_node *Node = PushStruct(Arena, bk_tree_node);
    Node->IsActive = true;
    Node->Keyword = Keyword;
    Node->FirstChild = 0;
    Node->NextSibling = NextSibling;
    Node->DistanceFromParent = DistanceFromParent;

    return Node;
}

function void
BKTree_Insert(bk_tree *Tree, keyword *Keyword)
{
    if (Tree->Root)
    {
        bk_tree_node *Node = Tree->Root;
        for (;;)
        {
            keyword *NodeKeyword = Node->Keyword;
            u8 Distance = Tree->MatchFn(NodeKeyword->Word, NodeKeyword->Length, Keyword->Word, Keyword->Length);

            if (Distance == 0)
            {
                // NOTE(philip): The keyword is already in the BKTree. So reactivate it.
                // TODO(philip): When keyword removal from the hash table, update the link here.
                Node->IsActive = true;
                break;
            }
            else
            {
                b32 ChildExists = false;
                for (bk_tree_node *Child = Node->FirstChild;
                     Child;
                     Child = Child->NextSibling)
                {
                    if (Child->DistanceFromParent == Distance)
                    {
                        Node = Child;
                        ChildExists = true;

                        break;
                    }
                }

                if (!ChildExists)
                {
                    Node->FirstChild = AllocateNode(&Tree->Arena, Keyword, Distance, Node->FirstChild);
                    break;
                }
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
        u8 Distance = Tree->MatchFn(Candidate->Keyword->Word, Candidate->Keyword->Length,
                                    Keyword->Word, Keyword->Length);
        if (Candidate->IsActive && (Distance <= DistanceThreshold))
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
            if ((Child->DistanceFromParent >= ChildrenRangeStart) &&
                (Child->DistanceFromParent <= ChildrenRangeEnd))
            {
                PushCandidate(&Candidates, Child);
            }
        }
    }

    free(Candidates.Data);

    return Result;
}

//
// NOTE(philip): Deactivates a keyword from the tree. DOES NOT REMOVE IT.
//

function void
BKTree_Deactivate(bk_tree *Tree, keyword *Keyword)
{
    candidate_stack Candidates = { };
    for (bk_tree_node *Candidate = Tree->Root;
         Candidate;
         Candidate = PopCandidate(&Candidates))
    {
        u8 Distance = Tree->MatchFn(Candidate->Keyword->Word, Candidate->Keyword->Length,
                                    Keyword->Word, Keyword->Length);
        if (Distance == 0)
        {
            Candidate->IsActive = false;
            break;
        }
        else
        {
            for (bk_tree_node *Child = Candidate->FirstChild;
                 Child;
                 Child = Child->NextSibling)
            {
                if (Child->DistanceFromParent == Distance)
                {
                    PushCandidate(&Candidates, Child);
                    break;
                }
            }
        }
    }
}

function void
BKTree_Destroy(bk_tree *Tree)
{
    DestroyMemoryArena(&Tree->Arena);
    Tree->Root = 0;
    Tree->MatchFn = 0;
}
