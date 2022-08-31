/************************************************************************/
/*			KEGS: Apple //gs Emulator			*/
/*			Copyright 2002 by Kent Dickey			*/
/*									*/
/*		This code is covered by the GNU GPL			*/
/*									*/
/*	The KEGS web page is kegs.sourceforge.net			*/
/*	You may contact the author at: kadickey@alumni.princeton.edu	*/
/************************************************************************/

#pragma once
const char rcsid_windriver_c[] = "@(#)$KmKId: windriver.c,v 1.11 2004-11-24 16:43:46-05 kentd Exp $";

/* Based on code from Chea Chee Keong from KEGS32, which is available at */
/*  http://www.geocities.com/akilgard/kegs32 */

#define WIN32_LEAN_AND_MEAN	/* Tell windows we want less header gunk */
#define STRICT			/* Tell Windows we want compile type checks */

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
//#include <winsock2.h>

#ifdef ACTIVEGS
#include "../src/raster.h"
#endif

#ifdef ACTIVEGSKARATEKA
#include "../../drivers/karateka.h"
#include "../../drivers/karateka_interface.h"
#endif

//#define USE_GDIPLUS

#ifdef USE_GDIPLUS
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#endif

#include "../src/defc.h"
#include "../src/protos_windriver.h"
#include "../src/adb.h"
#include "../src/sim65816.h"
#include "../src/video.h"
#include "../src/graphcounter.h"
#include "../src/driver.h"

//extern	int g_screen_mdepth;
//extern	int g_screen_depth;

// OG replaced g_quit_sim_now by quitEmulator
// extern int g_quit_sim_now;


int	g_use_shmem = 1;
int	g_has_focus = 0;
int	g_auto_repeat_on = -1;

//extern Kimage g_mainwin_kimage;

//HDC	g_main_dc;	// OG dc cannot be a global
HDC	g_main_cdc;
//int	g_main_height = 0;

int	g_win_capslock_down = 0;

extern void x_async_paint();

extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

extern word32 g_full_refresh_needed;

extern int g_border_sides_refresh_needed;
extern int g_border_special_refresh_needed;
//extern int g_status_refresh_needed;

extern   int  g_lores_colors[];
extern int g_cur_a2_stat;

extern int g_a2vid_palette;

//extern int g_installed_full_superhires_colormap;

extern int g_screen_redraw_skip_amt;

extern word32 g_a2_screen_buffer_changed;

HWND	g_hwnd_main;
BITMAPINFO *g_bmapinfo_ptr = 0;
volatile BITMAPINFOHEADER *g_bmaphdr_ptr = 0;

int g_num_a2_keycodes = 0;


int g_win_button_states = 0;

// OG Added calc_ratio
int x_calc_ratio(float& ratiox,float& ratioy);

