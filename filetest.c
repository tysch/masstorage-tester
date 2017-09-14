/*
 * filetest.c
 *
 */




/*



// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count
uint32_t chkbuf_dev(char * buf, uint32_t bufsize,  struct fecblock * fecblocks, uint64_t *fpos, int nblocksizes, int isfectesting)
{
    uint32_t * ptr;
    uint32_t nerr = 0;
    uint32_t err_rs_block = 0;
    uint32_t posmod = 0;

    for(uint32_t i = 0; i <  bufsize; i += sizeof(uint32_t))
    {
        ptr = (uint32_t *)(buf + i);

        if((*ptr) != xorshift128())
        {
            nerr += sizeof(uint32_t);
            err_rs_block = 1;
        }

        // Reed-Solomon damaged blocks counting
        if(isfectesting)
        {
            posmod += sizeof(uint32_t);
            if(posmod == MIN_RS_BLOCKSIZE)
            {
                posmod = 0;

                if((err_rs_block != 0))
                    fecsize_test(fecblocks, 1, fpos, nblocksizes);
                else
                    fecsize_test(fecblocks, 0, fpos, nblocksizes);

                err_rs_block = 0;
            }
        }
    }
    return nerr;
}




void writefile(uint64_t nfile, char * path, char * buf,  FILE * logfile , int islogging, uint32_t bufsize)
{
    char filename[512];
    int fd;
    int32_t ret;

    sprintf(filename, "%s/%lli.dat", path, nfile);
    int fd = open(filename, O_WRONLY | O_SYNC | O_TRUNC | O_CREAT);

    if(fd == -1)
    {
        printf("\nfile %lli.dat access error!\n", nfile);
        if(islogging) fprintf(logfile, "\nfile %lli.dat access error!\n", nfile);
    }
    else
    {
        ret = write (fd, buf, bufsize);

        if ((ret == -1))
        {
            printf("\nfile %lli.dat write failure\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat write failure", nfile);
        }

        if ((ret < bufsize))
        {
            printf("\nfile %lli.dat write incomplete\n", nfile);
            if(islogging) fprintf(logfile, "\nfile write incomplete", nfile);
        }

        if(close(fd) == -1)
        {
            printf("\file %lli.dat is not closed properly!\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat is not closed properly!", nfile);
        }
    }
}



// Sorting routines for uint32[];
int cmpuint32_t (const void * elem1, const void * elem2)
{
    int f = *((uint32_t*)elem1);
    int s = *((uint32_t*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

void qsortuint32_t(uint32_t * arr, int size)
{
	qsort(arr, size, sizeof(*arr), cmpuint32_t);
}

// Returns most frequent result or 0 if there is more than one most occurent rsult

uint32_t major_occurence (uint32_t * data, int n)
{
	uint32_t curres = 0;
	uint32_t currlength = 0;
	uint32_t mstfreqres = 0;
	uint32_t maxlength = 0;

	// flags that there is more than one most frequent value
	int eqflag = 0;

	// sort all results
	qsortuint32_t(data, n);
	// finds longest
	for(int i = 0; i < n; i++)
	{
		if(data[i] != curres)
		{
			curres = data[i];
			currlength = 0;
		}
		if(currlength == maxlength)
		{
			eqflag = 1;
		}
		if(currlength > maxlength)
		{
			eqflag = 0;
			maxlength = currlength;
			mstfreqres = curres;
		}
	}
	if(eqflag) return 0;
	else return mstfreqres;
}






// Attempts to recover file size from MAX_CHECKED_FILES,
// Error-tolerant
uint32_t bufsize_retrieve(char * path, FILE * logfile, int islogging)
{
    // TODO: put path string size constant out
	//linux/limits.h.
	//#define PATH_MAX        4096    // # chars in a path name including nul
	char filename[512];
    uint32_t sizelist[MAX_CHECKED_FILES];
    int fd;
    int32_t ret;
    uint32_t bufsize;
    uint32_t nfile = 0;

	for(; nfile < MAX_CHECKED_FILES; nfile++)
	{
		sprintf(filename, "%s/%lli.dat", path, nfile);
		fd = open(filename, O_RDONLY);

		if(fd == -1)
		{
			printf("\nfile %i/%i opening error\n", nfile, MAX_CHECKED_FILES);
			printf(logfile, "\nfile %i/%i opening error\n", nfile, MAX_CHECKED_FILES);
			printopenerr(logfile, islogging);
			continue;
		}

		ret = lseek(fd, 0, SEEK_END);

		if(ret == -1)
		{
			printf("\nfile %i/%i size measurement error\n", nfile, MAX_CHECKED_FILES);
			printf(logfile, "\nfile %i/%i size measurement error\n", nfile, MAX_CHECKED_FILES);
			printlseekerr(logfile, islogging);
			continue;
		}

		sizelist[nfile] = ret;

		ret = close(fd);
        if(close(fd) == -1)
        {
            printf("\file %lli.dat is not closed properly!\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat is not closed properly!", nfile);
        }
	}

	bufsize = major_occurence (sizelist, MAX_CHECKED_FILES);

    if((bufsize == 0) &&    // no explicit maximum in file sizes
       (bufsize % 16 != 0)) // file size is not aligned properly
    {
    	printf("\nError: size and seed information file is unretrievable\n");
		if(islogging) fprintf(logfile, "\nError: size and seed information file is unretrievable\n");
    	exit(1);
    }
    else return bufsize;
}


void readbackfile(uint64_t nfile, char * path, char * buf,  FILE * logfile , int islogging, uint32_t bufsize)
{
    char filename[512];
    int fd;
    int32_t ret;
    uint32_t seed;
    uint32_t totsize;

    sprintf(filename, "%s/%lli.dat", path, nfile);
    int fd = open(filename, O_RDONLY);

    if(fd == -1)
    {
        printf("\nfile %lli.dat access error!\n", nfile);
        if(islogging) fprintf(logfile, "\nfile %lli.dat access error!\n", nfile);
    }
    else
    {
    	bufsize = lseek(fd, 0, SEEK_END);
    	if((bufsize % 16 != 0) || (bufsize < 16))
    	{
    		//printf("Error: size and seed information file is ")
		    //if(islogging) fprintf(logfile, "\nfile %lli.dat access error!\n", nfile);
    	}



    	ret = lseek (fd, (off_t) 1825, SEEK_SET);
    	if(ret == (off_t) -1)
    	{

    	}

    	ret = fileread(fd, buf, bufsize, logfile, islogging);
    	readseedandsize (buf, bufsize, &seed, &totsize);



        if(close(fd) == -1)
        {
            printf("\file %lli.dat is not closed properly!\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat is not closed properly!", nfile);
        }
    }
}



void fillfiles(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging , uint64_t totsize, uint32_t bufsize)
{
	uint64_t nfiles = totsize / bufsize;
	writeseedandsize(buf, seed, totsize, bufsize);
	writefile(0, path, buf, logfile , islogging, bufsize);
	for(uint64_t i = 1; i < nfiles; i++)
	{
		fillbuf(buf, bufsize);
		writefile(i, path, buf, logfile , islogging, bufsize);
	}
}
*/
