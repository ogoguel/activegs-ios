/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

// OG Added define for joystick
#define JOYSTICK_TYPE_KEYPAD	1000
#define JOYSTICK_TYPE_MOUSE		1001
#define JOYSTICK_TYPE_NATIVE_1	1002
#define JOYSTICK_TYPE_NATIVE_2	1003
#define JOYSTICK_TYPE_NONE		1004	// OG Added Joystick None
#define JOYSTICK_TYPE_ICADE		1005	// OG Added Joystick Icade

//#define NB_JOYSTICK_TYPE 5
#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

class s_paddles : public serialized
{
public:


	double	g_paddle_trig_dcycs;

	int	g_joystick_scale_factor_x ;
	int	g_joystick_scale_factor_y ;
	int	g_joystick_trim_amount_x ;
	int	g_joystick_trim_amount_y ;


	int	g_paddle_val[4] ;
			/* g_paddle_val[0]: Joystick X coord, [1]:Y coord */

	double	g_paddle_dcycs[4] ;
			/* g_paddle_dcycs are the dcycs the paddle goes to 0 */

	s_paddles()
	{
		INIT_SERIALIZED();

		g_joystick_scale_factor_x = 0x100;
		g_joystick_scale_factor_y = 0x100;
	
	}

	DEFAULT_SERIALIZE_IN_OUT ;

} XCODE_PRAGMA_PACK;

#ifdef _WIN32
#pragma pack(pop)
#endif

extern s_paddles g_paddles;

extern	int	g_joystick_type ;	// OG Trying to set native joystick as default	
extern	int	g_joystick_native_type1;
extern	int	g_joystick_native_type2;
extern	int	g_joystick_native_type ;
