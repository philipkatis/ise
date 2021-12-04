/*

  NOTE(philip): The following code is from the sigmod contest files. It it used as a test for the ISE
  implementation. The funcionality is exactly the same. The only changed made are to the code style.

*/

#include "ise.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

function s32
GetCurrentTimeMS()
{
    struct timeval Time;
    gettimeofday(&Time, 0);

    return Time.tv_sec * 1000 + Time.tv_usec / 1000;
}

function void
PrintFormattedTime(s32 Time)
{
    s32 RemainingTime = Time;

    s32 Hours = RemainingTime / (1000 * 60 * 60);
    RemainingTime %= (1000 * 60 * 60);

    s32 Minutes = RemainingTime / (1000 * 60);
    RemainingTime %= (1000 * 60);

    s32 Seconds = RemainingTime / 1000;
    RemainingTime %= 1000;

    s32 Milliseconds = RemainingTime;

    printf("[%.2d:%.2d:%.2d:%.3d] - %d ms", Hours, Minutes, Seconds, Milliseconds, Time);
}

global char TempStorage[MAX_DOCUMENT_LENGTH];

function void
Run(const char *FilePath)
{
    printf("Starting test...\n");

    FILE *File = fopen(FilePath, "rt");
    if (!File)
    {
        printf("[Error]: Could not open file (Path: %s)\n", FilePath);
        return;
    }

    int StartTime = GetCurrentTimeMS();

    InitializeIndex();

#if TODO
	unsigned int first_result=0;
	int num_cur_results=0;
	const int max_results=100;
	bool cur_results_ret[max_results];
	unsigned int cur_results_size[max_results];
	unsigned int* cur_results[max_results];
#endif

    while (true)
    {
        char CommandType;
        u32 ID;

        if (fscanf(File, "%c %u ", &CommandType, &ID) == EOF)
        {
            break;
        }

#if TODO
        if (num_cur_results && (CommandType == 's' || CommandType == 'e'))
        {
            for (s32 Index = 0;
                 Index < num_cur_results;
                 ++Index)
            {
                u32 DocumentID = 0;
                u32 QueryCount = 0;
                u32* Queries = 0;

				ErrorCode Result = GetNextAvailRes(&DocumentID, &QueryCount, &Queries);
                if (Result == EC_NO_AVAIL_RES)
                {
                    printf("[Error]: GetNextAvailRes() did not retrieve any results!\n");
                    return;
                }
                else if (Result != EC_SUCCESS)
                {
                    printf("[Error]: GetNextAvailRes() failed.\n");
                    return;
				}

				if(DocumentID < first_result || DocumentID - first_result>=(unsigned int)num_cur_results)
				{
					printf("The call to GetNextAvailRes() returned unknown document ID %u.\n", DocumentID);
					fflush(NULL);
					return;
				}
				if(cur_results_ret[DocumentID - first_result])
				{
					printf("The call to GetNextAvailRes() returned document (ID=%u) that has been delivered before.\n", DocumentID);
					fflush(NULL);
					return;
				}

				bool flag_error=false;

				if(QueryCount != cur_results_size[DocumentID - first_result])
				{
					flag_error=true;
				}

				for(int j=0;j<(int)QueryCount && !flag_error;j++)
				{
					if (Queries[j]!=cur_results[DocumentID - first_result][j])
					{
						flag_error=true;
					}
				}

				if(flag_error)
				{
					printf("The call to GetNextAvailRes() returned incorrect result for document ID %u.\n", DocumentID);
					printf("Your answer is: "); for(int j=0;j<(int)QueryCount;j++) {if(j)printf(" "); printf("%u", Queries[j]);} printf("\n");
					printf("The correct answer is: "); for(int j=0;j<(int)cur_results_size[DocumentID - first_result];j++) {if(j)printf(" "); printf("%u", cur_results[DocumentID - first_result][j]);} printf("\n");
					fflush(NULL);
					return;
				}

				cur_results_ret[DocumentID - first_result]=true;
				if(QueryCount && Queries) free(Queries);
			}

			for(int i=0;i<num_cur_results;i++) {free(cur_results[i]); cur_results[i]=0; cur_results_size[i]=0; cur_results_ret[i]=false;}
			num_cur_results=0;
		}
#endif

        if (CommandType == 's')
        {
            s32 Type = 0;
            s32 Distance = 0;

            if (fscanf(File, "%d %d %*d %[^\n\r] ", &Type, &Distance, TempStorage) == EOF)
            {
                printf("[Error]: File contains corrupted data! (Path: %s)\n", FilePath);
                return;
            }

            ErrorCode Result = StartQuery(ID, TempStorage, (MatchType)Type, Distance);
            if (Result != EC_SUCCESS)
            {
                printf("[Error]: StartQuery() has failed!\n");
                return;
            }
        }
        else if (CommandType == 'e')
        {
            ErrorCode Result = EndQuery(ID);
            if (Result != EC_SUCCESS)
            {
                printf("[Error]: EndQuery() has failed!\n");
                return;
            }
        }
        else if (CommandType == 'm')
        {
            if (fscanf(File, "%*u %[^\n\r] ", TempStorage) == EOF)
            {
                printf("[Error]: File contains corrupted data! (Path: %s)\n", FilePath);
                return;
            }

            ErrorCode Result = MatchDocument(ID, TempStorage);
            if (Result != EC_SUCCESS)
            {
                printf("[Error]: MatchDocument() has failed!\n");
                return;
            }
        }
        else if (CommandType == 'r')
        {
            u32 ResultCount = 0;

            if (fscanf(File, "%u ", &ResultCount) == EOF)
            {
                printf("Corrupted Test File.\n");
                return;
			}

            unsigned int qid;
#if TODO
            if (num_cur_results==0)
            {
                first_result = ID;
            }

			cur_results_ret[num_cur_results]=false;
			cur_results_size[num_cur_results] = ResultCount;
			cur_results[num_cur_results]=(unsigned int*)malloc(ResultCount * sizeof(unsigned int));
#endif

			for(int i=0;i<(int)ResultCount;i++)
			{
				if(EOF==fscanf(File, "%u ", &qid))
				{
					printf("Corrupted Test File.\n");
					fflush(NULL);
					return;
				}

#if TODO
				cur_results[num_cur_results][i]=qid;
#endif
			}

#if TODO
            num_cur_results++;
#endif
        }
        else
        {
            printf("[Error]: Unknown command type! (Command: %c)\n", CommandType);
            return;
        }
    }

    DestroyIndex();

    s32 ElapsedTime = GetCurrentTimeMS() - StartTime;
    fclose(File);

    printf("Test complete! PASSED\n");

    printf("Elapsed Time: ");
    PrintFormattedTime(ElapsedTime);
    printf("\n");
}

s32
main(s32 ArgumentCount, char **Arguments)
{
    if (ArgumentCount <= 1)
    {
        Run("./data/test_data.txt");
    }
    else
    {
        Run(Arguments[1]);
    }

    return 0;
}
