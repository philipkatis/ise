function void
QueryList_Insert(query_list *List, query *Query)
{
    query_list_node *Node = (query_list_node *)calloc(1, sizeof(query_list_node));
    Node->Query = Query;
    Node->Next = List->Head;

    List->Head = Node;
}

function void
QueryList_Remove(query_list *List, query *Query)
{
    query_list_node *Previous = 0;
    for (query_list_node *Node = List->Head;
         Node;
         Node = Node->Next)
    {
        if (Node->Query == Query)
        {
            if (Previous)
            {
                Previous->Next = Node->Next;
            }
            else
            {
                List->Head = Node->Next;
            }

            free(Node);
            break;
        }

        Previous = Node;
    }
}

function void
QueryList_Destroy(query_list *List)
{
    query_list_node *Node = List->Head;
    while (Node)
    {
        query_list_node *Next = Node->Next;
        free(Node);
        Node = Next;
    }

    List->Head = 0;
}
