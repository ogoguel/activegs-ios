/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

int win_update_mouse(int x, int y, int button_states, int buttons_valid);
void win_event_mouse(int umsg,WPARAM wParam, LPARAM lParam);
void win_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags);
void win_event_quit(HWND hwnd);
void window_needs_full_refresh();
LRESULT CALLBACK win_event_handler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
int main(int argc, char **argv);
void check_input_events(void);
void x_update_color(int col_num, int red, int green, int blue, word32 rgb);
void x_update_physical_colormap(void);
void show_xcolor_array(void);
void x_driver_end(void);
void x_get_kimage(Kimage *kimage_ptr);
//void x_dev_video_init(void);
void x_redraw_status_lines(void);
void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height);
void x_push_done(void);
void x_auto_repeat_on(int must);
void x_auto_repeat_off(int must);
void x_hide_pointer(int do_hide);

