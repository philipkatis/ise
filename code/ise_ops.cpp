//
// NOTE(philip): Parses and deduplicates document words.
//

function keyword_table
ParseDocument(char *Document)
{
    // NOTE(philip): Find how many words are in the document, and tokenize on the whitespace.
    // TODO(philip): Currently assuming: 1) Each document has atleast one word, 2) all words are separated
    // by a single space character and 3) that the document is null-terminated.
    u64 WordCount = 1;

    for (char *Character = Document;
         *Character;
         ++Character)
    {
        if (*Character == ' ')
        {
            *Character = 0;
            ++WordCount;
        }
    }

    // NOTE(philip): Create a table that will store the words.
    // TODO(philip): The table stores more information than it is necessary. If that becomes a problem swap it out
    // for a more suitable alternative.
    keyword_table Words = KeywordTable_Create(WordCount);

    // NOTE(philip): Going through each word in the document and adding it to the table.
    // TODO(philip): Assuming that the document immediately starts with a word.
    char *Word = Document;

    for (u64 Index = 0;
         Index < WordCount;
         ++Index)
    {
        // TODO(philip): If we calculate the length of the word outsize of the table and pass it as a parameter,
        // we can skip an additional iteration over the word.
        KeywordTable_Insert(&Words, Word);

        while (*Word)
        {
            ++Word;
        }

        ++Word;
    }

    return Words;
}

//
// NOTE(philip): Updates the query tree with the newly found keyword.
//

// TODO(philip): Make a match type enumerator to use, instead of the type.
function void
RegisterFoundKeyword(query_tree *MatchingQueries, keyword *Keyword, u32 Type, u32 Threshold)
{
    for (query_list_node *Node = Keyword->Queries.Head;
         Node;
         Node = Node->Next)
    {
        // NOTE(philip): Iterate over all the queries that the found keyword is part of, and if they match
        // our search parameters, register it..
        // TODO(philip): Doing some extra work here for hash table finds. Maybe split this function into two,
        // one for aproximate and one for exact matching.
        query *Query = Node->Query;
        if ((GetQueryType(Query) == Type) && (GetQueryDistance(Query) == Threshold))
        {
            // NOTE(philip): Attempt to insert the query to the answers, in case it doesn't exist.
            u32 KeywordCount = GetQueryKeywordCount(Query);
            query_tree_insert_result InsertResult = QueryTree_Insert(MatchingQueries, Query->ID, KeywordCount,
                                                                     Type, 0);
            // TODO(philip): Is this if statement needed? Replace with assert.
            query *Answer = InsertResult.Query;
            if (Answer)
            {
                // NOTE(philip): Iterate over the keywords in the query, and get the index of the keyword we
                // found. Then update the respective boolean.
                for (u32 Index = 0;
                     Index < KeywordCount;
                     ++Index)
                {
                    if (Query->Keywords[Index] == Keyword)
                    {
                        Answer->HasKeyword[Index] = true;
                        break;
                    }
                }
            }
        }
    }
}

// NOTE(philip): Returns the index of the hamming tree that contains words with this length.
#define GetHammingTreeIndex(Length) (Length - MIN_KEYWORD_LENGTH)

//
// NOTE(philip): Checks a document against the stored queries and produces an answer.
//

function void
FindDocumentAnswer(result_queue *Results, keyword_table *Keywords, bk_tree *HammingTrees, bk_tree *EditTree,
         u32 DocumentID, char *Document)
{
    // NOTE(philip): Parse the document and store it's words in a table.
    keyword_table DocumentWords = ParseDocument(Document);

    // NOTE(philip): Use a query tree, to store what keywords for what query have been found.
    // TODO(philip): The tree stores more information that it is necessary. If that becomes a problem swap it out
    // for a more suitable alternative.
    query_tree MatchingQueries = QueryTree_Create();

    // NOTE(philip): Iterate over the document words and look them up in the data structures.
    for (keyword_iterator Iterator = IterateAllKeywords(&DocumentWords);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *DocumentWord = GetValue(&Iterator);

        // NOTE(philip): Exact Matching
        {
            // NOTE(philip): Look the keyword up in the query keyword table for a possible exact match.
            // TODO(philip): Again in this case, we could also provide the length of the word.
            keyword *FoundKeyword = KeywordTable_Find(Keywords, DocumentWord->Word);
            if (FoundKeyword)
            {
                // NOTE(philip): If the keyword was found, we need to update the queries that contain it.
                RegisterFoundKeyword(&MatchingQueries, FoundKeyword, 0, 0);
            }
        }

        // NOTE(philip): Approximate Matching
        {
            // NOTE(philip): Find the Hamming tree that we need to search.
            bk_tree *HammingTree = HammingTrees + GetHammingTreeIndex(DocumentWord->Length);

            // NOTE(philip): Make a search for each possible maximum threshold.
            // TODO(philip): Making one pass on the hamming tree and then the edit tree might be bad.
            // Maybe split this loop into two to increase the chances of the tree being hot in the cache.
            for (u32 Threshold = 1;
                 Threshold <= MAX_DISTANCE_THRESHOLD;
                 ++Threshold)
            {
                // NOTE(philip): Hamming Tree
                {
                    // NOTE(philip): Find the keywords that match our parameters.
                    keyword_list FoundKeywords = BKTree_FindMatches(HammingTree, DocumentWord, Threshold);

                    // NOTE(philip): Iterate over all the keywords in the list, and register them.
                    for (keyword_list_node *Node = FoundKeywords.Head;
                         Node;
                         Node = Node->Next)
                    {
                        keyword *FoundKeyword = Node->Keyword;
                        RegisterFoundKeyword(&MatchingQueries, FoundKeyword, 1, Threshold);
                    }

                    KeywordList_Destroy(&FoundKeywords);
                }

                // NOTE(philip): Edit Tree
                {
                    // NOTE(philip): Find the keywords that match our parameters.
                    keyword_list FoundKeywords = BKTree_FindMatches(EditTree, DocumentWord, Threshold);

                    // NOTE(philip): Iterate over all the keywords in the list, and register them.
                    for (keyword_list_node *Node = FoundKeywords.Head;
                         Node;
                         Node = Node->Next)
                    {
                        keyword *FoundKeyword = Node->Keyword;
                        RegisterFoundKeyword(&MatchingQueries, FoundKeyword, 2, Threshold);
                    }

                    KeywordList_Destroy(&FoundKeywords);
                }
            }
        }
    }

    KeywordTable_Destroy(&DocumentWords);

    result Result = QueryTree_CompileResult(&MatchingQueries, DocumentID);
    QueryTree_Destroy(&MatchingQueries);

    PushResult(Results, Result);
}
