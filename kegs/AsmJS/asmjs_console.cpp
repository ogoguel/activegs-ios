/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../src/defc.h"
#include "../src/driver.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL.h>
#else
#include <sdl/SDL.h>
#endif

extern void kegsinit(void*);
extern void kegsshut();

extern char *g_status_ptrs[MAX_STATUS_LINES];


int
win_nonblock_read_stdin(int fd, char *bufptr, int len)
{
	
/*
	HANDLE	oshandle;
	DWORD	dwret;
	int	ret;

	errno = EAGAIN;
	oshandle = (HANDLE)_get_osfhandle(fd);	// get stdin handle
	dwret = WaitForSingleObject(oshandle, 1);	// wait 1msec for data
	ret = -1;
	if(dwret == WAIT_OBJECT_0) {
		ret = read(fd, bufptr, len);
	}
	return ret;
*/
    return 0;
}



void
x_fatal_exit(const char *str)
{
	
}
int
x_show_alert(int is_fatal, const char *str)
{
	return 0;
}

/*
int WINAPI WinMain (
	HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
     int nShowCmd)
{
	return main(0,0);
}
*/



extern int kegsmain_init(int argc, char **argv);
extern int g_config_control_panel;
extern void run_prog_init();
extern void run_prog_shut();
extern int run_prog_loop();
extern void asmjs_video_update();

void iter()
{
	static int in_iter = false;
	if (in_iter)
	{
		printf("too slow...\n");
		return ;
	}
	in_iter = true;
	run_prog_loop();
	asmjs_video_update();
	in_iter = false;
}

extern void kegs_driver();


#ifdef __cplusplus
      extern "C"
#endif

int main(int argc, char *argv[])
//int main(int argc, char **argv)
{

	
	g_driver.init(kegs_driver);


//	int ret =  kegsmain(argc, argv);
	kegsmain_init(argc,argv);

	// do go


	g_config_control_panel = 0;
	clear_halt();

	run_prog_init();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(iter,0,0);
#else
    while(1)
        iter();
#endif

	return 0;
}

extern void add_event_mouse(int _x,int _y, int _state, int _button);
	extern void simulate_space_event();

void
x_check_system_input_events()
{

	int x,y;
	SDL_PumpEvents();
	int s = SDL_GetMouseState(&x,&y);
	int b = s & SDL_BUTTON(1);
	add_event_mouse(x,y,b,1);	

	if (b)
	{
		simulate_space_event();
	}

}



void
x_redraw_status_lines()
{
	/*
	COLORREF oldtextcolor, oldbkcolor;
	char	*buf;
	int	line;
	int	len;
	int	height;
	int	margin;

	height = 16;
	margin = 0;

	HDC localdc = GetDC(g_hwnd_main);	// OG Use on the fly DC
	oldtextcolor = SetTextColor(localdc, 0);
	oldbkcolor = SetBkColor(localdc, 0xffffff);
	for(line = 0; line < MAX_STATUS_LINES; line++) {
		buf = g_status_ptrs[line];
		if(buf != 0) {
			len = strlen(buf);
			TextOut(localdc, 10, X_A2_WINDOW_HEIGHT +
				height*line + margin, buf, len);
		}
	}
	SetTextColor(localdc, oldtextcolor);
	SetBkColor(localdc, oldbkcolor);
	ReleaseDC(g_hwnd_main,localdc);
	*/
}

int x_calc_ratio(float& ratiox,float& ratioy)
{
	return 0; // not stretched
}


