#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


uint32_t state0;
uint32_t state1;
uint32_t state2;
uint32_t state3;

void reseed(uint64_t seed)
{
	state0 = seed & 0xFFFFFFFFU;
	state1 = seed >> 32;
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

void fill(uint64_t bytes)
{
	int n1gfiles = bytes / (1024*1024*1024);
	int fractsize = bytes % (1024*1024*1024);
	char filename[10];
	uint32_t buf[1024];
	for(int i = 0; i < n1gfiles; i++) //writing 1GiB files
	{
		sprintf(filename, "%i.dat",i);
		FILE* destfile = fopen(filename, "wb");
		for(int i = 0; i < 1024*1024;i++)
		{
			for(int j = 0; j < 256; j++)
			{
				buf[j] = xorshift128();
			}
			fwrite(buf, 4, 256, destfile);
		}
		fclose(destfile);
	}
	//writing remaining data
	int nblocks1k = fractsize / 1024;
	FILE* destfile = fopen("fract.dat", "wb");
	for(int i = 0; i < nblocks1k;i++)
	{
		for(int j = 0; j < 256; j++)
		{
			buf[j] = xorshift128();
		}
		fwrite(buf, 4, 256, destfile);
	}
	fclose(destfile);
}



int main(void)
{	
	reseed(1);
	fill(tobytes("1200M"));
	time_t start = time(0);
	printf("elapsed %.3f\n", (time(0) - start)/1.0);
	printf("%u \n", state0);	printf("%u \n", state1);	printf("%u \n", state2);	printf("%u \n", state3);
	return 0;
}