/*
// FROM THE IPHONE VERSION

struct kbdDefinition kbdDefinitions[]=
{
{ KBDWIDTH,"esc",0x35,0,0 },
{ KBDWIDTH,"1",0x12,"!",0 },
{ KBDWIDTH,"2",0x13,"@",0 },
{ KBDWIDTH,"3",0x14,"#",0 },
{ KBDWIDTH,"4",0x15,"$",0 },
{ KBDWIDTH,"5",0x17,"%",0 },
{ KBDWIDTH,"6",0x16,"^",0 },
{ KBDWIDTH,"7",0x1A,"&",0 },
{ KBDWIDTH,"8",0x1C,"*",0 },
{ KBDWIDTH,"9",0x19,"(",0 },
{ KBDWIDTH,"0",0x1D,")",0 },
{ KBDWIDTH,"-",0x1B,"_",0 },
{ KBDWIDTH,"=",0x18,"+",0 },
{ KBDWIDTHTAB,"delete",0x33,0,0 },
{ -1,0,0,0,0 },
{ KBDWIDTHTAB,"tab",0x30,0,0 },
{ KBDWIDTH,"q",0x0C,"Q",0 },
{ KBDWIDTH,"w",0x0D,"W",0 },
{ KBDWIDTH,"e",0x0E,"E",0 },
{ KBDWIDTH,"r",0x0F,"R",0 },
{ KBDWIDTH,"t",0x11,"T",0 },
{ KBDWIDTH,"y",0x10,"Y",0 },
{ KBDWIDTH,"u",0x20,"U",0 },
{ KBDWIDTH,"i",0x22,"I",0 },
{ KBDWIDTH,"o",0x1F,"O",0 },
{ KBDWIDTH,"p",0x23,"P",0 },
{ KBDWIDTH,"[",0x21,"{",0 },
{ KBDWIDTH,"]",0x1E,"}",0 },
	{ KBDWIDTHTAB,"menu",-1,0,0 },
{ -1,0,0,0,0 },
{ KBDWIDTHCTRL,"control",0x36,0,0 },
{ KBDWIDTH,"a",0x00,"A",0 },
{ KBDWIDTH,"s",0x01,"S",0 },
{ KBDWIDTH,"d",0x02,"D",0 },
{ KBDWIDTH,"f",0x03,"F",0 },
{ KBDWIDTH,"g",0x05,"G",0 },
{ KBDWIDTH,"h",0x04,"H",0 },
{ KBDWIDTH,"j",0x26,"J",0 },
{ KBDWIDTH,"k",0x28,"K",0 },
{ KBDWIDTH,"l",0x25,"L",0 },
{ KBDWIDTH,";",0x29,":",0 },
{ KBDWIDTH,"'",0x27,"\"",0 },
{ KBDWIDTHRET,"return",0x24,0,0 },
{ -1,0,0,0,0 },
	{ KBDWIDTHSHIFT,"shift",0x38,0,0 },
	{ KBDWIDTH,"z",0x06,"Z",0 },
	{ KBDWIDTH,"x",0x07,"X",0 },
	{ KBDWIDTH,"c",0x08,"C",0 },
	{ KBDWIDTH,"v",0x09,"V",0 },
	{ KBDWIDTH,"b",0x0B,"B",0 },
	{ KBDWIDTH,"n",0x2D,"N",0 },
	{ KBDWIDTH,"m",0x2E,"M",0 },
	{ KBDWIDTH,",",0x2B,"<",0 },
	{ KBDWIDTH,".",0x2F,">",0 },
	{ KBDWIDTH,"/",0x2C,"?",0 },
	{ KBDWIDTHSHIFT,"shift",0x38,0,0 },
	{ -1,0,0,0,0 },
	{ KBDWIDTH,"caps",0x39,0,0 },
	{ KBDWIDTH,"option",0x37,0,0 },
	{ KBDWIDTHAPPLE,"",0x3A,0,0 },
	{ KBDWIDTH,"`",0x12,0,0 },
	{ KBDWIDTHSPACE," ",0x31,0,0 },
	{ KBDWIDTH,"x",0x13,0,0 },
	{ KBDWIDTH,"->",0x3C,0,0 },
	{ KBDWIDTH,"<-",0x3B,0,0 },
	{ KBDWIDTH,"^",0x5B,0,0 },
	{ KBDWIDTH,"v",0x13,0,0 },

	{ 0,0,0,0,0 }

};
*/

