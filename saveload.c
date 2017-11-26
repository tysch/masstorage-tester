/*
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "options.h"

// Loads previous seed and size information
// Format: seed=uint32_t size=uint64_t
void load(struct options_s * options)
{
    char savefile[PATH_LENGTH];
    strcpy(savefile, options->logpath);
    strcat(savefile, "savefile.txt");
    FILE * fp = fopen(savefile, "r");
    int c = 0;
    char str[SAVESTR_LEN];
    int pos = 0;
    if(fp == NULL)
    {
        if((options->seed == 0) || (options->totsize == 0))
        {
            printf("\nNo save file found, and no run parameters specified, exiting now\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        else printf("\nNo save file found, initiating new test\n");
    }
    else
    {
        if((options->seed) && (options->totsize) && !(options->background))
        {
            fflush(stdin);
            printf("\nSave file found, resume previous test (y/n)?\n");
            while((c != 'y') && (c != 'n') ) c = getchar();
            if(c == 'y')
            {
                options->seed = 0;
                options->totsize = 0;
                options->bufsize = 0;

                // Read last line in save file
                while (fgets(str, SAVESTR_LEN, fp) != NULL);
                // Parse "seed=xxxx size=xxxxxx filesize=xxxxx"
                for(pos = 0; pos < SAVESTR_LEN - 1; pos++)
                {
                    if((str[pos] >= '0') && (str[pos] <= '9')) break;
                }
                while ((str[pos] >= '0') && (str[pos] <= '9'))
                {
                    options->seed *= 10;
                    options->seed += str[pos] - '0';
                    pos++;
                }
                for(; pos < SAVESTR_LEN - 1; pos++)
                {
                    if((str[pos] >= '0') && (str[pos] <= '9')) break;
                }
                while ((str[pos] >= '0') && (str[pos] <= '9'))
                {
                    options->totsize *= 10;
                    options->totsize += str[pos] - '0';
                    pos++;
                }
                for(; pos < SAVESTR_LEN - 1; pos++)
                {
                    if((str[pos] >= '0') && (str[pos] <= '9')) break;
                }
                while ((str[pos] >= '0') && (str[pos] <= '9'))
                {
                    options->bufsize *= 10;
                    options->bufsize += str[pos] - '0';
                    pos++;
                }
            }
        }
    }
}

void save(struct options_s * options)
{
    char savefile[PATH_LENGTH];
    strcpy(savefile, options->logpath);
    strcat(savefile, "savefile.txt");
    FILE * fp = fopen(savefile, "a");
    if(fp == NULL)
    {
        printf("\nFailed to create save file\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "seed=%i size=%lld filesize=%i\n", options->seed, (long long) options->totsize, options->bufsize);
    fclose(fp);
}
