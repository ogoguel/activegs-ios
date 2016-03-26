/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_RCSID_C
const char rcsid_protos_h[] = "@(#)$KmKId: protos.h,v 1.188 2004-12-06 19:08:34-05 kentd Exp $";
#endif

/* xdriver.c and macdriver.c and windriver.c */
int x_show_alert(int is_fatal, const char *str);
int win_nonblock_read_stdin(int fd, char *bufptr, int len);
void x_dialog_create_kegs_conf(const char *str);
void update_color_array(int col_num, int a2_color);
void set_border_color(int val);
void x_update_physical_colormap(void);
void x_show_color_array(void);
void x_auto_repeat_on(int must);
void install_text_colormap(void);
void set_border_color(int val);
void draw_status(int line_num, const char *string1);
void x_video_shut(void);
void x_video_init(void);
void x_update_color(int col_num, int red, int green, int blue, word32 rgb);
void redraw_border(void);
void check_input_events(void);
void x_redraw_status_lines(void);
void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height);
void x_push_done();
void x_hide_pointer(int);
void x_get_kimage(Kimage *kimage_ptr);
void x_full_screen(int do_full);

/* test65.c */
void do_gen_test(int got_num, int base_seed);


/* engine.s and engine_c.c */
void fixed_memory_ptrs_init();
void fixed_memory_ptrs_shut();	// OG Added shut to smoothly free up allocated memory
word32 get_itimer(void);

word32 get_memory_c(word32 addr, int cycs);
word32 get_memory16_c(word32 addr, int cycs);
word32 get_memory24_c(word32 addr, int cycs);

int get_memory_asm(word32 addr, int cycs);
int get_memory16_asm(word32 addr, int cycs);
int get_memory16_asm_fail(word32 addr, int cycs);
int get_memory_act_stub_asm(word32 addr, int cycs);
int get_memory16_act_stub_asm(word32 addr, int cycs);

void set_memory_c(word32 addr, word32 val, int cycs);
void set_memory16_c(word32 addr, word32 val, int cycs);
void set_memory24_c(word32 addr, word32 val, int cycs);

int enter_engine(Engine_reg *ptr);
void clr_halt_act(void);
void set_halt_act(int val);

/* special scc_macdriver.c prototypes */
int scc_serial_mac_init(int port);
void scc_serial_mac_change_params(int port);
void scc_serial_mac_fill_readbuf(int port, int space_left, double dcycs);
void scc_serial_mac_empty_writebuf(int port);

/* special scc_windriver.c prototypes */
int scc_serial_win_init(int port);
void scc_serial_win_change_params(int port);
void scc_serial_win_fill_readbuf(int port, int space_left, double dcycs);
void scc_serial_win_empty_writebuf(int port);

/* special joystick_driver.c prototypes */
void joystick_init(void);
void joystick_update(double dcycs);
void joystick_update_buttons(void);


/* END_HDR */

/* adb.c */
void adb_init(void);
void adb_shut(); // OG Added adb_shut()
void adb_reset(void);
void adb_log(word32 addr, int val);
void show_adb_log(void);
void adb_error(void);
void adb_add_kbd_srq(void);
void adb_clear_kbd_srq(void);
void adb_add_data_int(void);
void adb_add_mouse_int(void);
void adb_clear_data_int(void);
void adb_clear_mouse_int(void);
void adb_send_bytes(int num_bytes, word32 val0, word32 val1, word32 val2);
void adb_send_1byte(word32 val);
void adb_response_packet(int num_bytes, word32 val);
void adb_kbd_reg0_data(int a2code, int is_up);
void adb_kbd_talk_reg0(void);
void adb_set_config(word32 val0, word32 val1, word32 val2);
void adb_set_new_mode(word32 val);
int adb_read_c026(void);
void adb_write_c026(int val);
void do_adb_cmd(void);
int adb_read_c027(void);
void adb_write_c027(int val);
int read_adb_ram(word32 addr);
void write_adb_ram(word32 addr, int val);
int adb_get_keypad_xy(int get_y);
int update_mouse(int x, int y, int button_states, int buttons_valid);
int mouse_read_c024(double dcycs);
void mouse_compress_fifo(double dcycs);
void adb_key_event(int a2code, int is_up);
word32 adb_read_c000(void);
word32 adb_access_c010(void);
word32 adb_read_c025(void);
int adb_is_cmd_key_down(void);
int adb_is_option_key_down(void);
void adb_increment_speed(void);
void adb_physical_key_update(int a2code, int is_up);
void adb_virtual_key_update(int a2code, int is_up);
void adb_all_keys_up(void);
void adb_kbd_repeat_off(void);


