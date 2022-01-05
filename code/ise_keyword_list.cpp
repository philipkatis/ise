function void
KeywordList_Insert(keyword_list *List, keyword *Keyword)
{
    keyword_list_node *Node = (keyword_list_node *)calloc(1, sizeof(keyword_list_node));
    Node->Keyword = Keyword;
    Node->Next = List->Head;

    List->Head = Node;
}

function void
KeywordList_Destroy(keyword_list *List)
{
    keyword_list_node *Node = List->Head;
    while (Node)
    {
        keyword_list_node *Next = Node->Next;
        free(Node);
        Node = Next;
    }

    List->Head = 0;
}
