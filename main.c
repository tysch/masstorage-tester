#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

uint64_t totalbyteswritten = 0;
uint64_t mismatcherrors = 0;
uint64_t ioerrors = 0;
uint64_t passage = 0;

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

void bytestostr(uint64_t bytes, char * str)
{
	if(bytes < 1024) sprintf(str, "%i B", (int) bytes );
	if((bytes > 1024) && (bytes < 1024*1024)) sprintf(str, "%.1f KiB", bytes/1024.0);
	if((bytes > 1024*1024) && (bytes < 1024*1024*1024)) sprintf(str, "%.1f MiB", bytes/(1024.0*1024));
	if((bytes > 1024*1024*1024) && (bytes < 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.1f GiB", bytes/(1024.0*1024.0*1024.0));
	if((bytes > 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.1f TiB", bytes/(1024.0*1024.0*1024.0*1024.0));
}

void progress(int flag, double percent)
{
	percent *= 100;  
	char status[10];
	char tbwstr[20];
	char mmerrstr[20];
	if(flag) strcpy(status, "read");
	else strcpy(status,"write");
	bytestostr(totalbyteswritten,tbwstr);
	bytestostr(mismatcherrors,mmerrstr);
	printf("\rPassage = %-9lli  %-3.1f%% %-6s  TBW = %-7s  I/O errors = %-18lli  mismatch errors = %-10s", passage, percent ,status, tbwstr, ioerrors , mmerrstr);
	fflush(stdout);
}

void fill(uint64_t bytes)
{ 
	int n32mfiles = bytes / (1024*1024*32);
	bytes %= (1024*1024*32);
	char filename[10];
	uint32_t buf[256];
	for(int i = 0; i < n32mfiles; i++) /*writing 32MiB files*/
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
	/*writing remaining data*/
	sprintf(filename, "%i.dat",n32mfiles);
	FILE* destfile = fopen(filename, "wb");
	for(int i = 0; i < (bytes / 1024);i++)
	{
		for(int j = 0; j < 256; j++)
		{
			buf[j] = xorshift128();
		}
		totalbyteswritten += 4*fwrite(buf, 4, 256, destfile);
	}
	fclose(destfile);
	progress(0, 1.0);
}

void cmpbuf(uint32_t * buf1, uint32_t * buf2)
{
	for(int i = 0; i < 256;i++)
	{
		if(buf1[i] != buf2[i]) mismatcherrors += 4;
	}
}

void readback(uint64_t bytes)
{ 
	int n32mfiles = bytes / (1024*1024*32);
	bytes %= (1024*1024*32);
	char filename[10];
	uint32_t readbuf[256];
	uint32_t genbuf[256];
	for(int i = 0; i < n32mfiles; i++) /*writing 32MiB files*/
	{
		sprintf(filename, "%i.dat",i);
		FILE* destfile = fopen(filename, "rb");
		for(int j = 0; j < 32*1024;j++)
		{
			for(int k = 0; k < 256; k++)
			{
				genbuf[k] = xorshift128();
			}
			ioerrors += (4*fread(readbuf, 4, 256, destfile) - 1024);
			cmpbuf(readbuf, genbuf);
		}
		fclose(destfile);
		progress(1, (double)i/n32mfiles);
	}
	/*writing remaining data*/
	sprintf(filename, "%i.dat",n32mfiles);
	FILE* destfile = fopen(filename, "rb");
	for(int i = 0; i < (bytes / 1024);i++)
	{
		for(int j = 0; j < 256; j++)
		{
			genbuf[j] = xorshift128();
		}
		ioerrors += (4*fread(readbuf, 4, 256, destfile) - 1024);
		cmpbuf(readbuf, genbuf);
	}
	progress(1, 1.0);
	fclose(destfile);
	passage++;
}

//void cycle(uint64_t bytes)
//{
	//readprogress();
	//reseed();
	//fill();
	//readback();
	//writeprogress();	
//}

int main(void)
{	
	char dig[20];
	reseed(1);
	fill(tobytes("3000M"));
	reseed(1);
	readback(tobytes("3000M"));
	//printf("elapsed %.3f\n", (time(0) - start)/1.0);
	//printf("%u \n", state0);	printf("%u \n", state1);	printf("%u \n", state2);	printf("%u \n", state3);
	return 0;
}