/* clock.c */
double get_dtime(void);
int micro_sleep(double dtime);
void clk_bram_zero(void);
void clk_bram_set(int bram_num, int offset, int val);
void clk_setup_bram_version(void);
void clk_write_bram(FILE *fconf);
void update_cur_time(void);
void clock_update(void);
void clock_update_if_needed(void);
void clock_write_c034(word32 val);
void do_clock_data(void);


/* compile_time.c */




/* dis.c */
int get_num(void);
void debugger_help(void);
void do_debug_intfc(void);
word32 dis_get_memory_ptr(word32 addr);
void show_one_toolset(FILE *toolfile, int toolnum, word32 addr);
void show_toolset_tables(word32 a2bank, word32 addr);


enum mode_breakpoint
	{
		BRK_DISABLED=0,
		BRK_X = 1,
		BRK_READ=2,
		BRK_WRITE=4,
		BRK_PATCH=8,
		BRK_STOP=16,
		BRK_SLOWSPEED=32,
		BRK_DEFAULTSPEED=64,
		BRK_MESSAGE=128,
		BRK_ALL = BRK_STOP|BRK_X|BRK_READ|BRK_WRITE
	};


void set_bp(word32 _addr,enum mode_breakpoint _mode,const char* _patch,const char* _check);
void show_bp(void);
void delete_bp(word32 addr);
void do_blank(void);
void do_go(void);
void do_step(void);
void xam_mem(int count);
void show_hex_mem(int startbank, word32 start, int endbank, word32 end, int count);
int read_line(char *buf, int len);
void do_debug_list(void);
void dis_do_memmove(void);
void dis_do_pattern_search(void);
void dis_do_compare(void);
void do_debug_unix(void);
void do_debug_load(void);
int do_dis(FILE *outfile, word32 kpc, int accsize, int xsize, int op_provided, word32 instr);
void show_line(FILE *outfile, word32 kaddr, word32 operand, int size, char *string);
void halt_printf(const char *fmt, ...);
void halt2_printf(const char *fmt, ...);


/* scc.c */
void scc_init(void);
void scc_reset(void);
void scc_hard_reset_port(int port);
void scc_reset_port(int port);
void scc_regen_clocks(int port);
void scc_port_init(int port);
void scc_try_to_empty_writebuf(int port, double dcycs);
void scc_try_fill_readbuf(int port, double dcycs);
void scc_update(double dcycs);
void do_scc_event(int type, double dcycs);
void show_scc_state(void);
void scc_log(int regnum, word32 val, double dcycs);
void show_scc_log(void);
word32 scc_read_reg(int port, double dcycs);
void scc_write_reg(int port, word32 val, double dcycs);
void scc_maybe_br_event(int port, double dcycs);
void scc_evaluate_ints(int port);
void scc_maybe_rx_event(int port, double dcycs);
void scc_maybe_rx_int(int port, double dcycs);
void scc_clr_rx_int(int port);
void scc_handle_tx_event(int port, double dcycs);
void scc_maybe_tx_event(int port, double dcycs);
void scc_clr_tx_int(int port);
void scc_set_zerocnt_int(int port);
void scc_clr_zerocnt_int(int port);
void scc_add_to_readbuf(int port, word32 val, double dcycs);
void scc_add_to_readbufv(int port, double dcycs, const char *fmt, ...);
void scc_transmit(int port, word32 val, double dcycs);
void scc_add_to_writebuf(int port, word32 val, double dcycs);
word32 scc_read_data(int port, double dcycs);
void scc_write_data(int port, word32 val, double dcycs);


/* scc_socket_driver.c */
void scc_socket_init(int port);
void scc_socket_maybe_open_incoming(int port, double dcycs);
void scc_socket_open_outgoing(int port, double dcycs);
void scc_socket_make_nonblock(int port, double dcycs);
void scc_socket_change_params(int port);
void scc_socket_close(int port, int full_close, double dcycs);
void scc_accept_socket(int port, double dcycs);
void scc_socket_telnet_reqs(int port, double dcycs);
void scc_socket_fill_readbuf(int port, int space_left, double dcycs);
void scc_socket_recvd_char(int port, int c, double dcycs);
void scc_socket_empty_writebuf(int port, double dcycs);
void scc_socket_modem_write(int port, int c, double dcycs);
void scc_socket_do_cmd_str(int port, double dcycs);
void scc_socket_send_modem_code(int port, int code, double dcycs);
void scc_socket_modem_hangup(int port, double dcycs);
void scc_socket_modem_connect(int port, double dcycs);
void scc_socket_modem_do_ring(int port, double dcycs);
void scc_socket_do_answer(int port, double dcycs);