/* this table is used to search for the Windows VK_* in col 1 or 2 */
/* flags bit 8 is or'ed into the VK, so we can distinguish keypad keys */
/* regardless of numlock */
int g_a2_key_to_wsym[][3] =
{
    { 0x35,	VK_ESCAPE,	0 },
    { 0x7a,	VK_F1,	0 },
    { 0x78,	VK_F2,	0 },	// OG Was 7B but F2 is defined has 0x78 in a2_key_to_ascii
    { 0x63,	VK_F3,	0 },
    { 0x76,	VK_F4,	0 },
    { 0x60,	VK_F5,	0 },
    { 0x61,	VK_F6,	0 },
    { 0x62,	VK_F7,	0 },
    { 0x64,	VK_F8,	0 },
    { 0x65,	VK_F9,	0 },
    { 0x6d,	VK_F10,	0 },
    { 0x67,	VK_F11,	0 },
    { 0x6f,	VK_F12,	0 },
    { 0x69,	VK_F13,	0 },
    { 0x6b,	VK_F14,	0 },
    { 0x71,	VK_F15,	0 },
    { 0x7f, VK_PAUSE, VK_CANCEL+0x100 },

    { 0x32,	0xc0, 0 },		/* '`' */
    { 0x12,	'1', 0 },
    { 0x13,	'2', 0 },
    { 0x14,	'3', 0 },
    { 0x15,	'4', 0 },
    { 0x17,	'5', 0 },
    { 0x16,	'6', 0 },
    { 0x1a,	'7', 0 },
    { 0x1c,	'8', 0 },
    { 0x19,	'9', 0 },
    { 0x1d,	'0', 0 },
    { 0x1b,	0xbd, 0 },		/* '-' */
    { 0x18,	0xbb, 0 },		/* '=' */
    { 0x33,	VK_BACK, 0 },		/* backspace */
    { 0x72,	VK_INSERT+0x100, 0 },	/* Insert key */
    /*	{ 0x73,	XK_Home, 0 },		alias VK_HOME to be KP_Equal! */
    { 0x74,	VK_PRIOR+0x100, 0 },	/* pageup */
    { 0x47,	VK_NUMLOCK, VK_NUMLOCK+0x100 },	/* clear */
    { 0x51,	VK_HOME+0x100, 0 },		/* KP_equal is HOME key */
    { 0x4b,	VK_DIVIDE, VK_DIVIDE+0x100 },
    { 0x43,	VK_MULTIPLY, VK_MULTIPLY+0x100 },

    { 0x30,	VK_TAB, 0 },
    { 0x0c,	'Q', 0 },
    { 0x0d,	'W', 0 },
    { 0x0e,	'E', 0 },
    { 0x0f,	'R', 0 },
    { 0x11,	'T', 0 },
    { 0x10,	'Y', 0 },
    { 0x20,	'U', 0 },
    { 0x22,	'I', 0 },
    { 0x1f,	'O', 0 },
    { 0x23,	'P', 0 },
    { 0x21,	0xdb, 0 },	/* [ */
    { 0x1e,	0xdd, 0 },	/* ] */
    { 0x2a,	0xdc, 0 },	/* backslash, bar */
    { 0x75,	VK_DELETE+0x100, 0 },
    { 0x77,	VK_END+0x100, VK_END },
    { 0x79,	VK_NEXT+0x100, 0 },
    { 0x59,	VK_NUMPAD7, VK_HOME },
    { 0x5b,	VK_NUMPAD8, VK_UP },
    { 0x5c,	VK_NUMPAD9, VK_PRIOR },
    { 0x4e,	VK_SUBTRACT, VK_SUBTRACT+0x100 },

    // { 0x39,	VK_CAPITAL, 0 },  // Handled specially!
    { 0x00,	'A', 0 },
    { 0x01,	'S', 0 },
    { 0x02,	'D', 0 },
    { 0x03,	'F', 0 },
    { 0x05,	'G', 0 },
    { 0x04,	'H', 0 },
    { 0x26,	'J', 0 },
    { 0x28,	'K', 0 },
    { 0x25,	'L', 0 },
    { 0x29,	0xba, 0 },	/* ; */
    { 0x27,	0xde, 0 },	/* single quote */
    { 0x24,	VK_RETURN, 0 },
    { 0x56,	VK_NUMPAD4, VK_LEFT },
    { 0x57,	VK_NUMPAD5, VK_CLEAR },
    { 0x58,	VK_NUMPAD6, VK_RIGHT },
    { 0x45,	VK_ADD, 0 },

    { 0x38,	VK_SHIFT, 0 },
    { 0x06,	'Z', 0 },
    { 0x07,	'X', 0 },
    { 0x08,	'C', 0 },
    { 0x09,	'V', 0 },
    { 0x0b,	'B', 0 },
    { 0x2d,	'N', 0 },
    { 0x2e,	'M', 0 },
    { 0x2b,	0xbc, 0 },	/* , */
    { 0x2f,	0xbe, 0 },	/* . */
    { 0x2c,	0xbf, 0 },	/* / */
    { 0x3e,	VK_UP+0x100, 0 },
    { 0x53,	VK_NUMPAD1, VK_END },
    { 0x54,	VK_NUMPAD2, VK_DOWN },
    { 0x55,	VK_NUMPAD3, VK_NEXT },

    { 0x36,	VK_CONTROL, VK_CONTROL+0x100 },
    { 0x3a,	VK_SNAPSHOT+0x100, VK_MENU+0x100 },/* Opt=prntscrn or alt-r */

// OG ActiveGS map OA-CA to Win & AltKey
#ifndef ACTIVEGS
    { 0x37,	VK_SCROLL, VK_MENU },		/* Command=scr_lock or alt-l */
#else
    { 0x7f, VK_CANCEL, 0 },
    { 0x3A, VK_LWIN+0x100, VK_LWIN },
    { 0x37,	VK_MENU, 0 },		/* Command=alt-l */
    { 0x37,	VK_LMENU, 0 },		/* Command=alt-l */
    { 0x7F,	VK_SCROLL,0 },		/* RESET */
    { 0x36,	VK_LCONTROL, 0 },	// CTRL
#endif

    { 0x31,	' ', 0 },
    { 0x3b,	VK_LEFT+0x100, 0 },
    { 0x3d,	VK_DOWN+0x100, 0 },
    { 0x3c,	VK_RIGHT+0x100, 0 },
    { 0x52,	VK_NUMPAD0, VK_INSERT },
    { 0x41,	VK_DECIMAL, VK_DECIMAL },
    { 0x4c,	VK_RETURN+0x100, 0 },
    { -1, -1, -1 }
};

