/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include <time.h>
#ifdef _WIN32
# include <windows.h>
# include <mmsystem.h>
#else
# include <sys/time.h>
#endif

#include "clock.h"
#include "moremem.h"
#include "sim65816.h"

s_clock g_clock;

extern int Verbose;
//extern word32 g_vbl_count;	// OG change int to word32
//extern int g_rom_version;
extern int g_config_kegs_update_needed;


double
get_dtime()
{
#ifndef _WIN32
	struct timeval tp1;
	double	dsec;
	double	dusec;
#endif

	/* Routine used to return actual system time as a double */
	/* No routine cares about the absolute value, only deltas--maybe */
	/*  take advantage of that in future to increase usec accuracy */
double	dtime;
#ifdef _WIN32
	LARGE_INTEGER counter; 
	LARGE_INTEGER frequency; 
	QueryPerformanceCounter(&counter);
	QueryPerformanceFrequency(&frequency);
	dtime = (double)counter.QuadPart/(double)frequency.QuadPart;
//	dtime = timeGetTime() / 1000.0;
#else
	

# ifdef SOLARIS
	gettimeofday(&tp1, (void *)0);
# else
	gettimeofday(&tp1, (struct timezone *)0);
# endif

	dsec = (double)tp1.tv_sec;
	dusec = (double)tp1.tv_usec;

	dtime = dsec + (dusec / (1000.0 * 1000.0));
#endif

	return dtime;
}

int
micro_sleep(double dtime)
{
#ifndef _WIN32
	struct timeval Timer;
	int	ret;
#endif

	if(dtime <= 0.0) {
		return 0;
	}
	if(dtime >= 1.0) {
		halt_printf("micro_sleep called with %f!!\n", dtime);
		return -1;
	}

#if 0
	printf("usleep: %f\n", dtime);
#endif

#ifdef _WIN32
	Sleep((DWORD)(dtime * 1000));
#else
	Timer.tv_sec = 0;
	Timer.tv_usec = (dtime * 1000000.0);
	if( (ret = select(0, 0, 0, 0, &Timer)) < 0) {
		fprintf(stderr, "micro_sleep (select) ret: %d, errno: %d\n",
			ret, errno);
		return -1;
	}
#endif
	return 0;
}

void
clk_bram_zero()
{
	int	i, j;

	/* zero out all bram */
	for(i = 0; i < 2; i++) {
		for(j = 0; j < 256; j++) {
			g_clock.g_bram[i][j] = 0;
		}
	}
	g_clock.g_bram_ptr = &(g_clock.g_bram[0][0]);
}

void
clk_bram_set(int bram_num, int offset, int val)
{
	g_clock.g_bram[bram_num][offset] = val;
}


extern void x_clk_setup_bram_version();

void
clk_setup_bram_version()
{
	x_clk_setup_bram_version();
}

void
clk_write_bram(FILE *fconf)
{
	int	i, j, k;

	for(i = 0; i < 2; i++) {
		fprintf(fconf, "\n");
		for(j = 0; j < 256; j += 16) {
			fprintf(fconf, "bram%d[%02x] =", 2*i + 1, j);
			for(k = 0; k < 16; k++) {
				fprintf(fconf, " %02x", g_clock.g_bram[i][j+k]);
			}
			fprintf(fconf, "\n");
		}
	}
}

