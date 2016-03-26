/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef _WIN32
#	define _WINSOCKAPI_  
#endif
#include "StdString.h"
#include "defc.h"
#include "savestate.h"
#include "adb.h"
#include "async_event.h"
#include "clock.h"
#include "iwm.h"
#include "moremem.h"
#include "paddles.h"
#include "scc.h"
#include "sim65816.h"
#include "sound.h"
#include "video.h"
#include "compression.h"

#undef debug_printf
#define debug_printf(X,...) 


s_savestate g_savestate;

void x_sleep(int _ms)
{
	#ifdef WIN32
			Sleep(_ms);
	#else
			usleep(_ms);
	#endif
}

int savedState::maxSize=0;

savedState::savedState()
{
	param.vbl=0;
 
}

void savedState::release()
{
	if (isEmpty()) return ;

	adb.release();
	async_event.release();
	clock.release();
	iwm.release();
	moremem.release();
	paddles.release();
	scc.release();
	sim65816.release();
	sound.release();
	video.release();
	offscreen.release();
	param.vbl = 0;
}

void serialize::release()
{
	g_compression.remove_job(this);

	if (rledata)
	{
		x_free(rledata,rlesize,fastalloc);
		rledata = NULL;
	}

	if (data)
	{
		x_free(data,size,fastalloc);
		data=NULL;
	}
}

void savedState::save(int _fullsave, int _usefastalloc)
{
	#if (!defined(MAC) || defined(ACTIVEGSPLUGIN) ) && !defined(ACTIVEGS_NOSAVESTATE)
	
	release();

	fastalloc = _usefastalloc;

	int size = 0;

	param.dcycs = g_sim65816.g_last_vbl_dcycs;
	param.vbl = g_sim65816.g_vbl_count;
	param.memory_size = g_sim65816.g_mem_size_total ;
	param.rom = 0 ;
	
	debug_printf("capturing (%d) vbl:%d  dcycs:%f last_vbl_dcys:%f\n",_fullsave,vbl,dcycs,g_sim65816.g_last_vbl_dcycs);

	fullsave  = _fullsave;
	if (_fullsave)
	{
		size += g_adb.out(&adb,fastalloc);
		size += g_async_event.out(&async_event,fastalloc);
		size += g_clock.out(&clock,fastalloc);
		size += g_iwm.out(&iwm,fastalloc);
		size += g_moremem.out(&moremem,fastalloc);
		size += g_paddles.out(&paddles,fastalloc);
		size += g_scc.out(&scc,fastalloc);
		size += g_sim65816.out(&sim65816,fastalloc);
		size += g_sound.out(&sound,fastalloc);
		size += g_video.out(&video,fastalloc);
	}

#ifdef VIDEO_SINGLEVLINE
#define STEP 1
#else
#define STEP 2
#endif
	
	// sauvegarde l'offscreen (1 ligne sur 2)
	int wb = g_kimage_offscreen.width_act * ( g_kimage_offscreen.mdepth >> 3 );
	offscreen.size = wb  *  g_kimage_offscreen.height / STEP ;
	offscreen.fastalloc = fastalloc;
	offscreen.data = (byte*)x_malloc( offscreen.size,fastalloc );
	if (!offscreen.data)
		x_fatal_exit("out of memory error!");
	byte* ptrs  = (byte*)g_kimage_offscreen.data_ptr;
	byte* ptrd  = (byte*)offscreen.data;


	for(int h=0;h<g_kimage_offscreen.height;h+=STEP)
	{
		memcpy(ptrd,ptrs,wb);
		ptrd+=wb;
		ptrs+=wb*STEP;
	}
	size += offscreen.size; 

	if (maxSize<size)
		maxSize=size;

//	g_compression.add_job(&offscreen);

#endif

}

extern void refresh_offscreen();

