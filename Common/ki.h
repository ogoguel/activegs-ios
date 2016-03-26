/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifndef KEGS_KI_H
#define KEGS_KI_H

#include <stdio.h>
# if defined( _WIN32) && !defined(UNDER_CE)	
	#include <io.h>
#endif

#include "../kegs/src/defc.h"
#include "../kegs/src/iwm.h"
extern void	showStatus(const char* format,...);
extern void	showError(const char* format,...);
extern void	showProgress(const char* file,int percent);


// Emulator Interface

// Struct

#define MAXSLOT7DRIVES	MAX_C7_DISKS

//#define	HALT_WANTTOQUIT	0x20
#define	HALT_WANTTOBRK	0x40





// Globals

//extern	void (*ki_loadingptr)(int motorOn,int slot,int drive);
//extern	int	g_limit_speed;
extern	int g_fast_disk_emul;
//extern	unsigned char	*g_bram_ptr;
//extern int g_joystick_type;
extern int toggleJoystick(int);
//OGMAC
#ifdef _WIN32
#include <windows.h>
void win_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags);
extern	void kegsinit(HWND);
extern	LRESULT CALLBACK win_event_handler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
//extern	void  add_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags);
#endif

/*
extern	int	g_pause;
extern  int	g_halt_on_bad_read;
extern  int	g_ignore_bad_acc ;
extern  int	g_ignore_halts ;
*/

// Function
extern	const char* getLocalIMG(int _slot,int _drive);

extern	int	kegsmain(int argc, char** argv);
extern void	win_event_redraw();

#define get_halt_act() g_sim65816.halt_sim
extern	void	set_halt_act(int);
extern  void	clr_halt_act(void);


// Interface Kegs -> Control

extern	void updateBramParameters(unsigned char* bram);
extern  void readImageInfo(int _slot,int _drive, int _imagelen, int _imagestart, int _isprodos);
extern	int	mountImages(void);
extern void disableConsole(void);
extern	void fatalExit(int ret);
extern	void setInitialized(int _value);
extern	void setAcceptEvents(int _value);



#endif // KEGS_KI_H

//