void
update_cur_time()
{
	
	time_t	cur_time;
	unsigned int secs, secs2;


#ifdef UNDER_CE	// OG Not supported on WIndows CE
	/*
	SYSTEMTIME stime;
	FILETIME ftime;
	GetLocalTime(&stime);
	SystemTimeToFileTime(&stime,&ftime);
	cur_time = ftime.dwLowDateTime;
	*/
	cur_time = time(0);

	secs=0;
	secs2=0;

#else
	struct tm *tm_ptr;
	cur_time = time(0);

	/* Figure out the timezone (effectively) by diffing two times. */
	/* this is probably not right for a few hours around daylight savings*/
	/*  time transition */
	secs2 = (unsigned int)mktime(gmtime(&cur_time));
	tm_ptr = localtime(&cur_time);
	secs = (unsigned int)mktime(tm_ptr);

#ifdef DRIVER_OSX
	/* Mac OS X's mktime function modifies the tm_ptr passed in for */
	/*  the CDT timezone and breaks this algorithm.  So on a Mac, we */
	/*  will use the tm_ptr->gmtoff member to correct the time */
	secs = secs + tm_ptr->tm_gmtoff;
#else
	secs = (unsigned int)cur_time - (secs2 - secs);

	if(tm_ptr->tm_isdst) {
		/* adjust for daylight savings time */
		secs += 3600;
	}
#endif
#endif
	/* add in secs to make date based on Apple Jan 1, 1904 instead of */
	/*   Unix's Jan 1, 1970 */
	/*  So add in 66 years and 17 leap year days (1904 is a leap year) */
	secs += ((66*365) + 17) * (24*3600);

	g_clock.g_clk_cur_time = secs;

	clk_printf("Update g_clock.g_clk_cur_time to %08x\n", g_clock.g_clk_cur_time);
	g_clock.g_clk_next_vbl_update = g_sim65816.g_vbl_count + 5;
}

/* clock_update called by sim65816 every VBL */
void
clock_update()
{
	/* Nothing to do */
}

void
clock_update_if_needed()
{
	int	diff;

	diff = g_clock.g_clk_next_vbl_update - g_sim65816.g_vbl_count;
	if(diff < 0 || diff > 60) {
		/* Been a while, re-read the clock */
		update_cur_time();
	}
}

void
clock_write_c034(word32 val)
{
	g_moremem.g_c034_val = val & 0x7f;
	if((val & 0x80) != 0) {
		if((val & 0x20) == 0) {
			printf("c034 write not last = 1\n");
			/* set_halt(1); */
		}
		do_clock_data();
	}
}