void savedState::display()
{

	if (isEmpty()) return ;
	if (offscreen.data==NULL)
	{
		refresh_offscreen();
		refresh_video(1);
		return ;
	}


//	static double lastcycs=0.0;
	extern void drawText(Kimage* _dst, const char* text,int _x,int _y);

	#if !defined(MAC) || defined(ACTIVEGSPLUGIN)

	offscreen.expand();

	// génére une kimage temporaire
	Kimage temp;
	init_kimage(&temp,0,s_video.g_screen_depth, s_video.g_screen_mdepth);
    temp.width_req = X_A2_WINDOW_WIDTH;
    temp.width_act = X_A2_WINDOW_WIDTH;
    temp.height = X_A2_WINDOW_HEIGHT;

	int	pb = temp.mdepth >> 3;

	temp.data_ptr = (byte*)malloc(X_A2_WINDOW_WIDTH*X_A2_WINDOW_HEIGHT*pb);

#ifdef VIDEO_SINGLEVLINE
    // restore l'offscreen ˆ l'identique
	int wb = temp.width_act * pb;
	byte* ptrs  = (byte*)offscreen.data;
	byte* ptrd  = (byte*)temp.data_ptr;
    
	for(int h=0;h<g_kimage_offscreen.height;h++)
	{
		memcpy(ptrd,ptrs,wb);
		ptrd+=wb;
		ptrs+=wb;
	}
#else
	// restore l'offscreen (1 ligne sur 2 dupliquée)
	int wb = temp.width_act * pb;
	byte* ptrs  = (byte*)offscreen.data;
	byte* ptrd  = (byte*)temp.data_ptr;

	for(int h=0;h<g_kimage_offscreen.height;h+=2)
	{
		memcpy(ptrd,ptrs,wb);
		ptrd+=wb;
		memcpy(ptrd,ptrs,wb);
		ptrd+=wb;
		ptrs+=wb;
	}
#endif
	// push l'image => pour profiter des effets post prod
	x_push_kimage(&temp,0,0,0,0,temp.width_req,temp.height);

	offscreen.releaseuncompressed();

	// libère le fichier temp
	free(temp.data_ptr );

	refresh_video(1);
//	double dtime = get_dtime();
//	double delay = dtime - lastcycs;
//	lastcycs = dtime;

//	debug_printf("displaying frame:%d (%f)\n",vbl,delay);
#endif

}

void s_savestate::handleKey(int _key,int _isup)
{
	#if (!defined(MAC) || defined(ACTIVEGSPLUGIN)) && !defined(ACTIVEGS_NOSAVESTATE)
	if (!_isup && (_key == 0x3B || _key == 0x3C || _key == 0x24 /*RET*/|| _key == 0x60 /*VK_F5*/ ) )
	{
		if (_key == 0x24 || _key == 0x60)
		{
			
			r_sim65816.set_state(RUNNING);
			return ;
		}


		int next ;
		if (iCurState==-1)
		{
			// TODO A AMELIORER
			iCurState = (MAX_STATE+iNextState -1 )%MAX_STATE;
		}

		if (_key==0x3B)
		{
			targetStateSens = 0;
			
			// vérifie qu'il y a encore une clef

			int next  = (MAX_STATE+iCurState -1 )%MAX_STATE;
			while(next != iNextState )
			{
				if (states[next].isEmpty()) 
					// plus de clef
					break; 
				if (states[next].fullsave)
				{
					targetStateSens = -1;
					break;
				}
				next  = (MAX_STATE+next -1 )%MAX_STATE;
	
			}
			if (!targetStateSens)

				printf("begin rewind\n");
	
		}
		else
		{
			targetStateSens = +1;
			next  = (MAX_STATE+iCurState +1 )%MAX_STATE;

			if (next == iNextState || states[next].isEmpty() )
			{
				targetStateSens = 0;
				iCurState=-1;
				printf("end rewind\n");
				return ;
			}
		}

	}
#endif
}


int s_savestate::getSavedStateVBL()
{
	return state.param.vbl;

}


void s_savestate::saveState(const char* _filename)
{
	if (r_sim65816.get_state() ==IN_PAUSE)
	{
		// en pause, sauve immédiatement (sauf si on est en mode rewind

		if (iCurState!=-1)
		{
			printf("cannot save state if rewind has started");
			return ;
		}
		if (_filename==NULL)
		{
			// save to memory
			state.save(1,0);
			debug_printf("save state @%d \n",state->vbl);
		}
		else
		{
			savedState temp;
			temp.save(1,0);
			temp.writeToDisk(_filename);
		}
		
		return ;
	}
	

    stateActionRequired=STATE_SAVE;
    filename = _filename;
	
}


