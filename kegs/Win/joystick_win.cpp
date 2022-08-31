/************************************************************************/
/*			KEGS: Apple //gs Emulator			*/
/*			Copyright 2002 by Kent Dickey			*/
/*									*/
/*		This code is covered by the GNU GPL			*/
/*									*/
/*	The KEGS web page is kegs.sourceforge.net			*/
/*	You may contact the author at: kadickey@alumni.princeton.edu	*/
/************************************************************************/

const char rcsid_joystick_driver_c[] = "@(#)$KmKId: joystick_driver.c,v 1.12 2004-10-17 21:28:48-04 kentd Exp $";

#include "../src/defc.h"
#include "../src/moremem.h"
#include "../src/paddles.h"

# include <windows.h>
# include <mmsystem.h>


void joystick_init()
{
	JOYINFO info;
	JOYCAPS joycap;
	MMRESULT ret1, ret2;
	int	i;

	//	Check that there is a joystick device
	if(joyGetNumDevs() <= 0) {
		printf("No joystick hardware detected\n");
		g_joystick_native_type1 = -1;
		g_joystick_native_type2 = -1;
		return;
	}

	g_joystick_native_type1 = -1;
	g_joystick_native_type2 = -1;

	//	Check that at least joystick 1 or joystick 2 is available
	ret1 = joyGetPos(JOYSTICKID1, &info);
	ret2 = joyGetDevCaps(JOYSTICKID1, &joycap, sizeof(joycap));
	if(ret1 == JOYERR_NOERROR && ret2 == JOYERR_NOERROR) {
		g_joystick_native_type1 = JOYSTICKID1;
		printf("Joystick #1 = %s\n", joycap.szPname);
		g_joystick_native_type = JOYSTICKID1;
	}
	ret1 = joyGetPos(JOYSTICKID2, &info);
	ret2 = joyGetDevCaps(JOYSTICKID2, &joycap, sizeof(joycap));
	if(ret1 == JOYERR_NOERROR && ret2 == JOYERR_NOERROR) {
		g_joystick_native_type2 = JOYSTICKID2;
		printf("Joystick #2 = %s\n", joycap.szPname);
		if(g_joystick_native_type < 0) {
			g_joystick_native_type = JOYSTICKID2;
		}
	}

	for(i = 0; i < 4; i++) {
		g_paddles.g_paddle_val[i] = 32767;
	}
	g_moremem.g_paddle_buttons = 0xc;

	joystick_update(0.0);
}

void
joystick_update(double dcycs)
{
	JOYCAPS joycap;
	JOYINFO info;
	UINT	id;
	MMRESULT ret1, ret2;

	id = g_joystick_native_type;

	ret1 = joyGetDevCaps(id, &joycap, sizeof(joycap));
	ret2 = joyGetPos(id, &info);
	if(ret1 == JOYERR_NOERROR && ret2 == JOYERR_NOERROR) {
		// OG Convert to -32767/32767
		g_paddles.g_paddle_val[0] = (info.wXpos - joycap.wXmin) * 65535 / (joycap.wXmax - joycap.wXmin)  - 32768;
		g_paddles.g_paddle_val[1] = (info.wYpos - joycap.wYmin) * 65535 /	(joycap.wYmax - joycap.wYmin) - 32768;

			if(info.wButtons & JOY_BUTTON1) {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons | 1;
		} else {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons & (~1);
		}
		if(info.wButtons & JOY_BUTTON2) {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons | 2;
		} else {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons & (~2);
		}
		paddle_update_trigger_dcycs(dcycs);
	}
}

void
joystick_update_buttons()
{
	JOYINFOEX info;
	UINT id;

	id = g_joystick_native_type;

	info.dwSize = sizeof(JOYINFOEX);
	info.dwFlags = JOY_RETURNBUTTONS;
	if(joyGetPosEx(id, &info) == JOYERR_NOERROR) {
		if(info.dwButtons & JOY_BUTTON1) {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons | 1;
		} else {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons & (~1);
		}
		if(info.dwButtons & JOY_BUTTON2) {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons | 2;
		} else {
			g_moremem.g_paddle_buttons = g_moremem.g_paddle_buttons & (~2);
		}
	}
}

// OG
void joystick_shut()
{
}