extern int g_config_control_panel;
extern int g_config_control_panel;	// OG Expose g_config_control_panel

void x_update_modifiers(word32)
{
}


extern void add_event_mouse(int,int,int,int);
extern void add_event_key(int,int);

extern void simulate_space_event();

void x_recenter_hw_mouse()
{
	 /* move mouse to center of screen */
	POINT	pt;  
	pt.x = X_A2_WINDOW_WIDTH/2;
      pt.y = X_A2_WINDOW_HEIGHT/2;
      ClientToScreen(g_hwnd_main, &pt);
      SetCursorPos(pt.x, pt.y);    
}

void win_event_mouse_async(int umsg,WPARAM wParam, LPARAM lParam)
{
  
    word32	flags;
    int	buttons;
    int	x, y;

    x = LOWORD(lParam);
    y = HIWORD(lParam);
    
	flags = wParam;
	buttons = (flags & 1); // + (((flags >> 1) & 1) << 2) + (((flags >> 4) & 1) << 1);

	if (umsg == WM_LBUTTONDBLCLK)
        simulate_space_event();


	 if(g_adb.g_warp_pointer==WARP_POINTER)
	 {
		 
			int buttons_changed = (g_win_button_states != buttons);
			g_win_button_states = buttons; //(g_win_button_states & ~buttons_valid) | (button_states & buttons_valid);
			if ( (x == X_A2_WINDOW_WIDTH/2) && (y == X_A2_WINDOW_HEIGHT/2) && (!buttons_changed) )
			 {
				// tell adb routs to recenter but ignore this motion 
				add_event_mouse(x, y, 0, -1);
			//	win_recenter_hw_mouse();
				return ;
			}
			
      	}
	 else
	 {
		// OG Reformat the mouse coordinates
		float ratiox,ratioy;
		if (x_calc_ratio(ratiox,ratioy))
		{
			x = (int)((float)(x)/ratiox);
			y = (int)((float)(y)/ratioy);
		}
		x  -= BASE_MARGIN_LEFT;
		y -=  BASE_MARGIN_TOP;
	}
	
	 
#if 0
    if (umsg!=WM_MOUSEMOVE)
        printf("Mouse at %d, %d fl: %08x, but: %d\n", x, y, flags, buttons);
#endif

    // ajoute l'event
#ifdef ACTIVEGSKARATEKA
//	extern void jniTouch(int mode,float x, float y, int nbtap);
	if (umsg==WM_LBUTTONDOWN)
		jniTouch(0,EVT_TOUCHBEGAN,(float)x,(float)y,1);
	else
	if (umsg==WM_LBUTTONUP)
		jniTouch(0,EVT_TOUCHENDED,(float)x,(float)y,1);
#else
    add_event_mouse(x, y, buttons, 7);
#endif

	if(g_adb.g_warp_pointer == WARP_POINTER)
		x_recenter_hw_mouse();
     
}

void win_event_key_async(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags)
{
    word32	vk;
    int	a2code;
    int	is_up;
    int	capslock_down;
    int	i;
    if((flags & 0x4000) && down)
    {
        /* auto-repeating, just ignore it */
        return;
    }
    vk = raw_vk + (flags & 0x100);
#if 0
    printf("Key event, vk=%04x, down:%d, repeat: %d, flags: %08x\n",
           vk, down, repeat, flags);
#endif
    /* remap a few keys here.. sigh */
    if((vk & 0xff) == VK_APPS)
    {
        /* remap to command */
        vk = VK_MENU;
    }
    if((vk & 0xff) == VK_CAPITAL)
    {
        // Windows gives us up-and-down events of the actual key
        //  Use GetKeyState to get the true toggle state, and pass
        //  that on to the adb interface
        capslock_down = GetKeyState(VK_CAPITAL) & 0x01;
        if(capslock_down != g_win_capslock_down)
        {
            g_win_capslock_down = capslock_down;
            //adb_physical_key_update(0x39, !capslock_down);
            add_event_key(0x39, !capslock_down);
        }
        return;		// Do no more processing!
    }
    // cherche déjà en ascii
    a2code = -1;
    int a2shift = -1;
    is_up = !down;
    /*
    	extern const int a2_key_to_ascii[][4];

    	int nbascii =128;
    	for(int i=0;i<nbascii;i++)
    	{
    		if (a2_key_to_ascii[i][1]==vk)
    		{
    			// found no shift
    			a2code = a2_key_to_ascii[i][0];
    			break;
    		}
    		else
    		if (a2_key_to_ascii[i][1]==vk)
    		{
    			a2code = a2_key_to_ascii[i][0];
    			break;

    		}
    	}

    	if (a2code!=-1)
    	{
    		add_event_key(a2code,is_up);
    		return;
    	}
    */
    /* search a2key_to_wsym to find wsym in col 1 or 2 */
    i = 0;
    for(i = g_num_a2_keycodes-1; i >= 0; i--)
    {
        a2code = g_a2_key_to_wsym[i][0];
        if((vk == g_a2_key_to_wsym[i][1]) ||
                (vk == g_a2_key_to_wsym[i][2]))
        {
            vid_printf("Found vk:%04x = %02x\n", vk, a2code);
            //	adb_physical_key_update(a2code, is_up);
            add_event_key(a2code,is_up);
            return;
        }
    }
    printf("VK: %04x unknown\n", vk);
}

