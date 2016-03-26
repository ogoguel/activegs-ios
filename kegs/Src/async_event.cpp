/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include <math.h>
#include "defc.h"
#include "adb.h"
#include "async_event.h"
#include "sim65816.h"
#include "savestate.h"

extern void x_check_system_input_events();
extern void x_update_modifiers(word32);


s_async_event g_async_event;

void reset_async_event()
{
	
	g_async_event.max_event_key = 0;
	g_async_event.cur_event_key = 0;
	g_async_event.unstacking_events = 0;
	
	
}

void add_event_delay()
{
	g_async_event.mac_events[g_async_event.max_event_key].type = DELAY;
	g_async_event.max_event_key = (g_async_event.max_event_key+1)%MAX_MAC_EVENT_KEYS;

}

void add_event_key(int _key,int _isup)
{

//	printf("ADDING %X (%d)\n",_key,_isup);
#ifdef MAC	
	// Remap 0x7b-0x7e to 0x3b-0x3e (arrow keys on new mac keyboards) 
	if(_key >= 0x7b && _key <= 0x7e) {
		_key = _key - 0x40;
	}
#endif
	
#ifdef ACTIVEGS
	if (r_sim65816.get_state()==IN_PAUSE)
	{
		g_savestate.handleKey(_key,_isup);
		return ; 
	}
#endif

	if (r_sim65816.get_state()!=RUNNING) return ;


	g_async_event.mac_events[g_async_event.max_event_key].key = _key;
	g_async_event.mac_events[g_async_event.max_event_key].isup = _isup;
	g_async_event.mac_events[g_async_event.max_event_key].type = KEY;
	
	g_async_event.max_event_key = (g_async_event.max_event_key+1)%MAX_MAC_EVENT_KEYS;
}

/*
void x_add_event_key(int _key, int _up)
{
	add_event_key(_key,_up);
}
 */
 

extern int is_mouse_used_by_host();

void simulate_space_event()
{
	if (g_adb.g_simulate_space)
	{
		// regarde si la souris a été lu depuis 1s => simule un appui barre espace
		if (!is_mouse_used_by_host()) //g_last_c027_read < ( g_vbl_count - 60 ) )
		{
			printf("Simulate KEY (%X)\n",g_adb.g_simulate_space);
			add_event_key(g_adb.g_simulate_space,0);
			add_event_key(g_adb.g_simulate_space,1);
		}
	}
}

void add_event_mouse(int _x,int _y, int _state, int _button)
{

	if (r_sim65816.get_state()!=RUNNING) return ;

	g_async_event.mac_events[g_async_event.max_event_key].x = _x;
	g_async_event.mac_events[g_async_event.max_event_key].y = _y;
	g_async_event.mac_events[g_async_event.max_event_key].state = _state;
	g_async_event.mac_events[g_async_event.max_event_key].button = _button;
	g_async_event.mac_events[g_async_event.max_event_key].type = MOUSE;
	g_async_event.max_event_key = (g_async_event.max_event_key+1)%MAX_MAC_EVENT_KEYS;
}

void add_event_modifier(unsigned int _mod)
{
    /*
    static int lastmod = 0;
    if (_mod != lastmod)
        printf("newmod:%x\n",_mod);
    lastmod = _mod;
     */
	if (r_sim65816.get_state()!=RUNNING) return ;

	g_async_event.mac_events[g_async_event.max_event_key].mod = _mod;
	g_async_event.mac_events[g_async_event.max_event_key].type = MODIFIERS;
	g_async_event.max_event_key = (g_async_event.max_event_key+1)%MAX_MAC_EVENT_KEYS;
}



void check_input_events()
{
	
	x_check_system_input_events();

	g_async_event.unstacking_events = 1;
#define MAX_STACK 100
	int nbstack = 0;
	int	stacks[MAX_STACK];
	while(g_async_event.cur_event_key!=g_async_event.max_event_key)
	{
		int i = g_async_event.cur_event_key;
		switch(g_async_event.mac_events[i].type)
		{
			case DELAY:
				// skip event
				g_async_event.cur_event_key = (g_async_event.cur_event_key+1)%MAX_MAC_EVENT_KEYS;		
				g_async_event.unstacking_events = 0;
				return;
		
			case MODIFIERS:
				x_update_modifiers(	g_async_event.mac_events[i].mod);
				break;
			case KEY:
			{
				// regarde si on a déjà pushé l'évement dans cette frame
				int k = g_async_event.mac_events[i].key;
				int delayed=0;		
				for(int s = 0;s<nbstack;s++)
				{
					if (k==stacks[s])
					{
						delayed=1;
						break;
					}
				}
				
				if (delayed || nbstack==MAX_STACK)			
				{
				//	printf("delayed kbd event\n");
					g_async_event.unstacking_events = 0;
					return;
				}
#ifdef _DEBUG
           //     printf("### (vbl:%d) key %x %d\n",g_sim65816.g_vbl_count,g_async_event.mac_events[i].key, g_async_event.mac_events[i].isup);
#endif
				stacks[nbstack++]=k;
				adb_physical_key_update(	g_async_event.mac_events[i].key,   g_async_event.mac_events[i].isup);
				break;
			}
				
			case MOUSE:
		
				// Pas besoin delayer les events : géré par le systeme
				
				/*
				if (g_async_event.mac_events[i].button>0) // -1 means warp pointer
				{
					int s = g_async_event.mac_events[i].state & mac_events[i].button;
					int nblastvbl  = g_vbl_count - g_input_last_vbl;
					if (nblastvbl<0)
					{
						printf("alert !!!");
					}
					if ( nblastvbl <= 1 && s!= g_last_button_state )
					{
						printf("delaying mouse event %d,%d (%d) (%d)\n",g_async_event.mac_events[i].x,mac_events[i].y,s,nblastvbl);
						g_async_event.unstacking_events = 0;

						return;
					}
					g_input_last_vbl = g_vbl_count ;
					g_last_button_state = s;
				}
				 */
				
				update_mouse(	g_async_event.mac_events[i].x ,
							 g_async_event.mac_events[i].y ,
							 g_async_event.mac_events[i].state ,
							 g_async_event.mac_events[i].button) ;
				break;
		}
		g_async_event.cur_event_key = (g_async_event.cur_event_key+1)%MAX_MAC_EVENT_KEYS;		
	}

	g_async_event.unstacking_events = 0;
	
}

