#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>



static uint32_t state0 = 1;
static uint32_t state1 = 0;
static uint32_t state2 = 0;
static uint32_t state3 = 0;

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
		if(x[i] == ' ') continue;
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


int main(void)
{	
	time_t start = time(0);
	uint32_t t;
	uint32_t j = 0;
	FILE* destFile;
	destFile = fopen("test.dat", "wb");
	uint32_t buf[1024];
	for(int i = 0; i < 1024*1024;i++)
	{
		for(int j = 0; j < 256; j++)
		{
			buf[j] = xorshift128();
		}
		fwrite(buf, 4, 256, destFile);
	}
	printf("%llu\n",tobytes("1000m"));
	printf("elapsed %.3f\n", (time(0) - start)/1.0);
	printf("%u \n", state0);	printf("%u \n", state1);	printf("%u \n", state2);	printf("%u \n", state3);
	return 0;
}
