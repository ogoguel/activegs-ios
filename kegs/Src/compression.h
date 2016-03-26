/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#define MAX_JOB 32

void* x_realloc(void* ptr,int _newsize, int _formersize);
extern void* x_malloc(int size, int fastalloc);
extern void* x_free(void* ptr,int size, int fastalloc);

extern	int cacheSize;

class serialize;

class s_compression
{

	serialize* jobs[MAX_JOB];

public:
	s_compression()
	{
		memset(this,0,sizeof(*this));
	}
	~s_compression();
	
	int	add_job(serialize* _job);
	int	remove_job(serialize* _job);
	int	process_jobs(int _nbjob);
};

extern s_compression g_compression;