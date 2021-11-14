#include "ise_bk_tree.h"
#include "ise_match.h"

#include <stdlib.h>
#include <string.h>

bk_tree
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

bk_tree_node *
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

keyword_list
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

    void
    _BKTree_Visualize(bk_tree *Tree)
    {
        if (Tree->Root)
        {
            BKTree_VisualizeNode(Tree->Root, 0);
        }
    }
#endif