void
win_event_quit(HWND hwnd)
{
    r_sim65816.quit_emulator();
}

//extern int g_needfullrefreshfornextframe  ;

extern void ui_process_message( WPARAM wParam, LPARAM lParam);

void window_needs_full_refresh()
{
    // Nothing to do : the whole offscreen is always refreshed
}

LRESULT CALLBACK
win_event_handler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
#if 0
    extern word32	g_vbl_count;
    if (umsg==WM_LBUTTONDOWN)
        outputInfo("WM_LBUTTONDOWN (%d)\n",g_vbl_count);
    if (umsg==WM_LBUTTONUP)
        outputInfo("WM_LBUTTONUP (%d)\n",g_vbl_count);
    if (umsg==WM_LBUTTONDBLCLK)
        outputInfo("WM_LBUTTONDBLCLK (%d)\n",g_vbl_count);
#endif
    switch(umsg)
    {
	case WM_USER:	
		ui_process_message(wParam,lParam);
		return 0;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_LBUTTONDBLCLK:	// OG Added dblclk as WM_LBUTTONDOWN
        win_event_mouse_async(umsg,wParam, lParam);
        return 0;
    case WM_ERASEBKGND:
        window_needs_full_refresh();
        return 1;
        break;
    case WM_PAINT:
        x_async_paint();
        break;
        HANDLE_MSG(hwnd, WM_KEYUP, win_event_key_async);
        HANDLE_MSG(hwnd, WM_KEYDOWN, win_event_key_async);
        HANDLE_MSG(hwnd, WM_SYSKEYUP, win_event_key_async);
        HANDLE_MSG(hwnd, WM_SYSKEYDOWN, win_event_key_async);
        HANDLE_MSG(hwnd, WM_DESTROY, win_event_quit);
    }
#if 0
    switch(umsg)
    {
    case WM_NCACTIVATE:
    case WM_NCHITTEST:
    case WM_NCMOUSEMOVE:
    case WM_SETCURSOR:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_CONTEXTMENU:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_PAINT:
        break;
    default:
        printf("Got umsg2: %d\n", umsg);
    }
#endif
    return DefWindowProc(hwnd, umsg, wParam, lParam);
}

// OG Added Generic kegsinit/kegsshut functions

void kegsinit(HWND _hwnd)
{
    g_hwnd_main = _hwnd;
    HDC localdc = GetDC(g_hwnd_main);
    SetTextColor(localdc, 0);
    SetBkColor(localdc, 0xFFFFFF);
    g_main_cdc = CreateCompatibleDC(localdc);
    ReleaseDC(g_hwnd_main,localdc);
}
void kegsshut()
{
    if (g_main_cdc)
    {
        DeleteDC(g_main_cdc);
        g_main_cdc = NULL;
    }
    g_hwnd_main = NULL;
}


void
x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
}

void
x_update_physical_colormap()
{
}

void
x_show_color_array()
{
    int i;
    for(i = 0; i < 256; i++)
    {
        printf("%02x: %08x\n", i, g_palette_8to1624[i]);
    }
}


// OG Add function to clear all get_images loaded (dev dependent)
void x_release_kimage(Kimage *kimage_ptr)
{
    if ((int)kimage_ptr->dev_handle != -1)
    {
        DeleteObject(kimage_ptr->dev_handle);
        kimage_ptr->dev_handle = (void*)-1;
    }
    else if (kimage_ptr->data_ptr)
    {
        free(kimage_ptr->data_ptr);
        kimage_ptr->data_ptr = NULL;
    }
}


