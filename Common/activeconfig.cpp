/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "Cemulatorctrl.h"
#include "../kegs/Src/iwm.h"
#include "../kegs/Src/config.h"
#include "../kegs/Src/savestate.h"

void x_notify_disk_insertion(int _size,int _start, int _isprodos,int slot, int drive)
{

	readImageInfo(slot,drive+1,_size,_start,_isprodos);
}


void checkImages()
{

	if (mountImages())
	{
		printf("Mounting images...\n");
		
		g_iwm.g_highest_smartport_unit = -1;
			
		insert_disk(5,0,getLocalIMG(5,1),0,0,0,0);
		insert_disk(5,1,getLocalIMG(5,2),0,0,0,0);
		insert_disk(6,0,getLocalIMG(6,1),0,0,0,0);
		insert_disk(6,1,getLocalIMG(6,2),0,0,0,0);
	
		for(int disk=1;disk<MAXSLOT7DRIVES;disk++)
			insert_disk(7,disk-1,getLocalIMG(7,disk),0,0,0,0);
		
		x_refresh_panel(PANEL_DISK);

		g_savestate.reset_rewind();
		g_savestate.reset_state();
	}
}

void config_init() 
{ 
	
}

//char targetSpeed[256];
char estimatedSpeed[256];

void	x_update_info(const char *_speed)
{
//	strcpy(targetSpeed,target);
	strcpy(estimatedSpeed,_speed);
}



void x_fatal_exit(const char* _str)
{
	x_display_alert(1,"ActiveGS Halted!",_str);	
}

void x_alert(const char* format,...)
{
	MyString theAlert;
	
	va_list argptr;
	va_start( argptr, format );  
	theAlert.FormatV(format,argptr);
	va_end(argptr);

	x_display_alert(0,"ActiveGS Warning",theAlert.c_str());	
}


void x_exit(int ret)
{
	MyString	str;
	str.Format("Critical Error : %d",ret);
	x_fatal_exit(str.c_str());
	
}

