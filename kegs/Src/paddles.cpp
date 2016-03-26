/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "adb.h"
#include "moremem.h"
#include "paddles.h"
#include "sim65816.h"

s_paddles g_paddles;

int	g_invert_paddles = 0;
int	g_swap_paddles = 0;

int	g_joystick_type = JOYSTICK_TYPE_NATIVE_1;	// OG Trying to set native joystick as default	
int	g_joystick_native_type1 = -1;
int	g_joystick_native_type2 = -1;
int	g_joystick_native_type = -1;


void
paddle_fixup_joystick_type()
{
	/* If g_paddles.g_joystick_type points to an illegal value, change it */
	if(g_joystick_type == JOYSTICK_TYPE_NATIVE_1) {
		g_joystick_native_type = g_joystick_native_type1;
		if(g_joystick_native_type1 < 0) {
			g_joystick_type = JOYSTICK_TYPE_KEYPAD;
			printf("no joy 1 --- switching to keypad\n");
		}
	}
	if(g_joystick_type == JOYSTICK_TYPE_NATIVE_2) {
		g_joystick_native_type = g_joystick_native_type2;
		if(g_joystick_native_type2 < 0) {
			g_joystick_type = JOYSTICK_TYPE_KEYPAD;
			printf("no joy 2 --- switching to keypad\n");
		}
	}
}

void
paddle_trigger(double dcycs)
{
	/* Called by read/write to $c070 */
	g_paddles.g_paddle_trig_dcycs = dcycs;

	/* Determine what all the paddle values are right now */
	paddle_fixup_joystick_type();

	switch(g_joystick_type) {
	case JOYSTICK_TYPE_KEYPAD:		/* Keypad Joystick */
		paddle_trigger_keypad(dcycs);
		break;
	case JOYSTICK_TYPE_MOUSE:		/* Mouse Joystick */
		paddle_trigger_mouse(dcycs);
		break;
	case JOYSTICK_TYPE_NONE:		/* None Joystick */
		paddle_trigger_none(dcycs);
		break;
#if defined(DRIVER_IOS) && defined(ACTIVEGS) && !defined(ACTIVEGSKARATEKA)
    case JOYSTICK_TYPE_ICADE:		/* Mouse Joystick */
        {
            extern void paddle_trigger_icade(double);
            paddle_trigger_icade(dcycs);
        }
        break;
#endif
	default:
		joystick_update(dcycs);
	}
}

void
paddle_trigger_mouse(double dcycs)
{
	int	val_x, val_y;
	int	mouse_x, mouse_y;

	val_x = 0;

	mouse_x = g_adb.g_mouse_raw_x;
	mouse_y = g_adb.g_mouse_raw_y;
	/* mous_phys_x is 0->560, convert that to -32768 to + 32767 cyc */
	/*  So subtract 280 then mult by 117 */
	val_x = (mouse_x - 280) * 117;

	/* mous_phys_y is 0->384, convert that to -32768 to + 32767 cyc */
	/*  so subtract 192 then mult by 180 to overscale it a bit */
	val_y = (mouse_y - 192) * 180;

	g_paddles.g_paddle_val[0] = val_x;
	g_paddles.g_paddle_val[1] = val_y;
	g_paddles.g_paddle_val[2] = 32767;
	g_paddles.g_paddle_val[3] = 32767;
	g_moremem.g_paddle_buttons |= 0xc;
	paddle_update_trigger_dcycs(dcycs);
}

void
paddle_trigger_none(double dcycs)
{
	g_paddles.g_paddle_val[0] = 32767;
	g_paddles.g_paddle_val[1] = 32767;
	g_paddles.g_paddle_val[2] = 32767;
	g_paddles.g_paddle_val[3] = 32767;
	g_moremem.g_paddle_buttons |= 0xc;
	paddle_update_trigger_dcycs(dcycs);
}

void
paddle_trigger_keypad(double dcycs)
{
	int	get_y;
	int	val_x, val_y;
    
	val_x = adb_get_keypad_xy(get_y=0);
	val_y = adb_get_keypad_xy(get_y=1);
	/* val_x and val_y are already scale -32768 to +32768 */
    
	g_paddles.g_paddle_val[0] = val_x;
	g_paddles.g_paddle_val[1] = val_y;
	g_paddles.g_paddle_val[2] = 32767;
	g_paddles.g_paddle_val[3] = 32767;
	g_moremem.g_paddle_buttons |= 0xc;
	paddle_update_trigger_dcycs(dcycs);
}

void
paddle_update_trigger_dcycs(double dcycs)
{
	double	trig_dcycs;
	int	val;
	int	paddle_num;
	int	scale, trim;
	int	i;

	for(i = 0; i < 4; i++) {
		paddle_num = i;
		if(g_swap_paddles) {
			paddle_num = i ^ 1;
		}
		val = g_paddles.g_paddle_val[paddle_num];
		if(g_invert_paddles) {
			val = -val;
		}

		/* convert -32768 to +32768 into 0->2816.0 cycles (the */
		/* paddle delay const) */
		/* First multiply by the scale factor to adjust range */
		if(paddle_num & 1) {
			scale = g_paddles.g_joystick_scale_factor_y;
			trim = g_paddles.g_joystick_trim_amount_y;
		} else {
			scale = g_paddles.g_joystick_scale_factor_x;
			trim = g_paddles.g_joystick_trim_amount_x;
		}
#if 0
		if(i == 0) {
			printf("val was %04x(%d) * scale %03x = %d\n",
				val, val, scale, (val*scale)>>16);
		}
#endif
		val = val * scale >> 16;
		/* Val is now from -128 to + 128 since scale is */
		/*  256=1.0, 128 = 0.5 */
		val = val + 128 + trim;
		if(val >= 255) {
			val = 280;	/* increase range */
		}


	// OG  Try to make the joystick up and running, even at fast speed (this might be turned into a user option?)	
	{
		double facteur = 11.04;

		if (g_sim65816.g_speed_fast)
		{
			if (g_sim65816.get_limit_speed() == SPEED_ZIP) 
					facteur /= g_sim65816.g_zip_pmhz / 2.6;		
			else
			if (g_sim65816.get_limit_speed() == SPEED_UNLIMITED)
				facteur /= g_sim65816.g_projected_pmhz / 2.6;		
		}
		else
		{
			// OG Hack to make the Joystick works with Miniprix (to investigate why it is needed...)
			if (g_sim65816.engine.kpc == 0x5A02)
				facteur /= 2.6;
		}

		/* convert 0->255 into 0->2816.0 cycles (the paddle delay const) */
			trig_dcycs = dcycs + (val * facteur);
		//	trig_dcycs = dcycs + (val * 11.04);	
		g_paddles.g_paddle_dcycs[i] = trig_dcycs;
		}
	}
}

int
read_paddles(double dcycs, int paddle)
{
	double	trig_dcycs;

	trig_dcycs = g_paddles.g_paddle_dcycs[paddle & 3];

	if(dcycs < trig_dcycs) {
		return 0x80;
	} else {
		return 0x00;
	}
}

void
paddle_update_buttons()
{
	paddle_fixup_joystick_type();
	joystick_update_buttons();
}