void s_savestate::restoreState(const char* _filename)
{
	

	if (r_sim65816.get_state() ==IN_PAUSE)
	{
		// en pause, on peut faire le restore immédiatement (si on est pas en mode rewind)

		if (iCurState!=-1)
		{
			printf("cannot restore state if rewind has started");
			return ;
		}
		if (_filename==NULL)
			{
				if (!state.isEmpty())
				{
					// on est revenu à une position quelconque : le rewind n'a plus de sens
					state.restore();
					reset_rewind();
				}
			}
			else
			{
				savedState temp;
				int ret = temp.loadFromDisk(_filename);
				if (ret)
				{
					temp.restore();
					reset_rewind();
				}
			}

		return ;
	}
	
    // prépare le save à la prochaine boucle
    stateActionRequired=STATE_RESTORE;
   	filename = _filename;
	
}


void s_savestate::delete_state(int pos)
{
	if (states[pos].isEmpty()) return ;

	states[pos].release();

	// détruit les état présents jusqu'à la prochaine clef
	int deleteState=(pos +1 )%MAX_STATE;	
	while(deleteState!=pos)
	{
		if (states[deleteState].isEmpty()) break;
		if (states[deleteState].fullsave)
			// on a trouvé la prochaine clef : on s'arrete
			break;
		// détruit les écran intermédiaires
		states[deleteState].release();
		debug_printf("deleting state:%d\n",deleteState);
		deleteState=(deleteState +1 )%MAX_STATE;	
	}
}


void s_savestate::handleState()
{

//	double d = get_dtime();
	#if (!defined(MAC) || defined(ACTIVEGSPLUGIN)) && !defined(ACTIVEGS_NOSAVESTATE)
	
	if (r_sim65816.should_emulator_terminate())
{
		printf("ignoring handleState : emulator is quitting...\n");
		return ;
	}
	
	iCurState = -1;

	int	b = r_sim65816.get_rewind_enable();

	int	savescreen = ( b && g_sim65816.g_vbl_count >= nextScreenVBL ) ;
	int s = r_sim65816.get_state();
	int fullsave = b && ( g_sim65816.g_vbl_count>=nextStateVBL || s==IN_PAUSE) ;  
	
	if (savescreen||fullsave)
	{
		nextScreenVBL = g_sim65816.g_vbl_count + VBL_DELAY_BETWEEN_SCREEN ;

		delete_state(iNextState);

		// s'assure que l'on a assez de mémoire libre
		int n = (iNextState +1 )%MAX_STATE;
		while( 1)
		{
			int f = get_free_memory_size();
			if ( f> (int)savedState::maxSize*1.20 )
				break;
			if (!states[n].isEmpty())
			{
				debug_printf("only %dKB remaining : trying to free space\n",f/1024);
				delete_state(n);
				debug_printf("now %dKB!\n",get_free_memory_size()/1204);
			}
			n = (n +1 )%MAX_STATE;
			if (n == iNextState)
			{
				x_fatal_exit("err: out of memory error");
			}
			
		}	

		// capture l'état courant	
		states[iNextState].save(fullsave,1);

		// positionne la prochaine place libre
		iNextState = (iNextState +1 )%MAX_STATE;	

		if (fullsave)
			nextStateVBL = g_sim65816.g_vbl_count+NB_SCREEN_PER_STATE*VBL_DELAY_BETWEEN_SCREEN;
	
	}
	else
		g_compression.process_jobs(1);
	
		
	switch( stateActionRequired)
	{
		case STATE_SAVE:
		//	if (filename.IsEmpty())
            if (filename[0]==0)
                    
			{
				state.save(1,0);
				debug_printf("save state @ \n",state->vbl);
			}
			else
			{
				savedState temp;
				temp.save(1,0);
				temp.writeToDisk(filename);
			}
			
			stateActionRequired = STATE_NONE;
			break;
			
		case STATE_RESTORE:
		//	if (filename.IsEmpty())
            if (filename[0]==0)
			{
				if (!state.isEmpty())
				{
					// on est revenu à une position quelconque : le rewind n'a plus de sens
					state.restore();
					reset_rewind();
				}
			}
			else
			{
				savedState temp;
				int ret = temp.loadFromDisk(filename);
				if (ret)
				{
					temp.restore();
					reset_rewind();
				}
			}
	
			
			stateActionRequired = STATE_NONE;
			break ;
				
		case STATE_NONE:


		//	if (r_sim65816.get_state() == IN_PAUSE)
			if (s == PAUSE_REQUESTED )	// !!!! L'ETAT PEUT AVOIR CHANGE DEPUIS LE DEBUT DE LA FONCTION : NE LE RELIT PAS!
			{
				r_sim65816.set_state(IN_PAUSE);
				printf("in pause (cache=%dMB, maxStateSize=%dMB)\n",cacheSize/(1024*1024),savedState::maxSize/(1024*1024));

				while(r_sim65816.get_state() == IN_PAUSE || ( iCurState!=-1 && !states[iCurState].isEmpty() && !states[iCurState].fullsave ) )
				{
					if (r_sim65816.should_emulator_terminate())
					{
						printf("aborting PAUSE mode...\n");
						r_sim65816.set_state(RUNNING);
						return;
					}
					x_sleep(1);	// 50hz

					if (targetStateSens)
					{
						double dtime = get_dtime();
						double  delaysincelastrefresh = dtime - lastrefreshcycs;
						if (delaysincelastrefresh > VBL_DELAY_BETWEEN_SCREEN/2/60.0)
						{
							if (targetStateSens>0)
								iCurState = (iCurState +1 )%MAX_STATE;	
							else
								iCurState  = (MAX_STATE+iCurState -1 )%MAX_STATE;

							states[iCurState].display();
		
							if (states[iCurState].fullsave)
								targetStateSens = 0;
							else
								lastrefreshcycs = dtime;

						}

					}

				}

				printf("out pause\n");

				
				if (iCurState!=-1)
				{
					
					if (!states[iCurState].isEmpty())
						states[iCurState].restore();
				
					int n = (iCurState+1)%MAX_STATE;
					while(n!=iNextState)
					{
						delete_state(n);
						n = ( n + 1 ) % MAX_STATE;
					}
					iNextState = (iCurState+1)%MAX_STATE;
	
					nextScreenVBL = g_sim65816.g_vbl_count + VBL_DELAY_BETWEEN_SCREEN;
					nextStateVBL = g_sim65816.g_vbl_count + NB_SCREEN_PER_STATE*VBL_DELAY_BETWEEN_SCREEN;

				}

			}
			break;
	}
#endif
//	double delay = get_dtime() - d;
//	g_graph.add_graph(1,delay,g_sim65816.g_vbl_count);
	
}


