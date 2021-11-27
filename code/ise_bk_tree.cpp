#include "ise_bk_tree.h"

#include <stdlib.h>
#include <string.h>

/*

  NOTE(philip): This array stores all the keyword matching functions. The elements are stored in such
  a way to allow for indexing into the array using the MatchType enum values.

*/

global match_function_type MatchFunctions[3] =
{
    IsExactMatch,
    CalculateHammingDistance,
    CalculateEditDistance
};

bk_tree
BKTree_Create(MatchType MatchType)
{
    bk_tree Result = { };

    // TODO(philip): Not supported yet.
    Assert(MatchType != MT_EXACT_MATCH);
    Result.MatchFunction = MatchFunctions[MatchType];

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

bk_tree_node *
BKTree_Insert(bk_tree *Tree, keyword *Keyword)
{
    bk_tree_node *Node = 0;

    if (Tree->Root)
    {
        u64 KeywordLength = strlen(Keyword->Word);
        bk_tree_node *CurrentNode = Tree->Root;
        while (true)
        {
            s32 DistanceFromCurrentNode = CalculateEditDistance(CurrentNode->Keyword->Word,
                                                                strlen(CurrentNode->Keyword->Word),
                                                                Keyword->Word, KeywordLength);
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

  NOTE(philip): This small data structure is a utility to help with the implementation of the BK-Tree search
  function.

*/

struct candidate_stack_node
{
    bk_tree_node *Data;
    candidate_stack_node *Next;
};

struct candidate_stack
{
    candidate_stack_node *Head;
    u64 Count;
};

function candidate_stack
CreateCandidateStack()
{
    candidate_stack Result = { };
    return Result;
}

function void
PushCandidate(candidate_stack *Stack, bk_tree_node *Data)
{
    candidate_stack_node *Node = (candidate_stack_node *)calloc(1, sizeof(candidate_stack_node));
    Node->Data = Data;
    Node->Next = Stack->Head;

    Stack->Head = Node;
    ++Stack->Count;
}

function bk_tree_node *
PopCandidate(candidate_stack *Stack)
{
    if (Stack->Count)
    {
        candidate_stack_node *Node = Stack->Head;

        bk_tree_node *Data = Node->Data;
        candidate_stack_node *Head = Node->Next;

        free(Node);

        Stack->Head = Head;
        --Stack->Count;

        return Data;
    }
    else
    {
        return 0;
    }
}

keyword_list
BKTree_FindMatches(bk_tree *Tree, char *Word, u32 DistanceThreshold)
{
    keyword_list Matches = KeywordList_Create();
    candidate_stack Candidates = CreateCandidateStack();

    u64 WordLength = strlen(Word);

    bk_tree_node *CurrentCandidate = Tree->Root;
    while (CurrentCandidate)
    {
        u32 DistanceFromCurrentCandidate = Tree->MatchFunction(CurrentCandidate->Keyword->Word,
                                                               strlen(CurrentCandidate->Keyword->Word), Word,
                                                               WordLength);

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
                PushCandidate(&Candidates, Child);
            }
        }

        CurrentCandidate = PopCandidate(&Candidates);
    }

    return Matches;
}

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

void
BKTree_Destroy(bk_tree *Tree)
{
    if (Tree->Root)
    {
        BKTree_DestroyNode(Tree->Root);
    }

    Tree->Root = 0;
}


#if ISE_DEBUG
    /*

      NOTE(philip): This function is used only for debugging. It prints the specified number of tabs to the
      command-line. It is mostly used for visualization.

    */

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

    /*

      NOTE(philip): This function is used only for debugging. It is a tool for visualizing the contents of a BK-Tree
      node, by dumping them into the command-line. This is a recursive function that also prints all of the node's
      siblings and children.

    */

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

    void
    _BKTree_Visualize(bk_tree *Tree)
    {
        if (Tree->Root)
        {
            BKTree_VisualizeNode(Tree->Root, 0);
        }
    }
#endif