/* scc_windriver.c */


/* scc_macdriver.c */


/* iwm.c */


/* joystick_driver.c */


/* moremem.c */
void moremem_init(); // OG Added moremem_init()
void fixup_brks(void);
void fixup_hires_on(void);
void fixup_bank0_2000_4000(void);
void fixup_bank0_0400_0800(void);
void fixup_any_bank_any_page(int start_page, int num_pages, byte *mem0rd, byte *mem0wr);
void fixup_intcx(void);
void fixup_wrdefram(int new_wrdefram);
void fixup_st80col(double dcycs);
void fixup_altzp(void);
void fixup_page2(double dcycs);
void fixup_ramrd(void);
void fixup_ramwrt(void);
void fixup_lcbank2(void);
void fixup_rdrom(void);
void set_statereg(double dcycs, int val);
void fixup_shadow_txt1(void);
void fixup_shadow_txt2(void);
void fixup_shadow_hires1(void);
void fixup_shadow_hires2(void);
void fixup_shadow_shr(void);
void fixup_shadow_iolc(void);
void update_shadow_reg(int val);
void fixup_shadow_all_banks(void);
void setup_pageinfo(void);
void show_bankptrs_bank0rdwr(void);
void show_bankptrs(int bnk);
void show_addr(byte *ptr);
int io_read(word32 loc, double *cyc_ptr);
void io_write(word32 loc, int val, double *cyc_ptr);
word32 get_lines_since_vbl(double dcycs);
int in_vblank(double dcycs);
int read_vid_counters(int loc, double dcycs);


/* paddles.c */
void paddle_fixup_joystick_type(void);
void paddle_trigger(double dcycs);
void paddle_trigger_mouse(double dcycs);
void paddle_trigger_none(double dcycs);
void paddle_trigger_keypad(double dcycs);
void paddle_update_trigger_dcycs(double dcycs);
int read_paddles(double dcycs, int paddle);
void paddle_update_buttons(void);


/* sim65816.c */
void show_pc_log(void);
word32 toolbox_debug_4byte(word32 addr);
void toolbox_debug_c(word32 xreg, word32 stack, double *cyc_ptr);
void show_toolbox_log(void);
word32 get_memory_io(word32 loc, double *cyc_ptr);
void set_memory_io(word32 loc, int val, double *cyc_ptr);
void show_regs_act(Engine_reg *eptr);
void show_regs(void);
void my_exit(int ret);
void do_reset(void);
void check_engine_asm_defines(void);
byte *memalloc_align(int size, int skip_amt, void **alloc_ptr);
void memory_ptr_init(void);
void memory_ptr_shut(void);	// OG Added shut
int kegsmain(int argc, char **argv);
void load_roms_init_memory(void);
void load_roms_shut_memory(void);	// OG Added shut
void kegs_expand_path(char *out_ptr, const char *in_ptr, int maxlen);
void setup_kegs_file(char *outname, int maxlen, int ok_if_missing, int can_create_file, const char **name_ptr);
void initialize_events(void);
void check_for_one_event_type(int type);
void add_event_entry(double dcycs, int type);
double remove_event_entry(int type);
void add_event_stop(double dcycs);
void add_event_doc(double dcycs, int osc);
void add_event_scc(double dcycs, int type);
void add_event_vbl(void);
void add_event_vid_upd(int line);
double remove_event_doc(int osc);
double remove_event_scc(int type);
void show_all_events(void);
void show_pmhz(void);
void setup_zip_speeds(void);
void run_prog(void);
void add_irq(word32 irq_mask);
void remove_irq(word32 irq_mask);
void take_irq(int is_it_brk);
void show_dtime_array(void);
void update_60hz(double dcycs, double dtime_now);
void do_vbl_int(void);
void do_scan_int(double dcycs, int line);
void check_scan_line_int(double dcycs, int cur_video_line);
void check_for_new_scan_int(double dcycs);
void init_reg(void);
void handle_action(word32 ret);
void do_break(word32 ret);
void do_cop(word32 ret);
void do_wdm(word32 arg);
void do_wai(void);
void do_stp(void);
void size_fail(int val, word32 v1, word32 v2);
int fatal_printf(const char *fmt, ...);
int kegs_vprintf(const char *fmt, va_list ap);
void must_write(int fd, char *bufptr, int len);
void clear_fatal_logs(void);
char *kegs_malloc_str(const char *in_str);


