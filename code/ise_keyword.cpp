#define KEYWORD_TREE_CANDIDATE_ARRAY_STORAGE_SIZE 1024

function void
InitializeKeywordTreeCandidateArray(keyword_tree_candidate_array *Array)
{
    Array->Capacity = KEYWORD_TREE_CANDIDATE_ARRAY_STORAGE_SIZE;
    Array->Count = 0;
    Array->Data = (keyword_tree_node **)calloc(1, Array->Capacity * sizeof(keyword_tree_node *));
}

function void
PushCandidate(keyword_tree_candidate_array *Array, keyword_tree_node *Candidate)
{
    Assert((Array->Count + 1) <= Array->Capacity);
    Array->Data[Array->Count++] = Candidate;
}

function keyword_tree_node *
PopCandidate(keyword_tree_candidate_array *Array)
{
    keyword_tree_node *Candidate = 0;

    if (Array->Count)
    {
        Candidate = Array->Data[--Array->Count];
    }

    return Candidate;
}

function void
DestroyKeywordTreeCandidateArray(keyword_tree_candidate_array *Array)
{
    free(Array->Data);

    Array->Capacity = 0;
    Array->Count = 0;
    Array->Data = 0;
}

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

    InitializeKeywordTreeCandidateArray(&Tree->Candidates);
}

function void
InsertIntoKeywordTree(keyword_tree *Tree, keyword *Keyword)
{
    keyword_tree_node *Subtree = Tree->Root;
    keyword_tree_node *Parent = 0;
    u64 DistanceFromParent = 0;

    while (Subtree)
    {
        u64 Distance = Tree->CalculateDistance(Subtree->Data->Word, Subtree->Data->Length,
                                               Keyword->Word, Keyword->Length);

        Parent = Subtree;
        DistanceFromParent = Distance;

        Subtree = Subtree->FirstChild;
        while (Subtree && (Subtree->DistanceFromParent != DistanceFromParent))
        {
            Subtree = Subtree->NextSibling;
        }
    }

    keyword_tree_node *Node = (keyword_tree_node *)calloc(1, sizeof(keyword_tree_node));
    Node->Data = Keyword;
    Node->DistanceFromParent = DistanceFromParent;

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

function void
FindMatchesInKeywordTree(keyword_tree *Tree, keyword *Keyword, u64 *MatchCount, keyword_tree_match *Matches)
{
    *MatchCount = 0;

    for (keyword_tree_node *Subtree = Tree->Root;
         Subtree;
         Subtree = PopCandidate(&Tree->Candidates))
    {
        u64 Distance = Tree->CalculateDistance(Subtree->Data->Word, Subtree->Data->Length,
                                               Keyword->Word, Keyword->Length);

        if (Distance <= MAX_DISTANCE_THRESHOLD)
        {
            Assert((*MatchCount + 1) <= KEYWORD_TREE_MATCH_STORAGE_SIZE);

            keyword_tree_match *Match = Matches + (*MatchCount)++;
            Match->Keyword = Subtree->Data;
            Match->Distance = Distance;
        }

        u64 SearchRangeStart = (((s64)(Distance - MAX_DISTANCE_THRESHOLD) > 0) ? (Distance - MAX_DISTANCE_THRESHOLD) : 1);
        u64 SearchRangeEnd = Distance + MAX_DISTANCE_THRESHOLD;

        for (keyword_tree_node *Child = Subtree->FirstChild;
             Child;
             Child = Child->NextSibling)
        {
            if ((Child->DistanceFromParent >= SearchRangeStart) && (Child->DistanceFromParent <= SearchRangeEnd))
            {
                PushCandidate(&Tree->Candidates, Child);
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

    DestroyKeywordTreeCandidateArray(&Tree->Candidates);
}
