/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "savestate.h"
#include "compression.h"
#include "../../Libraries/zlib123/zlib.h"

int rleencode(serialize* _data);

s_compression g_compression;


int serialize::expand()
{
	if (!rledata)
	{
	//	printf("cannot expand : no compressed data\n");
		return 0;
	}

	if (data)
	{
	//	printf("alreay expanded!");
		return 0;
	}

	Bytef* p = (Bytef*)x_malloc(size,fastalloc);
	uLongf dsize = size;
	::uncompress(p,&dsize,(const Bytef*)rledata,rlesize);

	// TODO ERROR CHECKING
	data = p;
	
	return 1;
}


int serialize::releaseuncompressed()
{
	if (!rledata)
	{
	//	printf("cannot release uncompressed : no compressed data!\n");
		return 0;
	}
	x_free(data,size,fastalloc);
	data = NULL;
	return 1;
}

int serialize::compress()
{
	if (rledata)
	{
	//	printf("already compressed\n");
		return 0;
	}

	byte* tmp  = (byte*)x_malloc(size,fastalloc);
//	rlesize = rleencode(this);

//	double t = get_dtime();
	uLongf destsize = size;
	::compress2 (tmp,&destsize,(const Bytef*)data,size,Z_BEST_SPEED);
	rlesize = destsize;
//	double delay = get_dtime() - t;
//	printf("compressed %d%% in %fs\n",(int)( (float)destsize*100/(float)size ),delay/1000);

	rledata = x_realloc(tmp,rlesize,size);

	releaseuncompressed();
	return 1;
}

int s_compression::add_job(serialize* _s)
{
	for(int i=0;i<MAX_JOB;i++)
	{
		if (!jobs[i])
		{
			jobs[i] = _s;
			return 1;
		}
	}
	printf("too many jobs : aborted!");
	return 0;
}

int s_compression::remove_job(serialize* _s)
{
	for(int i=0;i<MAX_JOB;i++)
	{
		if (jobs[i]==_s)
		{
			jobs[i] = NULL;
	//		printf("job removed!\n");
			return 1;
		}
	}
//	printf("job not found !");
	return 0;
}


int s_compression::process_jobs(int _nbjob)
{
	for(int i=0;i<_nbjob;i++)
	{
		for(int j=0;j<MAX_JOB;j++)
		{
			if (jobs[j])
			{
				serialize* s = jobs[j]; 
				jobs[j] = NULL;
				s->compress();
			}
		}
	}
	return 0;
}

s_compression::~s_compression()
{
	for(int j=0;j<MAX_JOB;j++)
	{
		if (jobs[j])
		{
			printf("some jobs remaining...\n");
			return ;
		}
	}
}
	

int rleencode(char* out,serialize* _data)
{

#define MIN_RUN     3                   /* minimum run length to encode */
#define MAX_RUN     (128 + MIN_RUN - 1) /* maximum run length to encode */
#define MAX_COPY    128                 /* maximum characters to copy */

/* maximum that can be read before copy block is written */
#define MAX_READ    (MAX_COPY + MIN_RUN - 1)


	char charBuf[256];
    int count = 0;

	int size = _data->size;
	char* ptr = (char*)_data->data;
	char currChar = *ptr++;
	size--;

	int compressedsize = 0;

    /* read input until there's nothing left */
    while (size)
    {
        charBuf[count]=currChar;
        count++;

        if (count >= MIN_RUN)
        {
            int i;

            /* check for run  charBuf[count - 1] .. charBuf[count - MIN_RUN]*/
            for (i = 2; i <= MIN_RUN; i++)
            {
                if (currChar != charBuf[count - i])
                {
                    /* no run */
                    i = 0;
                    break;
                }
            }

            if (i != 0)
            {
                /* we have a run write out buffer before run*/
                int nextChar;

                if (count > MIN_RUN)
                {
                    /* block size - 1 followed by contents */
					*out++ = count - MIN_RUN - 1;
					compressedsize++;
			
					memcpy(out,charBuf,count - MIN_RUN);
					out+= count - MIN_RUN;
					compressedsize += count - MIN_RUN;
					
                   }


                /* determine run length (MIN_RUN so far) */
                count = MIN_RUN;

                while (size)
				{
					nextChar = *ptr++;
					size--;
					if (nextChar == currChar)
					{
						count++;
						if (MAX_RUN == count)
						{
							/* run is at max length */
							break;
						}
					}
					else
						break;
                }

				*out++ = (int)(MIN_RUN - 1) - (int)(count);
				compressedsize++;

				*out++ = currChar;
				compressedsize++;

                if ((size) && (count != MAX_RUN))
                {
                    /* make run breaker start of next buffer */
                    charBuf[0] = nextChar;
                    count = 1;
                }
                else
                {
                    /* file or max run ends in a run */
                    count = 0;
                }
            }
        }

        if (MAX_READ == count)
        {
            int i;

       		*out++ = MAX_COPY - 1;
			compressedsize++;

			memcpy(out,charBuf,MAX_COPY);
			compressedsize+=MAX_COPY;
			out+=MAX_COPY;

			/* start a new buffer */
            count = MAX_READ - MAX_COPY;

            /* copy excess to front of buffer */
            for (i = 0; i < count; i++)
            {
                charBuf[i] = charBuf[MAX_COPY + i];
            }
        }

		if (size)
		{
			currChar = *ptr++;
			size--;
		}
    }

    /* write out last buffer */
    if (0 != count)
    {
        if (count <= MAX_COPY)
        {
            /* write out entire copy buffer */
			*out++= count -1;
			compressedsize++;

			memcpy(out,charBuf,count);
			out += count;
			compressedsize+=count;
        }
        else
        {
            /* we read more than the maximum for a single copy buffer */

			*out++ = MAX_COPY - 1;
			compressedsize++;
			
			memcpy(out,charBuf,MAX_COPY);
			out+=MAX_COPY;
			compressedsize+=MAX_COPY;
	
            /* write out remainder */
            count -= MAX_COPY;

			*out++ = count -1;
			compressedsize++;
            
			memcpy(out,&charBuf[MAX_COPY],count);
			out+=count;
			compressedsize+=count;
		}
    }

	printf("compressed from %d to %d\n",_data->size,compressedsize);
	return compressedsize;
}

