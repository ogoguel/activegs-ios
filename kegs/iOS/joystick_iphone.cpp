/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../src/defc.h"
#include "../src/paddles.h"
#include "../src/moremem.h"


void
joystick_init()
{
	g_joystick_native_type1 = 0;
	g_joystick_native_type2 = -1;
	g_joystick_native_type = JOYSTICK_TYPE_NATIVE_1;
}

extern float joyX,joyY;
extern int oaButton,caButton;
extern void x_notify_paddle(int _on);

void
joystick_update(double dcycs)
{
	x_notify_paddle(1);

	// non linear joy
	float x = joyX*1.0;
	float y = joyY*1.0;
	x = (x > 1.0?1.0:x); x = (x < -1.0?-1.0:x);
	y = (y > 1.0?1.0:y); y = (y < -1.0?-1.0:y);
	
	
	
	g_paddles.g_paddle_val[0] = 32767*x;
	g_paddles.g_paddle_val[1] = 32767*y;
	g_paddles.g_paddle_val[2] = 32767;
	g_paddles.g_paddle_val[3] = 32767;

	
//	g_moremem.g_paddle_buttons = (oaButton & 1) + (caButton & 1)*2 ;
	paddle_update_trigger_dcycs(dcycs);
}

void
joystick_update_buttons()
{
//	g_moremem.g_paddle_buttons = (oaButton & 1) + (caButton & 1)*2 ;
}

// OG
void joystick_shut()
{
}

