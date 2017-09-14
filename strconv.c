/*
 * strconv.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Converts bytes count to human readable string
void bytestostr(uint64_t bytes, char * str)
{
    if(bytes < 1024)
        sprintf(str, "%i B", (int) bytes );

    if((bytes >= 1024) && (bytes < 1024*1024))
        sprintf(str, "%.3f KiB", bytes/1024.0);

    if((bytes >= 1024*1024) && (bytes < 1024*1024*1024))
        sprintf(str, "%.3f MiB", bytes/(1024.0*1024));

    if((bytes >= 1024*1024*1024) && (bytes < 1024LL*1024LL*1024LL*1024LL))
        sprintf(str, "%.3f GiB", bytes/(1024.0*1024.0*1024.0));

    if((bytes >= 1024LL*1024LL*1024LL*1024LL))
        sprintf(str, "%.3f TiB", bytes/(1024.0*1024.0*1024.0*1024.0));
}

// converts seconds interval to date
void todate(uint64_t s, char * date)
{
	int sec = s % 60;
	s /= 60;
	int min = s % 60;
	s /= 60;
	int hours = s % 24;
	s /= 24;
	int days = s % 365;
	s /= 365;
	int years = s;
	if(years)
	{
	    sprintf(date, "%i yr %i days %02i:%02i:%02i", years, days, hours, min, sec);
	}
	else
	{
	    if(days)
	    {
	        sprintf(date, "%i days %02i:%02i:%02i", days, hours, min, sec);
	    }
	    else
	    {
	    	if(hours)
	        {
	            sprintf(date, "%02i:%02i:%02i", hours, min, sec);
	        }
	        else
	        {
	            if(min)
	            {
	                sprintf(date, "%02i:%02i", min, sec);
	            }
	            else sprintf(date, "%is", sec);
	        }
	    }
	}
}

// Converts human-readable size string to bytes
uint64_t tobytes(char * x)
{
    uint64_t bytes = 0;
    uint64_t multiplier = 1;
    for(int i = 0; i < strlen(x); i++)
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
                case 'T':
                    multiplier = 1024LL*1024LL*1024LL*1024LL;
                    break;
                case 't':
                    multiplier = 1024LL*1024LL*1024LL*1024LL;
                    break;
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
