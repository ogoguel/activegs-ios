/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#include "defc.h"
#include "StdString.h"

enum 
{
	STATE_NONE=0,
	STATE_SAVE,
	STATE_RESTORE,
	STATE_REWIND,

};
#define SAVEDSTATE_SIG 0x02007ADE
#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

enum savedStateFlags
{
	SAVESTATEFLAG_VIDEOSINGLEVLINE=1<<1,
	SAVESTATEFLAG_VIDEOSINGLEHLINE=1<<2
};

struct savedStateParam
{
	int size;
	int		version;
	double dcycs;
	int	   vbl;
	int		memory_size;
	int		rom;
	int		flags;
	savedStateParam() {
		size = sizeof(savedStateParam);
        version = SAVEDSTATE_SIG;
        rom = 0;
        dcycs=0.0;
		flags = 0;
#ifdef VIDEO_SINGLEVLINE
		flags |= SAVESTATEFLAG_VIDEOSINGLEVLINE;
#endif
#ifdef VIDEO_SINGLEHLINE
		flags |= SAVESTATEFLAG_VIDEOSINGLEHLINE;;
#endif

        memory_size = -1;
    };
} XCODE_PRAGMA_PACK;


#ifdef _WIN32
#pragma pack(pop)
#endif


class savedState
{
public:
	static  int maxSize;
	serialize adb;
	serialize async_event;
	serialize clock;
	serialize iwm;
	serialize moremem;
	serialize paddles;
	serialize scc;
	serialize sim65816;
	serialize sound;
	serialize video;
	serialize offscreen;

	savedStateParam param;
	int	   fullsave;
	int	   fastalloc;
		int	loadFromDiskInternal(const char* _filename);
public:
	savedState();
	~savedState()
	{
		release();
	}
	void	save(int _fullsave, int _usefastalloc);
	void	writeToDisk(const char* _filename);
	int	loadFromDisk(const char* _filename);
	void	restore();
	void	release();
	void	display();
	int		isEmpty() { return !param.vbl; };

};


#define NB_KEY_STATE	10
#define NB_SCREEN_PER_STATE	(2*60/VBL_DELAY_BETWEEN_SCREEN)
#define VBL_DELAY_BETWEEN_SCREEN	2

#define MAX_STATE	(NB_KEY_STATE*NB_SCREEN_PER_STATE)

#if !defined(DRIVER_IOS) && !defined(DRIVER_ANDROID)
#define CACHE_SIZE (256*1024*1024)
#endif

class s_savestate
{
	
	savedState states[MAX_STATE];
	int iNextState;
	int iCurState;
	word32 nextStateVBL;
	word32 nextScreenVBL;
	int	targetStateSens;
	double	lastrefreshcycs;

	byte*	cache;
	int		cachepos;
	int		cachefree;
	int cacheSize;

public:
    
	savedState state;
    MyString filename;
    int stateActionRequired;
	
    
			s_savestate();
			~s_savestate();
	void	reset_rewind();
	void	reset_state();
	void	delete_state(int);
	void	init();
	void	shut();
	void	saveState(const char* _filename);
	void	restoreState(const char* _filename);
//	void	saveStateToDisk(const char* _filename);
//	void	restoreStateFromDisk(const char* _filename);
	int		getSavedStateVBL();
	void	handleState();
	void	handleKey(int _key,int _isup);

	void*	x_free(void*,int,int);
	void*	x_malloc(int,int);
	int		get_free_memory_size();
	
};



extern s_savestate g_savestate;
