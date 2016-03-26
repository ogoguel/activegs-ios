/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"

enum ui_msg
{
	LOADING_ON=1304,
	LOADING_OFF,
	REFRESH_STATUS,
	SHOW_CURSOR
};

enum {
	PLATFORM_WINDOWS = 100,
	PLATFORM_MACOSX,
	PLATFORM_IOS,
	PLATFORM_ANDROID };


enum {
	ENV_DESKTOP = 2000,
	ENV_TOUCH,
	ENV_BROWSER };





struct s_driver
{
	int	  platform ;
	int environment;
	char activegsxml[1024];
	void (*x_config_load_roms)();
	void (*x_post_event)(const char* _msg);
	int	(*x_handle_fkey)(int _key);
	void (*x_handle_state)();
	void (*x_handle_state_on_paint)(float _width,float _height);
	void (*x_apply_default_options)(void*);
	void (*x_fixed_memory_ptr)();
    void (*x_notify_eject)();
    

	s_driver()
	{
		memset(this,0,sizeof(*this));
	}

	void init(void (*_driver)());
	void setLaunchXML(const char* _xml){
		if (_xml!=NULL)
			strcpy(activegsxml,_xml);
		else
			activegsxml[0] = 0;
	}
};

extern s_driver g_driver;