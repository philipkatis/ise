function void
KeywordTable(void)
{
    // NOTE(philip): In depth test of table creation, destruction, insertion and duplicate insertion.
    {
        keyword_table Table = KeywordTable_Create(2);

        TEST_CHECK(Table.Buckets != 0);
        TEST_CHECK(Table.BucketCount == 2);
        TEST_CHECK(Table.ElementCount == 0);

        keyword_table_insert_result InsertResult = KeywordTable_Insert(&Table, "hello");

        TEST_CHECK(InsertResult.Keyword != 0);
        TEST_CHECK(InsertResult.Exists == false);

        TEST_CHECK(Table.ElementCount == 1);
        TEST_CHECK(Table.Buckets[0] == 0);
        TEST_CHECK(Table.Buckets[1] != 0);
        TEST_CHECK(Table.Buckets[1]->Next == 0);
        TEST_CHECK(strcmp(Table.Buckets[1]->Data.Word, "hello") == 0);

        InsertResult = KeywordTable_Insert(&Table, "hello");

        TEST_CHECK(InsertResult.Keyword != 0);
        TEST_CHECK(InsertResult.Exists == true);
        TEST_CHECK(strcmp(InsertResult.Keyword->Word, "hello") == 0);

        KeywordTable_Destroy(&Table);

        TEST_CHECK(Table.Buckets == 0);
        TEST_CHECK(Table.BucketCount == 0);
        TEST_CHECK(Table.ElementCount == 0);
    }

    // NOTE(philip): Multiple keyword insertions with rehashing. Search.
    {
        keyword_table Table = KeywordTable_Create(2);

        keyword_table_insert_result HelloInsertResult = KeywordTable_Insert(&Table, "hello");
        keyword_table_insert_result WorldInsertResult = KeywordTable_Insert(&Table, "world");
        keyword_table_insert_result BazingaInsertResult = KeywordTable_Insert(&Table, "bazinga");

        TEST_CHECK(Table.BucketCount == 4);
        TEST_CHECK(Table.ElementCount == 3);
        TEST_CHECK(Table.Buckets[0] == 0);

        TEST_CHECK(Table.Buckets[1] != 0);
        TEST_CHECK(&Table.Buckets[1]->Data == BazingaInsertResult.Keyword);
        TEST_CHECK(Table.Buckets[1]->Next != 0);
        TEST_CHECK(&Table.Buckets[1]->Next->Data == WorldInsertResult.Keyword);
        TEST_CHECK(Table.Buckets[1]->Next->Next != 0);
        TEST_CHECK(&Table.Buckets[1]->Next->Next->Data == HelloInsertResult.Keyword);

        keyword *Keyword = KeywordTable_Find(&Table, "test");
        TEST_CHECK(Keyword == 0);

        Keyword = KeywordTable_Find(&Table, "bazinga");
        TEST_CHECK(Keyword != 0);

        KeywordTable_Destroy(&Table);

    }
}
