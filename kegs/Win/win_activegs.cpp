#include "../../common/cemulatorctrl.h"
#include "../src/sim65816.h"
#include "../src/video.h"

extern int g_config_control_panel;
extern HWND g_hwnd_main;



int
win_nonblock_read_stdin(int fd, char *bufptr, int len)
{
	extern int activeRead(char* buf);
	return activeRead(bufptr);
}


int
x_show_alert(int is_fatal, const char *str)
{
	return 0;
}



void
x_redraw_status_lines()
{
}

// OG Added Stretchable window

int x_calc_ratio(float& ratiox,float& ratioy)
{

#ifdef UNDER_CE
	return 0;
#else
	RECT rect;
	::GetClientRect(g_hwnd_main,&rect);
	ratiox  = (float)(rect.right - rect.left)/(float)X_A2_WINDOW_WIDTH;
	ratioy  = (float)(rect.bottom - rect.top)/(float)X_A2_WINDOW_HEIGHT;
	if ( (ratiox ==  1.0 ) && (ratioy == 1.0) )
		return 0; // not stretched
	else
		return 1;
#endif
}

//extern	void addFrameRate(int _id);

//extern Kimage g_win32_offscreen ;
extern HDC	g_main_cdc;


void x_check_system_input_events()
{
#ifndef ACTIVEGS
	if (r_sim65816.get_state()==IN_PAUSE)
	{
		int oldpanel = g_config_control_panel;
		extern int halt_sim;
		g_config_control_panel = 1;
		while (!r_sim65816.should_emulator_terminate())
			Sleep(100);
		g_config_control_panel = oldpanel;
	}
#endif
}