void x_video_shut()	// Should be renamed to dev_video_shut()  ???
{
//#ifdef ACTIVEGS
    x_release_kimage(&g_kimage_offscreen);
//#endif
    x_release_kimage(&s_video.g_mainwin_kimage);
    GlobalFree(g_bmapinfo_ptr); // allocated in x_dev_video_init
    g_bmapinfo_ptr = 0;
    printf("win32 video driver end\n");
}


void x_get_kimage(Kimage *kimage_ptr)
{
    
    int	width;
    int	height;
    int	depth, mdepth;
    int	size;
    width = kimage_ptr->width_req;
    height = kimage_ptr->height;
    depth = kimage_ptr->depth;
    mdepth = kimage_ptr->mdepth;
    size = 0;

	/* Use g_bmapinfo_ptr, adjusting width, height */
#ifdef USE_GDIPLUS
	Bitmap* bmp = new Bitmap(width,height,PixelFormat32bppARGB);
	kimage_ptr->dev_handle  = bmp;

	BitmapData* bdata = new BitmapData;
	bdata->Width = width,
	bdata->Height = height;
	bdata->Stride = 4*width;
	bdata->PixelFormat = PixelFormat32bppRGB; 
	bdata->Scan0 = (VOID*)malloc(height*width*4);
	bdata->Reserved = NULL;
	kimage_ptr->dev_handle2  = bdata;

	kimage_ptr->data_ptr = (byte*)bdata->Scan0;

#else
    g_bmaphdr_ptr->biWidth = width;
    g_bmaphdr_ptr->biHeight = -height;
    HDC localdc = GetDC(g_hwnd_main);	// OG Use on the fly DC
    kimage_ptr->dev_handle = CreateDIBSection(localdc,
                             g_bmapinfo_ptr, DIB_RGB_COLORS,
                             (VOID **)&(kimage_ptr->data_ptr), NULL, 0);
    ReleaseDC(g_hwnd_main,localdc);
#endif
	printf("kim: %p, dev:%p data: %p, size: %08x\n", kimage_ptr,
           kimage_ptr->dev_handle, kimage_ptr->data_ptr, size);
    return;
}


void x_video_init()
{
    int	extra_size;

	printf("Preparing graphics system\n");

	
#ifdef USE_GDIPLUS
	GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR           gdiplusToken;
   
   // Initialize GDI+.
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

	// OG fix g_num_a2_keycodes identification
    g_num_a2_keycodes = sizeof(g_a2_key_to_wsym)/3/sizeof(int);
	g_num_a2_keycodes--; // last entry=-1

    
#ifndef UNDER_CE
    s_video.g_screen_depth = 24;
    s_video.g_screen_mdepth = 32;
#else
	// OG Forcing 16bits depth for WiNCE
    s_video.g_screen_depth = 16;
    s_video.g_screen_mdepth = 16;
    s_video.g_red_mask = 0xff;
    s_video.g_green_mask = 0xff;
    s_video.g_blue_mask = 0xff;
    s_video.g_red_left_shift = 10;
    s_video.g_green_left_shift = 5;
    s_video.g_blue_left_shift = 0;
    s_video.g_red_right_shift = 3;
    s_video.g_green_right_shift = 3;
    s_video.g_blue_right_shift = 3;
#endif
    extra_size = sizeof(RGBQUAD);
    if(s_video.g_screen_depth == 8)
    {
        extra_size = 256 * sizeof(RGBQUAD);
    }
    g_bmapinfo_ptr = (BITMAPINFO *)GlobalAlloc(GPTR,
                     sizeof(BITMAPINFOHEADER) + extra_size);
    g_bmaphdr_ptr = (BITMAPINFOHEADER *)g_bmapinfo_ptr;
    g_bmaphdr_ptr->biSize = sizeof(BITMAPINFOHEADER);
    g_bmaphdr_ptr->biWidth = A2_WINDOW_WIDTH;
    g_bmaphdr_ptr->biHeight = -A2_WINDOW_HEIGHT;
    g_bmaphdr_ptr->biPlanes = 1;
    g_bmaphdr_ptr->biBitCount = s_video.g_screen_mdepth;
    g_bmaphdr_ptr->biCompression = BI_RGB;
    g_bmaphdr_ptr->biClrUsed = 0;
    video_get_kimages();
    if(s_video.g_screen_depth != 8)
    {
        //	Allocate g_mainwin_kimage
        video_get_kimage(&s_video.g_mainwin_kimage, 0, s_video.g_screen_depth,s_video.g_screen_mdepth);
    }
    s_video.g_installed_full_superhires_colormap = 1;

	init_kimage(&g_kimage_offscreen,0,s_video.g_screen_depth, s_video.g_screen_mdepth);
    g_kimage_offscreen.width_req = X_A2_WINDOW_WIDTH;
    g_kimage_offscreen.width_act = X_A2_WINDOW_WIDTH;
    g_kimage_offscreen.height = X_A2_WINDOW_HEIGHT;
    x_get_kimage(&g_kimage_offscreen);

#ifndef UNDER_CE
    ShowWindow(g_hwnd_main, SW_SHOWDEFAULT);
#else
    ShowWindow(g_hwnd_main, SW_SHOW);
#endif
    UpdateWindow(g_hwnd_main);

#ifdef USE_RASTER
 	x_init_raster((void*)g_hwnd_main);
#endif

	printf("Done with dev_video_init\n");
    fflush(stdout);
}