/* smartport.c */
void smartport_error(void);
void smartport_log(word32 start_addr, int cmd, int rts_addr, int cmd_list);
void do_c70d(word32 arg0);
void do_c70a(word32 arg0);
int do_read_c7(int unit_num, word32 buf, int blk);
int do_write_c7(int unit_num, word32 buf, int blk);
int do_format_c7(int unit_num);
void do_c700(word32 ret);


/* sound.c */
void doc_log_rout(char *msg, int osc, double dsamps, int etc);
void show_doc_log(void);
void sound_init(void);
void sound_init_general(void);
void parent_sound_get_sample_rate(int read_fd);
void set_audio_rate(int rate);
void sound_reset(double dcycs);
void sound_shutdown(void);
void sound_update(double dcycs);
void open_sound_file(void);
void close_sound_file(void);
void check_for_range(word32 *addr, int num_samps, int offset);
void send_sound_to_file(word32 *addr, int shm_pos, int num_samps);
void send_sound(int real_samps, int size);
void show_c030_state(void);
void show_c030_samps(int *outptr, int num);
void sound_play(double dsamps);
void doc_handle_event(int osc, double dcycs);
void doc_sound_end(int osc, int can_repeat, double eff_dsamps, double dsamps);
void add_sound_irq(int osc);
void remove_sound_irq(int osc, int must);
void start_sound(int osc, double eff_dsamps, double dsamps);
void wave_end_estimate(int osc, double eff_dsamps, double dsamps);
void remove_sound_event(int osc);
void doc_write_ctl_reg(int osc, int val, double dsamps);
void doc_recalc_sound_parms(int osc, double eff_dcycs, double dsamps);
int doc_read_c030(double dcycs);
int doc_read_c03c(double dcycs);
int doc_read_c03d(double dcycs);
void doc_write_c03c(int val, double dcycs);
void doc_write_c03d(int val, double dcycs);
void doc_show_ensoniq_state(int osc);


/* sound_driver.c */
void reliable_buf_write(word32 *shm_addr, int pos, int size);
void reliable_zero_write(int amt);
void child_sound_loop(int read_fd, int write_fd, word32 *shm_addr);
void child_sound_playit(word32 tmp);


/* video.c */
void video_init(void);
void show_a2_line_stuff(void);
void video_reset(void);
void video_update(void);
int video_all_stat_to_line_stat(int line, int new_all_stat);
int *video_update_kimage_ptr(int line, int new_stat);
void change_a2vid_palette(int new_palette);
void check_a2vid_palette(void);
void change_display_mode(double dcycs);
void video_update_all_stat_through_line(int line);
void change_border_color(double dcycs, int val);
void update_border_info(void);
void update_border_line(int st_line_offset, int end_line_offset, int color);
void video_border_pixel_write(Kimage *kimage_ptr, int starty, int num_lines, word32 val, int st_off, int end_off);
void redraw_changed_text_40(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int altcharset, int bg_val, int fg_val, int pixels_per_line);
void redraw_changed_text_80(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int altcharset, int bg_val, int fg_val, int pixels_per_line);
void redraw_changed_gr(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_dbl_gr(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_hires(int start_offset, int start_line, int num_lines, int color, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_hires_bw(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_hires_color(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_dbl_hires(int start_offset, int start_line, int num_lines, int color, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_dbl_hires_bw(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int pixels_per_line);
void redraw_changed_dbl_hires_color(int start_offset, int start_line, int num_lines, int reparse, byte *screen_data, int pixels_per_line);
int video_rebuild_super_hires_palette(word32 scan_info, int line, int reparse);
void redraw_changed_super_hires(int start_offset, int start_line, int num_lines, int in_reparse, byte *screen_data);
void display_screen(void);
void video_update_event_line(int line);
void video_check_input_events(void);
void video_update_through_line(int line);
void video_refresh_lines(int st_line, int num_lines, int must_reparse);
void refresh_border(void);
void end_screen(void);
void read_a2_font(void);
void video_get_kimage(Kimage *kimage_ptr, int extend_info, int depth, int mdepth);
void video_get_kimages(void);
void video_convert_kimage_depth(Kimage *kim_in, Kimage *kim_out, int startx, int starty, int width, int height);
void video_push_lines(Kimage *kimage_ptr, int start_line, int end_line, int left_pix, int right_pix);
void video_push_border_sides_lines(int src_x, int dest_x, int width, int start_line, int end_line);
void video_push_border_sides(void);
void video_push_border_special(void);
void video_push_kimages(void);
void video_update_color_raw(int col_num, int a2_color);
void video_update_color_array(int col_num, int a2_color);
void video_update_colormap(void);
void video_update_status_line(int line, const char *string);
void video_show_debug_info(void);
word32 float_bus(double dcycs);