#define CHECK_RESTORE_VAR(CLASS,VAR) \
	if ( g_##CLASS.VAR != ((s_##CLASS*)(##CLASS.data))->##VAR) \
	printf("serialization error for var "#CLASS"."#VAR" %d vs. %d",g_##CLASS.VAR,((s_##CLASS*)(##CLASS.data))->##VAR);

#define CHECK_RESTORE_CLASS(CLASS) \
	if ( sizeof(s_##CLASS) != ((s_##CLASS*)(##CLASS.data))->_size) \
		printf("serialization error for class "#CLASS" %d vs. %d\n",sizeof(s_##CLASS),((s_##CLASS*)(##CLASS.data))->_size );

#define SAVE_VERSION 0x01020304

void serialize::save(const char*_name,void* _fileptr)
{

	FILE* f= (FILE*)_fileptr;
	fwrite(_name,1,4,f);
	word32 version = SAVE_VERSION;
	fwrite(&version,1,sizeof(version),f);
	fwrite(&size,1,sizeof(size),f);
	fwrite(data,1,size,f);

}

int serialize::load(const char*_name,void* _fileptr)
{
	char name[5];
	memset(name,0,5);
	FILE* f= (FILE*)_fileptr;
	fread(name,1,4,f);
	if (strcmp(name,_name))
	{
		printf("#%s save state name mismatch(%s vs %s)\n",_name,_name,name);
		return 0;
	}

	word32 version = 0;
	fread(&version,1,sizeof(version),f);
	if (version != SAVE_VERSION)
	{
		printf("#%s save state version mismatch(%X vs %X)\n",_name,version,SAVE_VERSION);
		return 0;
	}

	int nbread = fread(&size,1,sizeof(size),f);
	if (nbread !=4)
	{
		printf("#%s save state size mismatch(%d)\n",_name,size);
		return 0;
	}

	fastalloc = 0;
	data = (void*)malloc(size);
	nbread = fread(data,1,size,f);
	if (nbread!=size)
	{
		printf("#%s save state data mismatch(%d vs %d)\n",_name,nbread,size);
		return 0;
	}

	return 1;
}

void savedState::writeToDisk(const char* _filename)
{
	if (isEmpty())
	{
		printf("nothing to save");
		return ;
	}
	if (_filename==NULL)
	{
		printf("no filename");
		return ;
	}
	FILE* f= fopen(_filename,"wb");
	if (f==NULL)
	{
		printf("cannot create file");
		return ;
	}

	fwrite(&param,1,sizeof(savedStateParam),f);

	adb.save("adb ",f);
	async_event.save("asyn",f);
	clock.save("cloc",f);
	iwm.save("iwm ",f);
	moremem.save("more",f);
	paddles.save("padd",f);
	scc.save("scc ",f);
	sim65816.save("sim6",f);
	sound.save("soun",f);
	video.save("vide",f);
//	offscreen.save("offs",f);	ne sauve pas l'offscreen
	
	

	
	fclose(f);
}

int savedState::loadFromDisk(const char* _filename)
{
	release();
	int ret = loadFromDiskInternal(_filename);
	if (!ret)
		release();
	return ret;
}

int savedState::loadFromDiskInternal(const char* _filename)
{

	FILE* f= fopen(_filename,"rb");
	if (f==NULL)
	{
		printf("cannot load %s\n",_filename);
		return 0;
	}

	int nbread = fread(&param,1,sizeof(savedStateParam),f);
	if (nbread!=sizeof(savedStateParam))
	{
		printf("could not read savedStateParam\n");
		return 0;
	}
	if (param.size != sizeof(savedStateParam))
	{
		printf("alignement mismatch\n");
		return 0;
	}

	if (param.version != SAVEDSTATE_SIG)
	{
		printf("invalid sig\n");
		return 0;
	}
	
	int ret = adb.load("adb ",f);
	if (ret==0)
		return 0;

	ret = async_event.load("asyn",f);
	if (ret==0)
		return 0;

	clock.load("cloc",f);
	if (ret==0)
		return 0;

	iwm.load("iwm ",f);
	if (ret==0)
		return 0;

	moremem.load("more",f);
	if (ret==0)
		return 0;

	paddles.load("padd",f);
	if (ret==0)
		return 0;

	scc.load("scc ",f);
	if (ret==0)
		return 0;

	sim65816.load("sim6",f);
	if (ret==0)
		return 0;

	sound.load("soun",f);
	if (ret==0)
		return 0;

	video.load("vide",f);
	if (ret==0)
		return 0;

	offscreen.release();
	/*
	offscreen.load("offs",f);
	if (ret==0)
		return 0;
	*/
	

	return 1;
}

void savedState::restore()
{
	//writeToDisk("d:\\temp\\test.bin");

	if (isEmpty())
	{
		printf("nothing to restore!\n");
		return ;
	}

#if (!defined(MAC) || defined(ACTIVEGSPLUGIN)) && !defined(ACTIVEGS_NOSAVESTATE)
	
	debug_printf("current state :vbl:%d  dcycs:%f last_vbl_dcys:%f\n",g_sim65816.g_vbl_count,g_sim65816.g_cur_dcycs,g_sim65816.g_last_vbl_dcycs);
	
#ifdef WIN32
	CHECK_RESTORE_CLASS(sim65816);	
	CHECK_RESTORE_VAR(sim65816,g_mem_size_total);
#endif
	g_adb.in(&adb);
	g_async_event.in(&async_event);
	g_clock.in(&clock);
	g_iwm.in(&iwm);
	g_moremem.in(&moremem);
	g_paddles.in(&paddles);
	g_scc.in(&scc);
	g_sim65816.in(&sim65816);
	g_sound.in(&sound);
	g_video.in(&video);

	r_sim65816.request_update_emulator_runtime_config();
	display();
	printf("restoring vbl:%d  dcycs:%f last_vbl_dcys:%f\n",param.vbl,param.dcycs,g_sim65816.g_last_vbl_dcycs);
	
#endif
}

void s_savestate::init()
{
#ifdef WIN32
	 MEMORYSTATUS memStatus = {0};
	 ::GlobalMemoryStatus(&memStatus);
   DWORD dwTotalPhys =memStatus.dwTotalPhys;
   printf("Total physical memory: %u MB\n", dwTotalPhys / (1024*1024) );
#endif

	reset_rewind();

	stateActionRequired=STATE_NONE;
	state.release();

#ifdef CACHE_SIZE
	cache = (byte*)malloc(CACHE_SIZE);
	printf("Nb State = %d\n",MAX_STATE);
	printf("Max replay = %fs\n",(float)MAX_STATE/60.0);
	printf("Key duration = %fs\n",(float)(NB_SCREEN_PER_STATE*VBL_DELAY_BETWEEN_SCREEN)/60.0);	
#else
	cache=NULL;
#endif
	cachefree = 0;
	cachepos = 0;
	cacheSize= 0;


}


void* s_savestate::x_free(void* _data,int _size, int _fastalloc)
{
	if (!cache || !_fastalloc)
	{
		free(_data);
		return NULL; 
	}

	int pos = (intptr_t)_data - (intptr_t)cache;

	// xxxx#######xxFREEE....POS#####
	
	//printf("freeing :%d (size:%d)\n",pos,_size);
	// POSxxxxxx######xxxxFREEE

	if (cachepos<=pos || cachepos>cachefree)
	{
		cachepos = pos + _size;
	//	printf("newpos:%d\n",cachepos);
	}
	
	cacheSize -= _size;
	return NULL;
}

int	s_savestate::get_free_memory_size()
{
#ifdef CACHE_SIZE
	int p  = cachepos;
	int f  = cachefree;

	if (f>=p) 
		// #####POS.....FREE#######
		return MAX(p,CACHE_SIZE-f);
	else
		// ....FREE#####POS...
		return p-f;
#else
	return 0;
#endif
}
	
void* x_free(void* _data,int _size, int _fastalloc)
{
	return g_savestate.x_free(_data,_size,_fastalloc);

}

void* s_savestate::x_malloc(int _size, int _fastalloc)
{

	if (!cache || !_fastalloc)
	{
		void* p = (void*)malloc(_size);
		if (!p)
			x_fatal_exit("Out of Memory Error : relaunch with rewind mode off");
		cacheSize+=_size;
		return p;
	}
#ifndef CACHE_SIZE
	// ne doit jamais arriver
	return NULL;
#else
	int c = cachefree;

	if (c >= cachepos)
	{
		// ......POS#####################FREE.........

		if (c+_size > CACHE_SIZE)
		{
			// on boucle
			c = 0;
			// FREEE......POS#####################00000000
			if (c+_size > cachepos)
			{
				printf("x_malloc : out of memory error\n");
				return NULL;
			}
		}
		else
		{
			// ok
		}
	}
	else
	{
		// ######FREE......POS##########################

		if (c+_size < cachepos)
		{
			// il y a de la place entre free & pos = on l'utilise
			// ok
		}
		else
		{
			printf("x_malloc: out of memory error\n");
			return NULL;

		}
	}
	
	cachefree = c+_size;
	cacheSize += _size;
//	printf("alloc size:%d ptr:%d (cachepos:%d cachefree:%d)\n",_size,c,cachepos,cachefree);
	return &cache[c];
#endif
}

void* x_malloc(int _size, int _fastalloc)
{
	return g_savestate.x_malloc(_size,_fastalloc);

}
void* x_realloc(void* ptr,int _newsize, int _formersize)
{
	printf("x_realloc not implemented\n");
	return ptr;
	/*
	void*p  =(void*)x_malloc(_newsize);
	memcpy(p,ptr,_newsize);
	x_free(ptr,_formersize);
	return p;
	*/
}

void s_savestate::reset_state()
{
	state.release();
}

void s_savestate::reset_rewind()
{
	printf("reset_rewind\n");
	for(int i=0;i<MAX_STATE;i++)
		states[i].release();
	
	// reset mempory pointers
	cachefree=0;
	cachepos=0;
	iNextState = 0;

	// reset recording vbl
	nextScreenVBL = 0;
	nextStateVBL = 0;

}

void s_savestate::shut()
{

	reset_rewind();
	state.release();
	
	if (cache)
	{
		free(cache);
		cache=NULL;
	}
		
}

s_savestate::s_savestate()
{
	
    stateActionRequired = 0;
	iNextState = 0;
	iCurState = 0 ;
	nextStateVBL= 0;
	nextScreenVBL = 0;
	targetStateSens = 0;
	lastrefreshcycs = 0.0;
	cache = NULL;
	cachepos = 0;
	cachefree = 0;
    cacheSize = 0;
    

}

s_savestate::~s_savestate()
{
	
}

