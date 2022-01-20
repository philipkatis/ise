#ifndef ISE_BK_TREE_H
#define ISE_BK_TREE_H

struct bk_tree_node
{
    b64 IsActive;
    keyword *Keyword;

    bk_tree_node *FirstChild;
    bk_tree_node *NextSibling;

    u8 DistanceFromParent;
    u8 Padding01;
    u16 Padding02;
    u32 Padding03;

    u64 Padding[3];
};

static_assert(sizeof(bk_tree_node) == 64);

typedef u8 (*match_fn)(char *A, u8 LengthA, char *B, u8 LengthB);

struct candidate_stack
{
    bk_tree_node **Data;
    u64 Capacity;
    u64 Count;
};

struct bk_tree
{
    memory_arena Arena;
    bk_tree_node *Root;
    match_fn MatchFn;
};

#endif
