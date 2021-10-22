#ifndef ISE_H
#define ISE_H

// NOTE(philip): This limit is based on the SIGMOD 2013 Programming Contest referenced by the
// project assignment. We assume that this limit does not include the zero-termination character.
#define MAX_KEYWORD_LENGTH 31

/*

  NOTE(philip): The max Levenshtein distance between two keywords cannot exceed the length
  of the bigger one. A Levenshtein distance of zero, means the two keywords are the same and
  thus cannot be in the tree at the same time.

  TODO(philip): There are several problems with this approach. Firstly, storing 30 8-byte pointers
  is not an attractive option. A better alternative would be to store all the tree nodes in a
  single continuous chunck of memory and using a 32-bit index into it. This reduces the size of this
  structure from 272 bytes to 160 bytes. The 32-bit index can access up to 4 billion entries. Plenty.
  Furthermore, we can improve this further with a custom allocator, cutting down on expensive OS
  allocations. Another benefit to this approach is deallocation. Since we are (probably) expected to
  deallocate the memory "properly", this can speed things up since we don't have to use a recursive
  function that visits each node.

*/

struct bk_tree_node
{
    char Word[MAX_KEYWORD_LENGTH + 1];
    bk_tree_node *Children[MAX_KEYWORD_LENGTH - 1];
};

#endif