void
do_clock_data()
{
	word32	mask;
	int	read;
	int	op;

	clk_printf("In do_clock_data, g_clock.g_clk_mode: %02x\n", g_clock.g_clk_mode);

	read = g_moremem.g_c034_val & 0x40;
	switch(g_clock.g_clk_mode) {
	case CLK_IDLE:
		g_clock.g_clk_read = (g_moremem.g_c033_data >> 7) & 1;
		g_clock.g_clk_reg1 = (g_moremem.g_c033_data >> 2) & 3;
		op = (g_moremem.g_c033_data >> 4) & 7;
		if(!read) {
			/* write */
			switch(op) {
			case 0x0:	/* Read/write seconds register */
				g_clock.g_clk_mode = CLK_TIME;
				clock_update_if_needed();
				break;
			case 0x3:	/* internal registers */
				g_clock.g_clk_mode = CLK_INTERNAL;
				if(g_clock.g_clk_reg1 & 0x2) {
					/* extend BRAM read */
					g_clock.g_clk_mode = CLK_BRAM2;
					g_clock.g_clk_reg1 = (g_moremem.g_c033_data & 7) << 5;
				}
				break;
			case 0x2:	/* read/write ram 0x10-0x13 */
				g_clock.g_clk_mode = CLK_BRAM1;
				g_clock.g_clk_reg1 += 0x10;
				break;
			case 0x4:	/* read/write ram 0x00-0x0f */
			case 0x5: case 0x6: case 0x7:
				g_clock.g_clk_mode = CLK_BRAM1;
				g_clock.g_clk_reg1 = (g_moremem.g_c033_data >> 2) & 0xf;
				break;
			default:
				halt_printf("Bad c033_data in CLK_IDLE: %02x\n",
					g_moremem.g_c033_data);
			}
		} else {
			printf("clk read from IDLE mode!\n");
			/* set_halt(1); */
			g_clock.g_clk_mode = CLK_IDLE;
		}
		break;
	case CLK_BRAM2:
		if(!read) {
			/* get more bits of bram addr */
			if((g_moremem.g_c033_data & 0x83) == 0x00) {
				/* more address bits */
				g_clock.g_clk_reg1 |= ((g_moremem.g_c033_data >> 2) & 0x1f);
				g_clock.g_clk_mode = CLK_BRAM1;
			} else {
				halt_printf("CLK_BRAM2: c033_data: %02x!\n",
						g_moremem.g_c033_data);
				g_clock.g_clk_mode = CLK_IDLE;
			}
		} else {
			halt_printf("CLK_BRAM2: clock read!\n");
			g_clock.g_clk_mode = CLK_IDLE;
		}
		break;
	case CLK_BRAM1:
		/* access battery ram addr g_clock.g_clk_reg1 */
		if(read) {
			if(g_clock.g_clk_read) {
				/* Yup, read */
				g_moremem.g_c033_data = g_clock.g_bram_ptr[g_clock.g_clk_reg1];
				clk_printf("Reading BRAM loc %02x: %02x\n",
					g_clock.g_clk_reg1, g_moremem.g_c033_data);
			} else {
				halt_printf("CLK_BRAM1: said wr, now read\n");
			}
		} else {
			if(g_clock.g_clk_read) {
				halt_printf("CLK_BRAM1: said rd, now write\n");
			} else {
				/* Yup, write */
				clk_printf("Writing BRAM loc %02x with %02x\n",g_clock.g_clk_reg1, g_moremem.g_c033_data);
				g_clock.g_bram_ptr[g_clock.g_clk_reg1] = g_moremem.g_c033_data;
				g_config_kegs_update_needed = 1;
			}
		}
		g_clock.g_clk_mode = CLK_IDLE;
		break;
	case CLK_TIME:
		if(read) {
			if(g_clock.g_clk_read == 0) {
				halt_printf("Reading time, but in set mode!\n");
			}
			g_moremem.g_c033_data = (g_clock.g_clk_cur_time >> (g_clock.g_clk_reg1 * 8)) &
									0xff;
			clk_printf("Returning time byte %d: %02x\n",
				g_clock.g_clk_reg1, g_moremem.g_c033_data);
		} else {
			/* Write */
			if(g_clock.g_clk_read) {
				halt_printf("Write time, but in read mode!\n");
			}
			clk_printf("Writing TIME loc %d with %02x\n",
				g_clock.g_clk_reg1, g_moremem.g_c033_data);
			mask = 0xff << (8 * g_clock.g_clk_reg1);

			g_clock.g_clk_cur_time = (g_clock.g_clk_cur_time & (~mask)) |
				((g_moremem.g_c033_data & 0xff) << (8 * g_clock.g_clk_reg1));
		}
		g_clock.g_clk_mode = CLK_IDLE;
		break;
	case CLK_INTERNAL:
		if(read) {
			printf("Attempting to read internal reg %02x!\n",
				g_clock.g_clk_reg1);
		} else {
			switch(g_clock.g_clk_reg1) {
			case 0x0:	/* test register */
				if(g_moremem.g_c033_data & 0xc0) {
					printf("Writing test reg: %02x!\n",
						g_moremem.g_c033_data);
					/* set_halt(1); */
				}
				break;
			case 0x1:	/* write protect reg */
				clk_printf("Writing clk wr_protect: %02x\n",
					g_moremem.g_c033_data);
				if(g_moremem.g_c033_data & 0x80) {
					printf("Stop, wr clk wr_prot: %02x\n",
						g_moremem.g_c033_data);
					/* set_halt(1); */
				}
				break;
			default:
				halt_printf("Writing int reg: %02x with %02x\n",
					g_clock.g_clk_reg1, g_moremem.g_c033_data);
			}
		}
		g_clock.g_clk_mode = CLK_IDLE;
		break;
	default:
		halt_printf("clk mode: %d unknown!\n", g_clock.g_clk_mode);
		g_clock.g_clk_mode = CLK_IDLE;
		break;
	}
}

