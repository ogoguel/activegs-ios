/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

void show_simple_alert(const char *str1, const char *str2, const char *str3, int num);
void x_dialog_create_kegs_conf(const char *str);
int x_show_alert(int is_fatal, const char *str);void update_window(void);

//void mac_update_modifiers(word32 state);
void mac_warp_mouse(void);
void check_input_events(void);
void temp_run_application_event_loop(void);
int main(int argc, char *argv[]);
void x_update_color(int col_num, int red, int green, int blue, word32 rgb);
void x_update_physical_colormap(void);
void x_show_color_array(void);


void x_get_kimage(Kimage *kimage_ptr);
//void x_dev_video_init(void);
void x_redraw_status_lines(void);
void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height);
void x_push_done(void);
void x_auto_repeat_on(int must);
void x_auto_repeat_off(int must);
void x_hide_pointer(int do_hide);
void x_full_screen(int do_full);
void update_main_window_size(void);

#ifndef __EVENTS__
typedef unsigned short                          EventModifiers;
enum {
	/* modifiers */
	activeFlagBit                 = 0,    /* activate? (activateEvt and mouseDown)*/
	btnStateBit                   = 7,    /* state of button?*/
	cmdKeyBit                     = 8,    /* command key down?*/
	shiftKeyBit                   = 9,    /* shift key down?*/
	alphaLockBit                  = 10,   /* alpha lock down?*/
	optionKeyBit                  = 11,   /* option key down?*/
	controlKeyBit                 = 12,   /* control key down?*/
	rightShiftKeyBit              = 13,   /* right shift key down? Not supported on Mac OS X.*/
	rightOptionKeyBit             = 14,   /* right Option key down? Not supported on Mac OS X.*/
	rightControlKeyBit            = 15    /* right Control key down? Not supported on Mac OS X.*/
};
enum {
	activeFlag                    = 1 << activeFlagBit,
	btnState                      = 1 << btnStateBit,
	cmdKey                        = 1 << cmdKeyBit,
	shiftKey                      = 1 << shiftKeyBit,
	alphaLock                     = 1 << alphaLockBit,
	optionKey                     = 1 << optionKeyBit,
	controlKey                    = 1 << controlKeyBit,
	rightShiftKey                 = 1 << rightShiftKeyBit, /* Not supported on Mac OS X.*/
	rightOptionKey                = 1 << rightOptionKeyBit, /* Not supported on Mac OS X.*/
	rightControlKey               = 1 << rightControlKeyBit /* Not supported on Mac OS X.*/
};
#endif