void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
                   int width, int height)
{

    int pixsize = kimage_ptr->mdepth>>3 ;
    int srcrow = kimage_ptr->width_act * pixsize;
    int dstrow = g_kimage_offscreen.width_act * pixsize;
    int byte2copy = width * pixsize ;
    char* ptrdest = (char*)g_kimage_offscreen.data_ptr +dstrow*desty + destx*pixsize;
    char* srcdest = (char*)kimage_ptr->data_ptr + srcrow*srcy + srcx*pixsize;
    int dbl = 1;
#if !defined(VIDEO_SINGLEVLINE)
    int crt = (r_sim65816.get_video_fx()==VIDEOFX_CRT);
#else
	int crt = 0;
#endif
    for(int i=0; i<height; i+=1)
    {
        if (crt && (i&1))
		{
			for(int j=0;j<width;j++)
			{
				word32 pixel = ((word32*)srcdest)[j];
				pixel >>=1;
				pixel &=0x7F7F7F7F,
				((word32*)ptrdest)[j]=pixel;
			}
           // memset(ptrdest,0,byte2copy);
		}
        else
            memcpy(ptrdest,srcdest,byte2copy);
        ptrdest += dstrow;
        srcdest += srcrow;
    }
    g_video_offscreen_has_been_modified = 1;
}


void
x_push_done()
{
}

void
x_auto_repeat_on(int must)
{
}

void
x_auto_repeat_off(int must)
{
}


void
x_hide_pointer(int do_hide)
{
	static int previous_hide = 0; // visible by default
	if (previous_hide == do_hide)
		return ;
	if (do_hide==1 && g_adb.g_warp_pointer != WARP_POINTER)
		return ;

		previous_hide = do_hide;
		::PostMessage(g_hwnd_main,WM_USER,SHOW_CURSOR,do_hide);
}



void ui_hide_pointer(LONG do_hide)
{
	int count;
    if(do_hide)
    {
       count= ShowCursor(0);
    }
    else
    {
       count = ShowCursor(1);
    }
//	printf("hide pointer %d (count:%d)\n",do_hide,count);
}

void ui_process_message(WPARAM wParam, LPARAM lParam)
{
	if (wParam==SHOW_CURSOR)	
		return ui_hide_pointer(lParam);
}

void
x_full_screen(int do_full)
{
    return;
}

//int x_screen_dirty = 0;

void x_refresh_video()
{
#ifdef USE_RASTER
	extern void x_async_paint();
	x_async_paint();
#else
	InvalidateRect(g_hwnd_main,NULL,0);
	UpdateWindow(g_hwnd_main);
#endif
	
}

extern BOOL StretchBltPlus(
  HDC hdcDest,
  int nXOriginDest,
  int nYOriginDest,
  int nWidthDest,
  int nHeightDest,
  HDC hdcSrc,
  int nXOriginSrc,
  int nYOriginSrc,
  int nWidthSrc,
  int nHeightSrc,
  DWORD dwRop
) ;

