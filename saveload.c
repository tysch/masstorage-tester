/*
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Loads previous seed and size information
// Format: seed=uint32_t size=uint64_t
void load(uint32_t *seed, uint64_t *size, uint32_t *filesize)
{
	FILE * fp = fopen("savefile.txt", "r");
	int c;
	char str[64];
	int pos = 0;
	if(fp == NULL)
	{
		if((*seed == 0) || (*size == 0))
		{
			printf("\nNo save file found, and no run parameters specified, exiting now\n");
			fflush(stdout);
			exit(1);
		}
		else printf("\nNo save file found, initiating new test\n");
	}
	else
	{
		if((*seed) && (*size))
		{
			printf("\nSave file found, resume previous test (y/n)?\n");
			c = getchar();
			if(c == 'y')
			{
				*seed = 0;
				*size = 0;
				*filesize = 0;

				// Read last line in save file
				while (fgets(str, 64, fp) != NULL);
				// Parse "seed=xxxx size=xxxxxx filesize=xxxxx"
				for(pos = 0; pos < 40; pos++)
				{
					if((str[pos] >= '0') && (str[pos] <= '9')) break;
				}
				while ((str[pos] >= '0') && (str[pos] <= '9'))
				{
					*seed *= 10;
					*seed += str[pos] - '0';
					pos++;
				}
				for(; pos < 40; pos++)
				{
			        if((str[pos] >= '0') && (str[pos] <= '9')) break;
				}
				while ((str[pos] >= '0') && (str[pos] <= '9'))
				{
					*size *= 10;
					*size += str[pos] - '0';
					pos++;
				}
				for(; pos < 40; pos++)
				{
			        if((str[pos] >= '0') && (str[pos] <= '9')) break;
				}
				while ((str[pos] >= '0') && (str[pos] <= '9'))
				{
					*filesize *= 10;
					*filesize += str[pos] - '0';
					pos++;
				}
			}
		}
	}
}

void save(uint32_t seed, uint64_t size, uint32_t filesize)
{
	FILE * fp = fopen("savefile.txt", "a");
	if(fp == NULL)
	{
		printf("\nFailed to create save file\n");
		exit(1);
	}
	fprintf(fp, "seed=%i size=%lld filesize=%i\n", seed, (long long) size, filesize);
	fclose(fp);
}


