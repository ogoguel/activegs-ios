/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

enum event_type
{
	KEY,MOUSE,MODIFIERS,DELAY
};

#define MAX_MAC_EVENT_KEYS 1024
#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

typedef struct 
{
	enum event_type type;
	int key, isup;
	int x,y,state,button;
	unsigned int mod;
	
}struct_event XCODE_PRAGMA_PACK;


class  s_async_event : public serialized
{
public:

	int max_event_key ;
	int cur_event_key ;
	int unstacking_events ;
	
	struct_event mac_events[MAX_MAC_EVENT_KEYS] ;

	s_async_event()
	{
		memset(this,0,sizeof(*this));
	}

	DEFAULT_SERIALIZE_IN_OUT ;

} XCODE_PRAGMA_PACK;

#ifdef _WIN32
#pragma pack(pop)
#endif

extern void add_event_key(int _key,int _isup);
extern void add_event_mouse(int _x,int _y, int _state, int _button);
extern void add_event_modifier(unsigned int _mod);
extern void add_event_delay();

extern s_async_event g_async_event;