void x_async_paint()
{

	
    extern void addConsoleWindow(Kimage*);
#ifndef USE_RASTER
    RECT r;
    BOOL b = GetUpdateRect( g_hwnd_main,&r,FALSE);
    if (!b)
    {
        // System  Paint : ignore
        return ;
    }
#endif
    if (!r_sim65816.is_emulator_offscreen_available())
    {
        printf("skip frame\n");
        PAINTSTRUCT ps;
        RECT r;
        GetClientRect(g_hwnd_main,&r);
        HDC hdc = BeginPaint(g_hwnd_main,&ps);
        HBRUSH brush = CreateSolidBrush(RGB(36,34,244));
        FillRect(hdc, &r, brush);
        DeleteObject(brush);
      
        EndPaint(g_hwnd_main,&ps);
       
        return ;
    }

//	DRAW_BORDER();

	CHANGE_BORDER(1,0xFF);
	CHANGE_BORDER(2,0xFF);

    addFrameRate(0);

#if defined(ACTIVEGS)
    addConsoleWindow(&g_kimage_offscreen);
#ifdef ENABLE_GRAPH

	static double lastframe = 0;
	double curframe = get_dtime();
	double delay = curframe - lastframe;
	double frame = 1/delay;
	g_graph.add_graph(1,(float)frame,g_sim65816.g_vbl_count);
	lastframe = curframe;
	g_graph.draw_graph(&g_kimage_offscreen,g_sim65816.g_vbl_count);
#endif
#endif

#ifdef USE_GDIPLUS
	PAINTSTRUCT ps;
	HDC localdc = BeginPaint(g_hwnd_main,&ps);	// Faster than GetDC in WM_PAINT response
	Graphics dest(localdc);
	
	int srcx =0;
    int srcy =0;
    int destx = 0;
    
	int desty = 0;
    int width = g_kimage_offscreen.width_act;
    int height = g_kimage_offscreen.height;
    
	Bitmap* bmp = (Bitmap*)g_kimage_offscreen.dev_handle;
	Graphics src(bmp);

	BitmapData* data = (BitmapData*)g_kimage_offscreen.dev_handle2;
	Rect rdata(0,0,width,height);
	Status s= bmp->LockBits(
      &rdata,
      ImageLockModeWrite|ImageLockModeUserInputBuf,
      PixelFormat32bppRGB,
      data);
 
   // Commit the changes and unlock the 50x30 portion of the bitmap.  
	s = bmp->UnlockBits(data);

	float ratiox,ratioy;
    int isStretched = x_calc_ratio(ratiox,ratioy);
    float xdest = (destx)*ratiox;
	float ydest = (desty)*ratioy;
	float wdest = (width)*ratiox;
	float hdest = (height)*ratioy;
  
	RectF rdest(xdest,ydest,wdest,hdest);
	/*
	SolidBrush	br(Color::AliceBlue);
	dest.FillRectangle(&br,rdest);
*/
	//dest.SetInterpolationMode(InterpolationModeHighQuality);
	dest.SetInterpolationMode(InterpolationModeDefault);
	s = dest.DrawImage(bmp, rdest, 0, 0, width, height, UnitPixel);
	
	EndPaint(g_hwnd_main,&ps);
	
  
#else
    void	*bitm_old;
    POINT	point;
    point.x = 0;
    point.y = 0;
    ClientToScreen(g_hwnd_main, &point);

//	PAINTSTRUCT ps;
//	HDC localdc = BeginPaint(g_hwnd_main,&ps);	// Faster than GetDC in WM_PAINT response
	HDC localdc = GetDC(g_hwnd_main);	// Faster than GetDC in WM_PAINT response

	HDC localcdc = g_main_cdc; 
    bitm_old = SelectObject(localcdc, g_kimage_offscreen.dev_handle);
    HRGN hrgn=NULL;

    int srcx =0;

    int srcy =0;
    int destx = 0;
    int desty = 0;
    int width = g_kimage_offscreen.width_act;
    int height = g_kimage_offscreen.height;
    float ratiox,ratioy;
    int isStretched = x_calc_ratio(ratiox,ratioy);
    if (!isStretched)
        BitBlt(localdc, destx, desty, width, height,	localcdc, srcx, srcy, SRCCOPY);
    else
    {
        float xdest = (destx)*ratiox;
        float ydest = (desty)*ratioy;
        float wdest = (width)*ratiox;
        float hdest = (height)*ratioy;
        BOOL err= StretchBlt(localdc,(int)xdest,(int)ydest,(int)wdest,(int)hdest,	localcdc, srcx, srcy,width,height, SRCCOPY);
    }
	SelectObject(localcdc, bitm_old);
	ReleaseDC(g_hwnd_main,localdc);
//	EndPaint(g_hwnd_main,&ps);
#endif
	CHANGE_BORDER(1,0);
	CHANGE_BORDER(2,0);

	if (g_driver.x_handle_state_on_paint!=NULL)
		g_driver.x_handle_state_on_paint((float)g_kimage_offscreen.width_act,(float)g_kimage_offscreen.height);
}
