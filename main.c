#include <stdio.h>
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

int main(void)
{
	time_t start = clock();
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
	printf("elapsed %.3f\n", (clock() - start)/1000000.0);
	printf("%u \n", state0);	printf("%u \n", state1);	printf("%u \n", state2);	printf("%u \n", state3);
	return 0;
}
