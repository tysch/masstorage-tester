#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

uint64_t totalbyteswritten = 0;
uint64_t totalbytesread = 0;
uint64_t ioerrors = 0;
uint64_t mismatcherrors = 0;


uint32_t state0;
uint32_t state1;
uint32_t state2;
uint32_t state3;

void reseed(uint32_t seed)
{
	state0 = seed;
	state1 = 0;
	state2 = 0;
	state3 = 0;
}

uint32_t xorshift128()
{
	uint32_t t = state3;
	t ^= t << 11;
	t ^= t >> 8;
	state3 = state2; state2 = state1; state1 = state0;
	t ^= state0;
	t ^= state0 >> 19;	
	state0 = t;
	return t;
}

uint64_t tobytes(char * x)
{
	uint64_t bytes = 0;
	uint64_t multiplier = 1;
	int len = 0;
	while(x[len++]);	
	for(int i = 0; i < len - 1; i++)
	{
		if((x[i] >= '0') && (x[i] < '9'))
		{
			bytes *= 10;
			bytes += (x[i] - 48);
		}
		if((x[i] > 'A') && (x[i] < 'z'))
		{
			switch (x[i])
			{
				case 'G':
					multiplier = 1024*1024*1024;
					break;
				case 'g':
					multiplier = 1024*1024*1024;
					break;
				case 'M':
					multiplier = 1024*1024;
					break;
				case 'm':
					multiplier = 1024*1024;
					break;
				case 'K':
					multiplier = 1024;
					break;
				case 'k':
					multiplier = 1024;
					break;
			}
		}
		bytes *= multiplier;
	}
	return bytes;
}

void tbwtostr(int mode, char * str)
{
	uint64_t i;	
	if(mode) i = totalbyteswritten;
	else i = totalbytesread;
	if(i < 1024) sprintf(str, "%i B", (int) i );
	if((i > 1024) && (i < 1024*1024)) sprintf(str, "%.1f KB", i/1024.0);
	if((i > 1024*1024) && (i < 1024*1024*1024)) sprintf(str, "%.1f MB", i/(1024.0*1024));
	if((i > 1024*1024*1024) && (i < 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.1f GB", i/(1024.0*1024.0*1024.0));
	if((i > 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.1f TB", i/(1024.0*1024.0*1024.0*1024.0));
}

void progress(int flag, double percent)
{
	char out[100];
	char tmp[100];
	percent *= 100;
	sprintf(out, "\r%.1f%%", percent);
	sprintf(tmp, "  ");
	strcat(out, tmp);
	tbwtostr(1, tmp);
	strcat(out, tmp);
	printf("                      ");
	printf("%s", out);
	fflush(stdout);
}

void fill(uint64_t bytes)
{
	int n32mfiles = bytes / (1024*1024*32);
	bytes %= (1024*1024*32);
	char filename[10];
	uint32_t buf[1024];
	for(int i = 0; i < n32mfiles; i++) //writing 32MiB files
	{
		sprintf(filename, "%i.dat",i);
		FILE* destfile = fopen(filename, "wb");
		for(int j = 0; j < 32*1024;j++)
		{
			for(int k = 0; k < 256; k++)
			{
				buf[k] = xorshift128();
			}
			totalbyteswritten += 4*fwrite(buf, 4, 256, destfile);
		}
		fclose(destfile);
		progress(0, (double)i/n32mfiles);
	}
	//writing remaining data
	sprintf(filename, "%i.dat",n32mfiles);
	FILE* destfile = fopen(filename, "wb");
	for(int i = 0; i < (bytes / 1024);i++)
	{
		for(int j = 0; j < 256; j++)
		{
			buf[j] = xorshift128();
		}
		totalbyteswritten += 4*fwrite(buf, 4, 256, destfile);
		progress(0, 1.0);
	}
	fclose(destfile);
}

int main(void)
{	
	char dig[20];
	reseed(1);
	fill(tobytes("1200M"));
	//printf("elapsed %.3f\n", (time(0) - start)/1.0);
	//printf("%u \n", state0);	printf("%u \n", state1);	printf("%u \n", state2);	printf("%u \n", state3);
	return 0;
}
