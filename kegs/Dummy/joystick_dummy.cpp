/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../src/defc.h"
#ifndef _MSC_VER	// OG Unknown MSFT header
#include <sys/time.h>
#endif
#include "../src/paddles.h"
#include "../src/moremem.h"

/*
extern int g_joystick_native_type1;		
extern int g_joystick_native_type2;		
extern int g_joystick_native_type;		
extern int g_paddle_buttons;
extern int g_paddle_val[];
*/
void
joystick_init()
{
	g_joystick_native_type1 = -1;
	g_joystick_native_type2 = -1;
	g_joystick_native_type = -1;
}

void
joystick_update(double dcycs)
{
	int	i;

	for(i = 0; i < 4; i++) {
		g_paddles.g_paddle_val[i] = 32767;
	}
	g_moremem.g_paddle_buttons = 0xc;
}

void
joystick_update_buttons()
{
}

// OG
void joystick_shut()
{
}

