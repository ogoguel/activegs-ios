/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

const char rcsid_adb_h[] = "@(#)$KmKId: adb.h,v 1.11 2004-10-13 23:30:33-04 kentd Exp $";

#include "defc.h"

enum {
	ADB_IDLE = 0,
	ADB_IN_CMD,
	ADB_SENDING_DATA,
};

#define ADB_C027_MOUSE_DATA	0x80
#define ADB_C027_MOUSE_INT	0x40
#define ADB_C027_DATA_VALID	0x20
#define ADB_C027_DATA_INT	0x10
#define ADB_C027_KBD_VALID	0x08
#define ADB_C027_KBD_INT	0x04
#define ADB_C027_MOUSE_COORD	0x02
#define ADB_C027_CMD_FULL	0x01

#define ADB_C027_NEG_MASK	( ~ (				\
	 	ADB_C027_MOUSE_DATA | ADB_C027_DATA_VALID |	\
		ADB_C027_KBD_VALID | ADB_C027_MOUSE_COORD |	\
		ADB_C027_CMD_FULL))


#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif


STRUCT(Mouse_fifo)
{
	_ALIGNED(8) double	dcycs;
	int	x;
	int	y;
	int	buttons;
}
XCODE_PRAGMA_PACK
;

enum warpmode
{
	WARP_NONE = 0,
	WARP_POINTER,
	WARP_TOUCHSCREEN 
};


class s_adb : public serialized
{
public:
	int		fullscreen ;
	int		halt_on_all_c027;
	word32	g_adb_repeat_delay;
	word32	g_adb_repeat_rate;
	word32	g_adb_repeat_info;
	word32	g_adb_char_set ;
	word32	g_adb_layout_lang;

	word32	g_adb_interrupt_byte;
	int		g_adb_state;

	word32	g_adb_cmd ;
	int		g_adb_cmd_len;
	int		g_adb_cmd_so_far ;
	word32	g_adb_cmd_data[16];

#define MAX_ADB_DATA_PEND	16

	word32	g_adb_data[MAX_ADB_DATA_PEND];
	int		g_adb_data_pending;

	word32	g_c027_val;
	word32	g_c025_val;

	byte	adb_memory[256];

	word32	g_adb_mode ;		/* mode set via set_modes, clear_modes */

	enum warpmode		g_warp_pointer;
	int		g_hide_pointer;
	int		g_unhide_pointer;

	int		g_mouse_a2_x ;
	int		g_mouse_a2_y ;
	int		g_mouse_a2_button ;
	int		g_mouse_fifo_pos ;
	int		g_mouse_raw_x ;
	int		g_mouse_raw_y;

#define ADB_MOUSE_FIFO		8

	Mouse_fifo g_mouse_fifo[ADB_MOUSE_FIFO] ;

	int		g_mouse_warp_x ;
	int		g_mouse_warp_y ;

	int		g_adb_mouse_valid_data ;
	int		g_adb_mouse_coord ;

#define MAX_KBD_BUF		8

	int		g_key_down ;
	int		g_hard_key_down ;
	int		g_a2code_down ;
	int		g_kbd_read_no_update ;
	int		g_kbd_chars_buffered ;
	int		g_kbd_buf[MAX_KBD_BUF];

	word32	g_adb_repeat_vbl ;

	int		g_kbd_dev_addr;		/* ADB physical kbd addr */
	int		g_mouse_dev_addr ;		/* ADB physical mouse addr */

	int		g_kbd_ctl_addr;		/* ADB microcontroller's kbd addr */
	int		g_mouse_ctl_addr;		/* ADB ucontroller's mouse addr*/
			/* above are ucontroller's VIEW of where mouse/kbd */
			/*  are...if they are moved, mouse/keyboard funcs */
			/*  should stop (c025, c000, c024, etc). */

	word32	g_virtual_key_up[4];	/* bitmask of all possible 128 a2codes */
				/* indicates which keys are up=1 by bit */

	int		g_keypad_key_is_down[10] ;/* List from 0-9 of which keypad */
					/*  keys are currently pressed */

#define MAX_ADB_KBD_REG3	16

	int		g_kbd_reg0_pos;
	int		g_kbd_reg0_data[MAX_ADB_KBD_REG3];
	int		g_kbd_reg3_16bit ;			/* also set in adb_reset()! */


	int		g_adb_init;
	word32	g_last_c027_read; // OG to monitor if mouse is being used!

	int		g_simulate_space;	// OG Mouse button can simulate space key
	int g_need_to_decrement_fifo;

    s_adb()
    {
        init();
    }
	void init()
	{
		INIT_SERIALIZED();

		g_adb_repeat_delay = 45;
		g_adb_repeat_rate = 3;
		g_adb_repeat_info = 0x23;
		g_adb_state = ADB_IDLE;

		g_adb_cmd = (word32)-1;
		g_kbd_dev_addr = 2;		/* ADB physical kbd addr */
		g_mouse_dev_addr = 3;		/* ADB physical mouse addr */

		g_kbd_ctl_addr = 2;		/* ADB microcontroller's kbd addr */
		g_mouse_ctl_addr = 3;		/* ADB ucontroller's mouse addr*/
			/* above are ucontroller's VIEW of where mouse/kbd */
			/*  are...if they are moved, mouse/keyboard funcs */
			/*  should stop (c025, c000, c024, etc). */

		g_kbd_reg3_16bit = 0x602;			/* also set in adb_reset()! */
		g_simulate_space = 1;	// OG Mouse button can simulate space key
		g_warp_pointer = WARP_NONE;
	}
	
	DEFAULT_SERIALIZE_IN_OUT ;

}
XCODE_PRAGMA_PACK
;

#ifdef _WIN32
#pragma pack(pop)
#endif

extern s_adb g_adb;
