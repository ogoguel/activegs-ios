/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include <time.h>

#include "defc.h"
#include "moremem.h"
#include "sim65816.h"
#include "video.h"
#ifdef ACTIVEGS
#include "graphcounter.h"
#endif

sd_video g_video;
ss_video s_video;

Kimage g_kimage_offscreen;
int g_video_offscreen_has_been_modified = 0 ;

//extern int g_save_cur_a2_stat;

extern int g_config_control_panel;

typedef byte Change;
	word32 g_refresh_bytes_xfer = 0;



char	g_status_buf[MAX_STATUS_LINES][STATUS_LINE_LENGTH + 1];
char	*g_status_ptrs[MAX_STATUS_LINES] = { 0 };

	word32 g_font40_even_bits[0x100][8][16/4];
	word32 g_font40_odd_bits[0x100][8][16/4];
	word32 g_font80_off0_bits[0x100][8][12/4];
	word32 g_font80_off1_bits[0x100][8][12/4];
	word32 g_font80_off2_bits[0x100][8][12/4];
	word32 g_font80_off3_bits[0x100][8][12/4];

	word32 g_palette_8to1624[256];
	word32 g_a2palette_8to1624[256];

	int	g_expanded_col_0[16];
	int	g_expanded_col_1[16];
	int	g_expanded_col_2[16];

int g_video_act_margin_left = BASE_MARGIN_LEFT;
int g_video_act_margin_right = BASE_MARGIN_RIGHT;
int g_video_act_margin_top = BASE_MARGIN_TOP;
 int g_video_act_margin_bottom = BASE_MARGIN_BOTTOM;
 int g_video_act_width = X_A2_WINDOW_WIDTH;
 int g_video_act_height = X_A2_WINDOW_HEIGHT;

const int xxxg_dbhires_colors[] = {
		/* rgb */
		0x000,		/* 0x0 black */
		0xd03,		/* 0x1 deep red */
		0x852,		/* 0x2 brown */
		0xf60,		/* 0x3 orange */
		0x070,		/* 0x4 dark green */
		0x555,		/* 0x5 dark gray */
		0x0d0,		/* 0x6 green */
		0xff0,		/* 0x7 yellow */
		0x009,		/* 0x8 dark blue */
		0xd0d,		/* 0x9 purple */
		0xaaa,		/* 0xa light gray */
		0xf98,		/* 0xb pink */
		0x22f,		/* 0xc medium blue */
		0x6af,		/* 0xd light blue */
		0x0f9,		/* 0xe aquamarine */
		0xfff		/* 0xf white */
};

word32 g_dhires_convert[4096];	/* look up table of 7 bits (concat): */
				/* { 4 bits, |3 prev bits| } */

const byte g_dhires_colors_16[] = {
		0x00,	/* 0x0 black */
		0x02,	/* 0x1 dark blue */
		0x04,	/* 0x2 dark green */
		0x06,	/* 0x3 medium blue */
		0x08,	/* 0x4 brown */
		0x0a,	/* 0x5 light gray */
		0x0c,	/* 0x6 green */
		0x0e,	/* 0x7 aquamarine */
		0x01,	/* 0x8 deep red */
		0x03,	/* 0x9 purple */
		0x05,	/* 0xa dark gray */
		0x07,	/* 0xb light blue */
		0x09,	/* 0xc orange */
		0x0b,	/* 0xd pink */
		0x0d,	/* 0xe yellow */
		0x0f/* 0xf white */
};

 int  g_lores_colors[] = {
		/* rgb */
		0x000,		/* 0x0 black */
		0xd03,		/* 0x1 deep red */
		0x009,		/* 0x2 dark blue */
		0xd0d,		/* 0x3 purple */
		0x070,		/* 0x4 dark green */
		0x555,		/* 0x5 dark gray */
		0x22f,		/* 0x6 medium blue */
		0x6af,		/* 0x7 light blue */
		0x852,		/* 0x8 brown */
		0xf60,		/* 0x9 orange */
		0xaaa,		/* 0xa light gray */
		0xf98,		/* 0xb pink */
		0x0d0,		/* 0xc green */
		0xff0,		/* 0xd yellow */
		0x0f9,		/* 0xe aquamarine */
		0xfff		/* 0xf white */
};

const word32 g_bw_hires_convert[4] = {
	BIGEND(0x00000000),
	BIGEND(0x0f0f0000),
	BIGEND(0x00000f0f),
	BIGEND(0x0f0f0f0f)
};

const word32 g_bw_dhires_convert[16] = {
	BIGEND(0x00000000),
	BIGEND(0x0f000000),
	BIGEND(0x000f0000),
	BIGEND(0x0f0f0000),

	BIGEND(0x00000f00),
	BIGEND(0x0f000f00),
	BIGEND(0x000f0f00),
	BIGEND(0x0f0f0f00),

	BIGEND(0x0000000f),
	BIGEND(0x0f00000f),
	BIGEND(0x000f000f),
	BIGEND(0x0f0f000f),

	BIGEND(0x00000f0f),
	BIGEND(0x0f000f0f),
	BIGEND(0x000f0f0f),
	BIGEND(0x0f0f0f0f),
};

const word32 g_hires_convert[64] = {
	BIGEND(0x00000000),	/* 00,0000 = black, black, black, black */
	BIGEND(0x00000000),	/* 00,0001 = black, black, black, black */
	BIGEND(0x03030000),	/* 00,0010 = purp , purp , black, black */
	BIGEND(0x0f0f0000),	/* 00,0011 = white, white, black, black */
	BIGEND(0x00000c0c),	/* 00,0100 = black, black, green, green */
	BIGEND(0x0c0c0c0c),	/* 00,0101 = green, green, green, green */
	BIGEND(0x0f0f0f0f),	/* 00,0110 = white, white, white, white */
	BIGEND(0x0f0f0f0f),	/* 00,0111 = white, white, white, white */
	BIGEND(0x00000000),	/* 00,1000 = black, black, black, black */
	BIGEND(0x00000000),	/* 00,1001 = black, black, black, black */
	BIGEND(0x03030303),	/* 00,1010 = purp , purp , purp , purp */
	BIGEND(0x0f0f0303),	/* 00,1011 = white ,white, purp , purp */
	BIGEND(0x00000f0f),	/* 00,1100 = black ,black, white, white */
	BIGEND(0x0c0c0f0f),	/* 00,1101 = green ,green, white, white */
	BIGEND(0x0f0f0f0f),	/* 00,1110 = white ,white, white, white */
	BIGEND(0x0f0f0f0f),	/* 00,1111 = white ,white, white, white */

	BIGEND(0x00000000),	/* 01,0000 = black, black, black, black */
	BIGEND(0x00000000),	/* 01,0001 = black, black, black, black */
	BIGEND(0x06060000),	/* 01,0010 = blue , blue , black, black */
	BIGEND(0x0f0f0000),	/* 01,0011 = white, white, black, black */
	BIGEND(0x00000c0c),	/* 01,0100 = black, black, green, green */
	BIGEND(0x09090c0c),	/* 01,0101 = orang, orang, green, green */
	BIGEND(0x0f0f0f0f),	/* 01,0110 = white, white, white, white */
	BIGEND(0x0f0f0f0f),	/* 01,0111 = white, white, white, white */
	BIGEND(0x00000000),	/* 01,1000 = black, black, black, black */
	BIGEND(0x00000000),	/* 01,1001 = black, black, black, black */
	BIGEND(0x06060303),	/* 01,1010 = blue , blue , purp , purp */
	BIGEND(0x0f0f0303),	/* 01,1011 = white ,white, purp , purp */
	BIGEND(0x00000f0f),	/* 01,1100 = black ,black, white, white */
	BIGEND(0x09090f0f),	/* 01,1101 = orang ,orang, white, white */
	BIGEND(0x0f0f0f0f),	/* 01,1110 = white ,white, white, white */
	BIGEND(0x0f0f0f0f),	/* 01,1111 = white ,white, white, white */

	BIGEND(0x00000000),	/* 10,0000 = black, black, black, black */
	BIGEND(0x00000000),	/* 10,0001 = black, black, black, black */
	BIGEND(0x03030000),	/* 10,0010 = purp , purp , black, black */
	BIGEND(0x0f0f0000),	/* 10,0011 = white, white, black, black */
	BIGEND(0x00000909),	/* 10,0100 = black, black, orang, orang */
	BIGEND(0x0c0c0909),	/* 10,0101 = green, green, orang, orang */
	BIGEND(0x0f0f0f0f),	/* 10,0110 = white, white, white, white */
	BIGEND(0x0f0f0f0f),	/* 10,0111 = white, white, white, white */
	BIGEND(0x00000000),	/* 10,1000 = black, black, black, black */
	BIGEND(0x00000000),	/* 10,1001 = black, black, black, black */
	BIGEND(0x03030606),	/* 10,1010 = purp , purp , blue , blue */
	BIGEND(0x0f0f0606),	/* 10,1011 = white ,white, blue , blue */
	BIGEND(0x00000f0f),	/* 10,1100 = black ,black, white, white */
	BIGEND(0x0c0c0f0f),	/* 10,1101 = green ,green, white, white */
	BIGEND(0x0f0f0f0f),	/* 10,1110 = white ,white, white, white */
	BIGEND(0x0f0f0f0f),	/* 10,1111 = white ,white, white, white */

	BIGEND(0x00000000),	/* 11,0000 = black, black, black, black */
	BIGEND(0x00000000),	/* 11,0001 = black, black, black, black */
	BIGEND(0x06060000),	/* 11,0010 = blue , blue , black, black */
	BIGEND(0x0f0f0000),	/* 11,0011 = white, white, black, black */
	BIGEND(0x00000909),	/* 11,0100 = black, black, orang, orang */
	BIGEND(0x09090909),	/* 11,0101 = orang, orang, orang, orang */
	BIGEND(0x0f0f0f0f),	/* 11,0110 = white, white, white, white */
	BIGEND(0x0f0f0f0f),	/* 11,0111 = white, white, white, white */
	BIGEND(0x00000000),	/* 11,1000 = black, black, black, black */
	BIGEND(0x00000000),	/* 11,1001 = black, black, black, black */
	BIGEND(0x06060606),	/* 11,1010 = blue , blue , blue , blue */
	BIGEND(0x0f0f0606),	/* 11,1011 = white ,white, blue , blue */
	BIGEND(0x00000f0f),	/* 11,1100 = black ,black, white, white */
	BIGEND(0x09090f0f),	/* 11,1101 = orang ,orang, white, white */
	BIGEND(0x0f0f0f0f),	/* 11,1110 = white ,white, white, white */
	BIGEND(0x0f0f0f0f),	/* 11,1111 = white ,white, white, white */
};

 int g_screen_index[] = {
		0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380,
		0x028, 0x0a8, 0x128, 0x1a8, 0x228, 0x2a8, 0x328, 0x3a8,
		0x050, 0x0d0, 0x150, 0x1d0, 0x250, 0x2d0, 0x350, 0x3d0
};


void
video_init()
{
	word32	col[4];
	Kimage	*kimage_ptr;
	word32	*ptr;
	word32	val0, val1, val2, val3;
	word32	match_col;
	word32	next_col, next2_col, next3_col;
	word32	val;
	word32	cur_col;
	int	width, height;
	int	total_bytes;
	int	i, j;

// OG Reinit globals
	g_video.g_a2_screen_buffer_changed = (word32)-1;
	g_video.g_full_refresh_needed = (word32)-1;
	g_video.g_cycs_in_40col = 0;
	g_video.g_cycs_in_xredraw = 0;
	
	g_refresh_bytes_xfer = 0;

	g_video.g_video_dcycs_check_input = 0.0;
	//g_video.g_video_extra_check_inputs = 0;	// OG Initialized
	
	g_video.g_need_redraw = 1;
	g_video.g_palette_change_summary = 0;

	g_video.g_border_sides_refresh_needed = 1;
	g_video.g_border_special_refresh_needed = 1;
	g_video.g_border_line24_refresh_needed = 1;
	g_video.g_status_refresh_needed = 1;

	g_video.g_vbl_border_color = 0;
	g_video.g_border_last_vbl_changes = 0;

	g_video.g_use_dhr140 = 0;
//	s_video.g_use_bw_hires = 0;

	g_video.g_new_a2_stat_cur_line = 0;
	g_video.g_vid_update_last_line = 0;

	g_video.g_cur_a2_stat = ALL_STAT_TEXT | ALL_STAT_ANNUNC3 |(0xf << BIT_ALL_STAT_TEXT_COLOR);


	g_video.g_a2vid_palette = 0xe;
	s_video.g_installed_full_superhires_colormap = 0;

	g_video.Max_color_size = 256;

	g_video.g_saved_a2vid_palette = -1;

	g_video.g_cycs_in_refresh_line = 0;
	g_video.g_cycs_in_refresh_ximage = 0;

	g_video.g_num_lines_superhires = 0;
	g_video.g_num_lines_superhires640 = 0;
	g_video.g_num_lines_prev_superhires = 0;
	g_video.g_num_lines_prev_superhires640 = 0;

	g_video.g_flash_count = 0;

	g_video.g_screen_redraw_skip_count = 0;
	//	g_video.g_screen_redraw_skip_amt = -1;

	g_video.g_cycs_in_check_input = 0;	

	g_video.g_needfullrefreshfornextframe = 1 ;	


	g_video.g_num_a2vid_palette_checks = 1;

	g_video.g_border_color = 0;	// OG Expose border color

	g_video.g_num_border_changes = 0;


/* Initialize video system */

	for(i = 0; i < 200; i++) {
		g_video.g_a2_line_kimage[i] = (Kimage *)0; // OG Changed from void* to kimage*
		g_video.g_a2_line_stat[i] = -1;
		g_video.g_a2_line_left_edge[i] = 0;
		g_video.g_a2_line_right_edge[i] = 0;
	}
	for(i = 0; i < 200; i++) {
		g_video.g_a2_new_all_stat[i] = 0;
		g_video.g_a2_cur_all_stat[i] = 1;
		for(j = 0; j < 8; j++) {
			s_video.g_saved_line_palettes[i][j] = (word32)-1;
		}
	}
	for(i = 0; i < 262; i++) {
		g_video.g_cur_border_colors[i] = -1;
	}

	g_video.g_new_a2_stat_cur_line = 0;

	x_video_init();

	read_a2_font();

	vid_printf("Zeroing out video memory\n");

	for(i = 0; i < 8; i++) {
		switch(i) {
		case 0:
			kimage_ptr = &(s_video.g_kimage_text[0]);
			break;
		case 1:
			kimage_ptr = &(s_video.g_kimage_text[1]);
			break;
		case 2:
			kimage_ptr = &(s_video.g_kimage_hires[0]);
			break;
		case 3:
			kimage_ptr = &(s_video.g_kimage_hires[1]);
			break;
		case 4:
			kimage_ptr = &s_video.g_kimage_superhires;
			break;
		case 5:
			kimage_ptr = &s_video.g_kimage_border_sides;
			break;
		case 6:
			kimage_ptr = &s_video.g_kimage_border_special;
			break;
        case 7:
            kimage_ptr = &s_video.g_kimage_border_special2;
            break;

		default:
			printf("i: %d, unknown\n", i);
			x_exit(3);
			break;
		}

		width = kimage_ptr->width_act;
		height = kimage_ptr->height;
		total_bytes = (kimage_ptr->mdepth >> 3) * width * height;
		
		ptr = (word32 *)kimage_ptr->data_ptr;
		for(j = 0; j < total_bytes >> 2; j++) {
			*ptr++ = 0;
		}
		
	}

	for(i = 0; i < SLOW_MEM_CH_SIZE; i++) {
		s_video.slow_mem_changed[i] = (word32)-1;
	}

	/* create g_expanded_col_* */
	for(i = 0; i < 16; i++) {
		val = (g_lores_colors[i] >> 0) & 0xf;
		g_expanded_col_0[i] = val;

		val = (g_lores_colors[i] >> 4) & 0xf;
		g_expanded_col_1[i] = val;

		val = (g_lores_colors[i] >> 8) & 0xf;
		g_expanded_col_2[i] = val;
	}

	/* create g_dhires_convert[] array */
	for(i = 0; i < 4096; i++) {
		/* Convert index bits 11:0 where 3:0 is the previous color */
		/*  and 7:4 is the current color to translate */
		/*  Bit 4 will be the first pixel displayed on the screen */
		match_col = i & 0xf;
		for(j = 0; j < 4; j++) {
			cur_col = (i >> (1 + j)) & 0xf;
			next_col = (i >> (2 + j)) & 0xf;
			next2_col = (i >> (3 + j)) & 0xf;
			next3_col = (i >> (4 + j)) & 0xf;
			cur_col = (((cur_col << 4) + cur_col) >> (3 - j)) & 0xf;

			if((cur_col == 0xf) || (next_col == 0xf) ||
							(next2_col == 0xf) ||
							(next3_col == 0xf)) {
				cur_col = 0xf;
				col[j] = cur_col;
				match_col = cur_col;
			} else if((cur_col == 0) || (next_col == 0) ||
					(next2_col == 0) || (next3_col == 0)) {
				cur_col = 0;
				col[j] = cur_col;
				match_col = cur_col;
			} else {
				col[j] = cur_col;
				match_col = cur_col;
			}
		}
		if(g_video.g_use_dhr140) {
			for(j = 0; j < 4; j++) {
				col[j] = (i >> 4) & 0xf;
			}
		}
		val0 = g_dhires_colors_16[col[0] & 0xf];
		val1 = g_dhires_colors_16[col[1] & 0xf];
		val2 = g_dhires_colors_16[col[2] & 0xf];
		val3 = g_dhires_colors_16[col[3] & 0xf];
#ifdef KEGS_LITTLE_ENDIAN
		val = (val3 << 24) + (val2 << 16) + (val1 << 8) + val0;
#else
		val = (val0 << 24) + (val1 << 16) + (val2 << 8) + val3;
#endif
		g_dhires_convert[i] = val;
	}
//	printf("change_display_mode");
	change_display_mode(g_sim65816.g_cur_dcycs);
//	printf("video_reset");
	video_reset();
//	printf("display_screen");
	display_screen();

	fflush(stdout);
}

void
show_a2_line_stuff()
{
	int	i;

	for(i = 0; i < 200; i++) {
		printf("line: %d: stat: %04x, ptr: %p, "
			"left_edge:%d, right_edge:%d\n",
			i, g_video.g_a2_line_stat[i], g_video.g_a2_line_kimage[i],
			g_video.g_a2_line_left_edge[i],
			g_video.g_a2_line_right_edge[i]);
	}

	printf("new_a2_stat_cur_line: %d, cur_a2_stat:%04x\n",
		g_video.g_new_a2_stat_cur_line, g_video.g_cur_a2_stat);
	for(i = 0; i < 200; i++) {
		printf("cur_all[%d]: %03x new_all: %03x\n", i,
			g_video.g_a2_cur_all_stat[i], g_video.g_a2_new_all_stat[i]);
	}

}

void
video_reset()
{
	int	stat;
	int	i;

	s_video.g_installed_full_superhires_colormap = (s_video.g_screen_depth != 8);
	stat = ALL_STAT_TEXT | ALL_STAT_ANNUNC3 |
					(0xf << BIT_ALL_STAT_TEXT_COLOR);

	if(r_sim65816.get_color_mode()==COLORMODE_BW)
	{
		stat |= ALL_STAT_COLOR_C021;
	}

	if(g_config_control_panel)
	{
		extern	int	g_save_cur_a2_stat;
		/* Don't update cur_a2_stat when in configuration panel */ 
		g_save_cur_a2_stat = stat;
	} else {
		g_video.g_cur_a2_stat = stat;
	}

	g_video.g_palette_change_summary = 0;
	for(i = 0; i < 16; i++) {
		g_video.g_palette_change_cnt[i] = 0;
	}

	/* install_a2vid_colormap(); */
	video_update_colormap();
}


void video_update()
{

		int	did_video;

	// OG g_needfullrefreshfornextframe
	if (g_video.g_needfullrefreshfornextframe)
	{
       
		g_video.g_full_refresh_needed = -1;
		g_video.g_a2_screen_buffer_changed = -1;
		g_video.g_status_refresh_needed = 1;
		g_video.g_border_sides_refresh_needed = 1;
		g_video.g_border_special_refresh_needed = 1;		
		g_video.g_needfullrefreshfornextframe = 0;
	}	

	update_border_info();

	video_check_input_events();

	g_video.g_screen_redraw_skip_count--;
	did_video = 0;
	if(g_video.g_screen_redraw_skip_count < 0) {
		did_video = 1;
		video_update_event_line(262);
		g_video.g_screen_redraw_skip_count = g_video.g_screen_redraw_skip_amt;
	}

	/* update flash */
	g_video.g_flash_count++;
	if(g_video.g_flash_count >= 30) {
		g_video.g_flash_count = 0;
		g_video.g_cur_a2_stat ^= ALL_STAT_FLASH_STATE;
		change_display_mode(g_sim65816.g_cur_dcycs);
	}


	check_a2vid_palette();


	if(did_video) {
		g_video.g_new_a2_stat_cur_line = 0;
		g_video.g_a2_new_all_stat[0] = g_video.g_cur_a2_stat;
		g_video.g_vid_update_last_line = 0;
		video_update_through_line(0);
	}
	
//	refresh_video(0);

	
}


int
video_all_stat_to_line_stat(int line, int new_all_stat)
{
	int	page, color, dbl;
	int	st80, hires, annunc3, mix_t_gr;
	int	altchar, text_color, bg_color, flash_state;
	int	mode;

	st80 = new_all_stat & ALL_STAT_ST80;
	hires = new_all_stat & ALL_STAT_HIRES;
	annunc3 = new_all_stat & ALL_STAT_ANNUNC3;
	mix_t_gr = new_all_stat & ALL_STAT_MIX_T_GR;

	page = EXTRU(new_all_stat, 31 - BIT_ALL_STAT_PAGE2, 1) && !st80;
	color = EXTRU(new_all_stat, 31 - BIT_ALL_STAT_COLOR_C021, 1);
	dbl = EXTRU(new_all_stat, 31 - BIT_ALL_STAT_VID80, 1);

	altchar = 0; text_color = 0; bg_color = 0; flash_state = 0;

	if(new_all_stat & ALL_STAT_SUPER_HIRES) {
		mode = MODE_SUPER_HIRES;
		page = 0; dbl = 0; color = 0;
	} else {
		if(line >= 192) {
			mode = MODE_BORDER;
			page = 0; dbl = 0; color = 0;
		} else if((new_all_stat & ALL_STAT_TEXT) ||
						(line >= 160 && mix_t_gr)) {
			mode = MODE_TEXT;
			color = 0;
			altchar = EXTRU(new_all_stat,
					31 - BIT_ALL_STAT_ALTCHARSET, 1);
			text_color = EXTRU(new_all_stat,
					31 - BIT_ALL_STAT_TEXT_COLOR, 4);
			bg_color = EXTRU(new_all_stat,
					31 - BIT_ALL_STAT_BG_COLOR, 4);
			flash_state = EXTRU(new_all_stat,
					31 - BIT_ALL_STAT_FLASH_STATE, 1);
			if(altchar) {
				/* don't bother flashing if altchar on */
				flash_state = 0;
			}
		} else {
			/* obey the graphics mode */
			dbl = dbl && !annunc3;
			if(hires) {
				color = color | EXTRU(new_all_stat,
					31 - BIT_ALL_STAT_DIS_COLOR_DHIRES, 1);
				mode = MODE_HGR;
			} else {
				mode = MODE_GR;
			}
		}
	}

	return((text_color << 12) + (bg_color << 8) + (altchar << 7) +
		(mode << 4) + (flash_state << 3) + (page << 2) +
		(color << 1) + dbl);
}

int *
video_update_kimage_ptr(int line, int new_stat)
{
	Kimage	*kimage_ptr;
	int	*mode_ptr;
	int	page;
	int	mode;

	page = (new_stat >> 2) & 1;
	mode = (new_stat >> 4) & 7;

	switch(mode) {
	case MODE_TEXT:
	case MODE_GR:
		kimage_ptr = &(s_video.g_kimage_text[page]);
		mode_ptr = &(g_video.g_mode_text[page][0]);
		break;
	case MODE_HGR:
		kimage_ptr = &(s_video.g_kimage_hires[page]);
		mode_ptr = &(g_video.g_mode_hires[page][0]);
		/*  arrange to force superhires reparse since we use the */
		/*    same memory */
		g_video.g_mode_superhires[line] = -1;
		break;
	case MODE_SUPER_HIRES:
		kimage_ptr = &s_video.g_kimage_superhires;
		mode_ptr = &(g_video.g_mode_superhires[0]);
		/*  arrange to force hires reparse since we use the */
		/*    same memory */
		g_video.g_mode_hires[0][line] = -1;
		g_video.g_mode_hires[1][line] = -1;
		break;
	case MODE_BORDER:
		/* Hack: reuse text page last line as the special border */
		kimage_ptr = &s_video.g_kimage_border_special2;
		mode_ptr = &(g_video.g_mode_border[0]);
		break;
	default:
		halt_printf("update_a2_ptrs: mode: %d unknown!\n", mode);
		return &(g_video.g_mode_superhires[0]);
	}

	vid_printf("video_update_kimage_ptr %d=%p\n",line,kimage_ptr);
	
	g_video.g_a2_line_kimage[line] = kimage_ptr;
	return mode_ptr;
}

void
change_a2vid_palette(int new_palette)
{
	int	i;

	for(i = 0; i < 200; i++) {
		g_video.g_mode_text[0][i] = -1;
		g_video.g_mode_text[1][i] = -1;
		g_video.g_mode_hires[0][i] = -1;
		g_video.g_mode_hires[1][i] = -1;
		g_video.g_mode_superhires[i] = -1;
		g_video.g_mode_border[i] = -1;
	}

	printf("Changed a2vid_palette to %x\n", new_palette);

	g_video.g_a2vid_palette = new_palette;
	g_video.g_cur_a2_stat = (g_video.g_cur_a2_stat & (~ALL_STAT_A2VID_PALETTE)) +
			(new_palette << BIT_ALL_STAT_A2VID_PALETTE);
	change_display_mode(g_sim65816.g_cur_dcycs);

	g_video.g_border_sides_refresh_needed = 1;
	g_video.g_border_special_refresh_needed = 1;
	g_video.g_status_refresh_needed = 1;
	g_video.g_palette_change_cnt[new_palette]++;
	g_video.g_border_last_vbl_changes = 1;
	for(i = 0; i < 262; i++) {
		g_video.g_cur_border_colors[i] = -1;
	}
}

void
check_a2vid_palette()
{
	int	sum;
	int	min;
	int	val;
	int	min_pos;
	int	count_cur;
	int	i;

	/* determine if g_video.g_a2vid_palette should change */
	/* This is the palette of least use on superhires so that the */
	/*  borders don't change when all 256 superhires colors are used */

	g_video.g_num_a2vid_palette_checks--;
	if(g_video.g_num_a2vid_palette_checks || s_video.g_installed_full_superhires_colormap){
		return;
	}

	g_video.g_num_a2vid_palette_checks = 60;

	sum = 0;
	min = 0x100000;
	min_pos = -1;
	count_cur = g_video.g_shr_palette_used[g_video.g_a2vid_palette];

	for(i = 0; i < 16; i++) {
		val = g_video.g_shr_palette_used[i];
		g_video.g_shr_palette_used[i] = 0;
		if(val < min) {
			min = val;
			min_pos = i;
		}
		sum += val;
	}

	if(g_video.g_a2vid_palette != min_pos && (count_cur > min)) {
		change_a2vid_palette(min_pos);
	}
}

void
change_display_mode(double dcycs)
{
	int	line, tmp_line;

	line = ((get_lines_since_vbl(dcycs) + 0xff) >> 8);
	if(line < 0) {
		line = 0;
		halt_printf("Line < 0!\n");
	}
	tmp_line = MIN(199, line);

	video_update_all_stat_through_line(tmp_line);

	if(line < 200) {
		g_video.g_a2_new_all_stat[line] = g_video.g_cur_a2_stat;
	}
	/* otherwise, g_video.g_cur_a2_stat is covered at the end of vbl */
}

void
video_update_all_stat_through_line(int line)
{
	int	start_line;
	int	prev_stat;
	int	max_line;
	int	i;

	start_line = g_video.g_new_a2_stat_cur_line;
	prev_stat = g_video.g_a2_new_all_stat[start_line];

	max_line = MIN(199, line);

	for(i = start_line + 1; i <= max_line; i++) {
		g_video.g_a2_new_all_stat[i] = prev_stat;
	}
	g_video.g_new_a2_stat_cur_line = max_line;
}




void
change_border_color(double dcycs, int val)
{
	int	pos;
	
	g_video.g_border_color = val;	// OG Expose border color

	pos = g_video.g_num_border_changes;
	g_video.g_border_changes[pos].fcycs = (float)(dcycs - g_sim65816.g_last_vbl_dcycs);
	g_video.g_border_changes[pos].val = val;
	// OG
	/* 
	static int max=100;
	if (max)
	{
		max--;
		double	dline = g_video.g_border_changes[pos].fcycs / 65.0;
		int new_line = (int)dline;
	
		printf("changing border color (%d) @vbl:%d,line:%d %f(%f)\n",val,g_sim65816.g_vbl_count,new_line,g_sim65816.g_last_vbl_dcycs,g_sim65816.g_cur_dcycs); 
	}
	*/
	pos++;
	g_video.g_num_border_changes = pos;

	if(pos >= MAX_BORDER_CHANGES) {
		halt_printf("num border changes: %d\n", pos);
		g_video.g_num_border_changes = 0;
	}
}

//extern int first;

void
update_border_info()
{
	double	dlines_per_dcyc;
	double	dcycs, dline, dcyc_line_start;
	int	offset;
	int	new_line_offset, last_line_offset;
	int	new_line;
	int	new_val;
	int	limit;
	int	color_now;
	int	i;

	
	/* to get this routine to redraw the border, change */
	/*  g_video.g_vbl_border_color,  set g_video.g_border_last_vbl_changes = 1 */
	/*  and change the cur_border_colors[] array */

	color_now = g_video.g_vbl_border_color;

	dlines_per_dcyc = (double)(1.0 / 65.0);
	limit = g_video.g_num_border_changes;
	if(g_video.g_border_last_vbl_changes || limit) {
		/* add a dummy entry */
		g_video.g_border_changes[limit].fcycs = DCYCS_IN_16MS + 21.0;
		g_video.g_border_changes[limit].val = (g_moremem.g_c034_val & 0xf);
		limit++;
	}
	last_line_offset = (-1 << 8) + 44;
	for(i = 0; i < limit; i++) {
		dcycs = g_video.g_border_changes[i].fcycs;
		dline = dcycs * dlines_per_dcyc;
		new_line = (int)dline;
		dcyc_line_start = (double)new_line * 65.0;		
		offset = ((int)(dcycs - dcyc_line_start)) & 0xff;

		/* here comes the tricky part */
		/* offset is from 0 to 65, where 0-3 is the right border of */
		/*  the previous line, 4-20 is horiz blanking, 21-24 is the */
		/*  left border and 25-64 is the main window */
		/* Convert this to a new notation which is 0-3 is the left */
		/*  border, 4-43 is the main window, and 44-47 is the right */
		/* basically, add -21 to offset, and wrap < 0 to previous ln */
		/* note this makes line -1 offset 44-47 the left hand border */
		/* for true line 261 on the screen */
		offset -= 21;
		if(offset < 0) {
			new_line--;
			offset += 64;
		}
		new_val = g_video.g_border_changes[i].val;
		new_line_offset = (new_line << 8) + offset;

		if(new_line_offset < -256 || new_line_offset >(262*256 + 0x80)){
			printf("new_line_offset: %05x\n", new_line_offset);
			new_line_offset = last_line_offset;
		}
		while(last_line_offset < new_line_offset) {
			/* see if this will finish it */
			if((last_line_offset & -256)==(new_line_offset & -256)){
				update_border_line(last_line_offset,
						new_line_offset, color_now);
				last_line_offset = new_line_offset;
			} else {
				update_border_line(last_line_offset,
						(last_line_offset & -256) + 65,
						color_now);
				last_line_offset =(last_line_offset & -256)+256;
			}
		}

		color_now = new_val;
	}

#if 0
	if(g_video.g_num_border_changes) {
		printf("Border changes: %d\n", g_video.g_num_border_changes);
	}
#endif

	if(limit > 1) {
		g_video.g_border_last_vbl_changes = 1;
	} else {
		g_video.g_border_last_vbl_changes = 0;
	}

	g_video.g_num_border_changes = 0;
	g_video.g_vbl_border_color = (g_moremem.g_c034_val & 0xf);
}

void
update_border_line(int st_line_offset, int end_line_offset, int color)
{
    
	word32	val;
	int	st_offset, end_offset;
	int	left, right;
	int	line;

	line = st_line_offset >> 8;
	if(line != (end_line_offset >> 8)) {
		halt_printf("ubl, %04x %04x %02x!\n", st_line_offset,
					end_line_offset, color);
	}
	if(line < -1 || line >= 262) {
		halt_printf("ubl-b, mod line is %d\n", line);
		line = 0;
	}
	if(line < 0 || line >= 262) {
		line = 0;
	}

	st_offset = st_line_offset & 0xff;
	end_offset = end_line_offset & 0xff;

	if((st_offset == 0) && (end_offset >= 0x41)) {
		/* might be the same as last time, save some work */
		if(g_video.g_cur_border_colors[line] == color) {
			return;
		}
		g_video.g_cur_border_colors[line] = color;
	} else {
		g_video.g_cur_border_colors[line] = -1;
	}

	val = (color + (g_video.g_a2vid_palette << 4));
	val = (val << 24) + (val << 16) + (val << 8) + val;

	/* 0-3: left border, 4-43: main window, 44-47: right border */
	/* 48-65: horiz blanking */
	/* first, do the sides from line 0 to line 199 */
	if((line < 200) || (line >= 262)) {
		if(line >= 262) {
			line = 0;
		}
		if(st_offset < 4) {
			/* left side */
			left = st_offset;
			right = MIN(4, end_offset);
			video_border_pixel_write(&s_video.g_kimage_border_sides,
				VIDEO_VFACTOR*line, VIDEO_VFACTOR, val, (left * BORDER_WIDTH)/4,
				(right * BORDER_WIDTH) / 4);
			g_video.g_border_sides_refresh_needed = 1;
		}
		if((st_offset < 48) && (end_offset >= 44)) {
			/* right side */
			left = MAX(0, st_offset - 44);
			right = MIN(4, end_offset - 44);

			video_border_pixel_write(&s_video.g_kimage_border_sides,
				VIDEO_VFACTOR*line, VIDEO_VFACTOR, val,
				BORDER_WIDTH + (left * EFF_BORDER_WIDTH/4),
				BORDER_WIDTH + (right * EFF_BORDER_WIDTH/4));
			g_video.g_border_sides_refresh_needed = 1;
		}
	}

	if((line >= 192) && (line < 200)) {
		if(st_offset < 44 && end_offset > 4) {
			left = MAX(0, st_offset - 4);
			right = MIN(40, end_offset - 4);
			
			video_border_pixel_write(&s_video.g_kimage_border_special2,
				VIDEO_VFACTOR*line, VIDEO_VFACTOR, val, left * 320*VIDEO_HFACTOR / 40,
				right * 320*VIDEO_HFACTOR / 40);
			g_video.g_border_line24_refresh_needed = 1;
			}
	}

	/* now do the bottom, lines 200 to 215 */
	if((line >= 200) && (line < (200 + 15 /*BASE_MARGIN_BOTTOM/2*/)) ) // OG bottom border height is not linked to the real border size : it is 15px anyway
	{
		line -= 200;
		left = st_offset;
		right = MIN(48, end_offset);
		video_border_pixel_write(&s_video.g_kimage_border_special, VIDEO_VFACTOR*line, VIDEO_VFACTOR,
			val, (left * X_A2_WINDOW_WIDTH / 48),
			(right * X_A2_WINDOW_WIDTH / 48));
		g_video.g_border_special_refresh_needed = 1;
	}

	/* and top, lines 236 to 262 */
	if((line >= (262 - 16 /*BASE_MARGIN_TOP/2*/)) && (line < 262)) { // OG top border height is not linked to the real border size : it is 16px anyway
		line -= (262 - 16 /*BASE_MARGIN_TOP/2*/);
		left = st_offset;
		right = MIN(48, end_offset);
		video_border_pixel_write(&s_video.g_kimage_border_special,
			BASE_MARGIN_BOTTOM + VIDEO_VFACTOR*line, VIDEO_VFACTOR, val,
			(left * X_A2_WINDOW_WIDTH / 48),
			(right * X_A2_WINDOW_WIDTH / 48));
		g_video.g_border_special_refresh_needed = 1;
	}
}

void
video_border_pixel_write(Kimage *kimage_ptr, int starty, int num_lines,
			word32 val, int st_off, int end_off)
{
	
	int	width;
	int	width_act;
	int	mdepth;
	int	num_words, num_bytes;
	int	bytes_per_pix;
	int	i, j;

	if(end_off <= st_off) {
		return;
	}

	width = end_off - st_off;
	width_act = kimage_ptr->width_act;
	mdepth = kimage_ptr->mdepth;
	bytes_per_pix = mdepth >> 3;
	num_bytes = width * bytes_per_pix;
	num_words = num_bytes >> 2;

	if(width > width_act) {
		halt_printf("border write but width %d > act %d\n", width,
				width_act);
	}

	if(mdepth == 16) {
		val = g_a2palette_8to1624[val & 0xff];
		val = (val << 16) + val;
	} else if(mdepth == 32) {
		/* 32-bit pixels */
		val = g_a2palette_8to1624[val & 0xff];
	}

    
	for(i = 0; i < num_lines; i++) 
    {
        
        if (mdepth==8)
        {
            byte* ptr = (byte *)&(kimage_ptr->data_ptr[(starty + i)*width_act*bytes_per_pix]);
            ptr += st_off;
            memset(ptr,val,width);
        }
        else
        {
		word32* ptr = (word32 *)&(kimage_ptr->data_ptr[(starty + i)*width_act*bytes_per_pix]);
		ptr += ((st_off * bytes_per_pix) / 4);
		// HACK: the above isn't really right when bytes_per_pix is less than four... 
		for(j = 0; j < num_words; j++) 
			*ptr++ = val;
		}
	}
}


#define CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, do_clear)		\
	ch_ptr = &(s_video.slow_mem_changed[mem_ptr >> CHANGE_SHIFT]);		\
	ch_bitpos = 0;							\
	bits_per_line = 40 >> SHIFT_PER_CHANGE;				\
	ch_shift_amount = (mem_ptr >> SHIFT_PER_CHANGE) & 0x1f;		\
	mask_per_line = (-(1 << (32 - bits_per_line)));			\
	mask_per_line = mask_per_line >> ch_shift_amount;		\
	ch_mask = *ch_ptr & mask_per_line;				\
	if(do_clear) {							\
		*ch_ptr = *ch_ptr & (~ch_mask);				\
	}								\
	ch_mask = ch_mask << ch_shift_amount;				\
									\
	if(reparse) {							\
		ch_mask = - (1 << (32 - bits_per_line));		\
	}

#define CH_LOOP_A2_VID(ch_mask, ch_tmp)					\
		ch_tmp = ch_mask & 0x80000000;				\
		ch_mask = ch_mask << 1;					\
									\
		if(!ch_tmp) {						\
			continue;					\
		}

void
redraw_changed_text_40(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int altcharset, int bg_val, int fg_val,
	int pixels_per_line)
{

	register word32 start_time, end_time;
	word32	*img_ptr;
	word32	*save_img_ptr;
#ifdef VIDEO_DOUBLEVLINE
	word32	*img_ptr2;
	word32	*save_img_ptr2;
#endif
	word32	*ch_ptr;
	const word32 *font_ptr1;
	const word32 *font_ptr2;
	byte	*slow_mem_ptr;
	byte	*b_ptr;
	word32	ch_mask;
	word32	ch_tmp;
	word32	line_mask;
	word32	mask_per_line;
	word32	mem_ptr;
	word32	tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	word32	palette_add;
	word32	diff_val;
	word32	and_val;
	word32	add_val;
	const word32	ff_val = 0x0f0f0f0f;
	word32	val0, val1;
	int	flash_state;
	int	y;
	int	x1, x2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line_mod8, st_line;
	int	i;

	/* always redraws to the next multiple of 8 lines due to redraw */
	/*  issues: char changed on one screen redraw at line 0 with */
	/*  num_lines=1.  We need to have drawn lines 1-7 also since line 1 */
	/*  will not see any changed bytes */
	st_line_mod8 = start_line & 7;
	st_line = start_line;

	start_line = start_line >> 3;

	y = start_line;
	line_mask = 1 << (y);
	mem_ptr = 0x400 + g_screen_index[y] + start_offset;
	if(mem_ptr < 0x400 || mem_ptr >= 0xc00) {
		halt_printf("redraw_changed_text: mem_ptr: %08x\n", mem_ptr);
	}

	CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, (st_line_mod8 == 0));
	/* avoid clearing changed bits unless we are line 0 (mod 8) */

	if(ch_mask == 0) {
		return;
	}

	GET_ITIMER(start_time);

	shift_per = (1 << SHIFT_PER_CHANGE);

	g_video.g_a2_screen_buffer_changed |= line_mask;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	diff_val = (fg_val - bg_val) & 0xf;
	and_val = diff_val + (diff_val << 8) + (diff_val << 16) +(diff_val<<24);
	add_val = bg_val + (bg_val << 8) + (bg_val << 16) + (bg_val << 24);
	

	flash_state = (g_video.g_cur_a2_stat & ALL_STAT_FLASH_STATE);

	for(x1 = 0; x1 < 40; x1 += shift_per) 
	{

		CH_LOOP_A2_VID(ch_mask, ch_tmp);

		left = MIN(x1, left);
		right = MAX(x1 + shift_per, right);
		slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);

		b_ptr = &screen_data[(8*y + st_line_mod8)*VIDEO_VFACTOR*pixels_per_line + x1*7*VIDEO_HFACTOR];
	
		vid_printf("b_ptr:%p screen_data:%p\n",b_ptr,screen_data);
		img_ptr = (word32 *)b_ptr;
#ifdef VIDEO_DOUBLEVLINE
		img_ptr2 = (word32 *)(b_ptr + pixels_per_line);
#endif


#ifndef VIDEO_DOUBLEHLINE
		for(x2 = 0; x2 < shift_per; x2 += 4) 
		{
		/*  do 4 chars at once! */
		val0 = slow_mem_ptr[0];
		val1 = slow_mem_ptr[1];
		word32 val2 = slow_mem_ptr[2];		
		word32 val3 = slow_mem_ptr[3];
		slow_mem_ptr += 4;

		if(!altcharset) {
			if(val0 >= 0x40 && val0 < 0x80) {
				if(flash_state) {
					val0 += 0x40;
				} else {
					val0 -= 0x40;
				}
			}
			if(val1 >= 0x40 && val1 < 0x80) {
				if(flash_state) {
					val1 += 0x40;
				} else {
					val1 -= 0x40;
				}
			}
			if(val2 >= 0x40 && val2 < 0x80) {
				if(flash_state) {
					val2 += 0x40;
				} else {
					val2 -= 0x40;
				}
			}
			if(val3 >= 0x40 && val3 < 0x80) {
				if(flash_state) {
					val3 += 0x40;
				} else {
					val3 -= 0x40;
				}
			}
		}
		save_img_ptr = img_ptr;
#ifdef VIDEO_DOUBLEVLINE
		save_img_ptr2 = img_ptr2;
#endif
		for(i = st_line_mod8; i < 8; i++) 
		{
			const word32* font_ptr0 = &(g_font80_off0_bits[val0][i][0]);
			tmp0 = (font_ptr0[0] & and_val) + add_val;

			const word32* font_ptr3 = &(g_font80_off3_bits[val1][i][0]);
			tmp1 = ((font_ptr0[1]+font_ptr3[0]) & and_val)+
					add_val;
				/* 3 bytes from ptr0, 1 from ptr3 */
			tmp2 = (font_ptr3[1] & and_val) + add_val;

			font_ptr2 = &(g_font80_off2_bits[val2][i][0]);
			tmp3 = ((font_ptr3[2]+font_ptr2[0]) & and_val)+
					add_val;
				/* 2 bytes from ptr3, 2 from ptr2*/
			tmp4 = (font_ptr2[1] & and_val) + add_val;

			font_ptr1 = &(g_font80_off1_bits[val3][i][0]);
			tmp5 = ((font_ptr2[2]+font_ptr1[0]) & and_val)+
					add_val;
				/* 1 byte from ptr2, 3 from ptr1 */
			tmp6 = (font_ptr1[1] & and_val) + add_val;

			tmp0 = (tmp0 & ff_val) + palette_add;
			tmp1 = (tmp1 & ff_val) + palette_add;
			tmp2 = (tmp2 & ff_val) + palette_add;
			tmp3 = (tmp3 & ff_val) + palette_add;
			tmp4 = (tmp4 & ff_val) + palette_add;
			tmp5 = (tmp5 & ff_val) + palette_add;
			tmp6 = (tmp6 & ff_val) + palette_add;

			img_ptr[0] = tmp0;
			img_ptr[1] = tmp1;
			img_ptr[2] = tmp2;
			img_ptr[3] = tmp3;
			img_ptr[4] = tmp4;
			img_ptr[5] = tmp5;
			img_ptr[6] = tmp6;

			img_ptr += (VIDEO_VFACTOR*pixels_per_line)/4;
		}
		img_ptr = save_img_ptr + 7;
#ifdef VIDEO_DOUBLEVLINE
		img_ptr2 = save_img_ptr2 + 7;
#endif
		}
	}
#else
		for(x2 = 0; x2 < shift_per; x2 += 2) 
		{
			val0 = *slow_mem_ptr++;
			val1 = *slow_mem_ptr++;

			if(!altcharset) {
				if(val0 >= 0x40 && val0 < 0x80) {
					if(flash_state) {
						val0 += 0x40;
					} else {
						val0 -= 0x40;
					}
				}
				if(val1 >= 0x40 && val1 < 0x80) {
					if(flash_state) {
						val1 += 0x40;
					} else {
						val1 -= 0x40;
					}
				}
			}
			save_img_ptr = img_ptr;
#ifdef VIDEO_DOUBLEVLINE
			save_img_ptr2 = img_ptr2;
#endif
			for(i = st_line_mod8; i < 8; i++) {
				font_ptr1 = &(g_font40_even_bits[val0][i][0]);
				tmp0 = (font_ptr1[0] & and_val) + add_val;
				tmp1 = (font_ptr1[1] & and_val) + add_val;
				tmp2 = (font_ptr1[2] & and_val) + add_val;

				font_ptr2 = &(g_font40_odd_bits[val1][i][0]);
				tmp3 = ((font_ptr1[3]+font_ptr2[0]) & and_val)+
						add_val;

				tmp4 = (font_ptr2[1] & and_val) + add_val;
				tmp5 = (font_ptr2[2] & and_val) + add_val;
				tmp6 = (font_ptr2[3] & and_val) + add_val;

				tmp0 = (tmp0 & ff_val) + palette_add;
				tmp1 = (tmp1 & ff_val) + palette_add;
				tmp2 = (tmp2 & ff_val) + palette_add;
				tmp3 = (tmp3 & ff_val) + palette_add;
				tmp4 = (tmp4 & ff_val) + palette_add;
				tmp5 = (tmp5 & ff_val) + palette_add;
				tmp6 = (tmp6 & ff_val) + palette_add;

				img_ptr[0] = tmp0;
				img_ptr[1] = tmp1;
				img_ptr[2] = tmp2;
				img_ptr[3] = tmp3;
				img_ptr[4] = tmp4;
				img_ptr[5] = tmp5;
				img_ptr[6] = tmp6;
#ifdef VIDEO_DOUBLEVLINE
				img_ptr2[0] = tmp0;
				img_ptr2[1] = tmp1;
				img_ptr2[2] = tmp2;
				img_ptr2[3] = tmp3;
				img_ptr2[4] = tmp4;
				img_ptr2[5] = tmp5;
				img_ptr2[6] = tmp6;

				img_ptr += (2*pixels_per_line)/4;
				img_ptr2 += (2*pixels_per_line)/4;
#else		
				img_ptr += (1*pixels_per_line)/4;
#endif	
			}

			img_ptr = save_img_ptr + 7;
#ifdef VIDEO_DOUBLEVLINE
			img_ptr2 = save_img_ptr2 + 7;
#endif
		}
	}
#endif // VIDEO_DOUBLEHLINE

	GET_ITIMER(end_time);

	for(i = 0; i < (8 - st_line_mod8); i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*7*VIDEO_HFACTOR);
		g_video.g_a2_line_right_edge[st_line + i] = (right*7*VIDEO_HFACTOR);
	}

	if(left >= right || left < 0 || right < 0) {
		printf("line %d, 40: left >= right: %d >= %d\n",
			start_line, left, right);
	}

	g_video.g_cycs_in_40col += (end_time - start_time);

	g_video.g_need_redraw = 0;
}

void
redraw_changed_text_80(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int altcharset, int bg_val, int fg_val,
	int pixels_per_line)
{
	const word32 *font_ptr0, *font_ptr1, *font_ptr2, *font_ptr3;
	word32	*ch_ptr;
	word32	*img_ptr;
	word32	*save_img_ptr;
#ifdef VIDEO_DOUBLEVLINE
	word32	 *img_ptr2;
	word32	 *save_img_ptr2;
#endif
	byte	*b_ptr;
	byte	*slow_mem_ptr;
	word32	ch_mask;
	word32	ch_tmp;
	word32	mask_per_line;
	word32	mem_ptr;
	word32	tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	word32	diff_val;
	word32	add_val, and_val, ff_val;
	word32	palette_add;
	word32	line_mask;
	word32	val0, val1, val2, val3;
	int	st_line_mod8, st_line;
	int	flash_state;
	int	y;
	int	x1, x2;
	int	i;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;

#ifndef VIDEO_DOUBLEHLINE
	return ;
#endif

	st_line_mod8 = start_line & 7;
	st_line = start_line;

	start_line = start_line >> 3;

	y = start_line;
	line_mask = 1 << (y);
	mem_ptr = 0x400 + g_screen_index[y] + start_offset;
	if(mem_ptr < 0x400 || mem_ptr >= 0xc00) {
		halt_printf("redraw_changed_text: mem_ptr: %08x\n", mem_ptr);
	}

	CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, (st_line_mod8 == 0));

	if(ch_mask == 0) {
		return;
	}

	shift_per = (1 << SHIFT_PER_CHANGE);

	g_video.g_a2_screen_buffer_changed |= line_mask;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	diff_val = (fg_val - bg_val) & 0xf;
	add_val = bg_val + (bg_val << 8) + (bg_val << 16) + (bg_val << 24);
	and_val = diff_val + (diff_val << 8) + (diff_val << 16) +(diff_val<<24);
	ff_val = 0x0f0f0f0f;

	flash_state = (g_video.g_cur_a2_stat & ALL_STAT_FLASH_STATE);

	for(x1 = 0; x1 < 40; x1 += shift_per) {
		CH_LOOP_A2_VID(ch_mask, ch_tmp);

		left = MIN(x1, left);
		right = MAX(x1 + shift_per, right);

		slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
		b_ptr = &screen_data[(y*8 + st_line_mod8)*VIDEO_VFACTOR*pixels_per_line + x1*14];
		img_ptr = (word32 *)b_ptr;
#ifdef VIDEO_DOUBLEVLINE
		img_ptr2 = (word32 *)(b_ptr + pixels_per_line);
#endif

		for(x2 = 0; x2 < shift_per; x2 += 2) {
			/*  do 4 chars at once! */

			val1 = slow_mem_ptr[0];
			val3 = slow_mem_ptr[1];
			val0 = slow_mem_ptr[0x10000];
			val2 = slow_mem_ptr[0x10001];
			slow_mem_ptr += 2;

			if(!altcharset) {
				if(val0 >= 0x40 && val0 < 0x80) {
					if(flash_state) {
						val0 += 0x40;
					} else {
						val0 -= 0x40;
					}
				}
				if(val1 >= 0x40 && val1 < 0x80) {
					if(flash_state) {
						val1 += 0x40;
					} else {
						val1 -= 0x40;
					}
				}
				if(val2 >= 0x40 && val2 < 0x80) {
					if(flash_state) {
						val2 += 0x40;
					} else {
						val2 -= 0x40;
					}
				}
				if(val3 >= 0x40 && val3 < 0x80) {
					if(flash_state) {
						val3 += 0x40;
					} else {
						val3 -= 0x40;
					}
				}
			}
			save_img_ptr = img_ptr;
#ifdef VIDEO_DOUBLEVLINE
			save_img_ptr2 = img_ptr2;
#endif
			for(i = st_line_mod8; i < 8; i++) {
				font_ptr0 = &(g_font80_off0_bits[val0][i][0]);
				tmp0 = (font_ptr0[0] & and_val) + add_val;

				font_ptr3 = &(g_font80_off3_bits[val1][i][0]);
				tmp1 = ((font_ptr0[1]+font_ptr3[0]) & and_val)+
						add_val;
					/* 3 bytes from ptr0, 1 from ptr3 */
				tmp2 = (font_ptr3[1] & and_val) + add_val;

				font_ptr2 = &(g_font80_off2_bits[val2][i][0]);
				tmp3 = ((font_ptr3[2]+font_ptr2[0]) & and_val)+
						add_val;
					/* 2 bytes from ptr3, 2 from ptr2*/
				tmp4 = (font_ptr2[1] & and_val) + add_val;

				font_ptr1 = &(g_font80_off1_bits[val3][i][0]);
				tmp5 = ((font_ptr2[2]+font_ptr1[0]) & and_val)+
						add_val;
					/* 1 byte from ptr2, 3 from ptr1 */
				tmp6 = (font_ptr1[1] & and_val) + add_val;

				tmp0 = (tmp0 & ff_val) + palette_add;
				tmp1 = (tmp1 & ff_val) + palette_add;
				tmp2 = (tmp2 & ff_val) + palette_add;
				tmp3 = (tmp3 & ff_val) + palette_add;
				tmp4 = (tmp4 & ff_val) + palette_add;
				tmp5 = (tmp5 & ff_val) + palette_add;
				tmp6 = (tmp6 & ff_val) + palette_add;

				img_ptr[0] = tmp0;
				img_ptr[1] = tmp1;
				img_ptr[2] = tmp2;
				img_ptr[3] = tmp3;
				img_ptr[4] = tmp4;
				img_ptr[5] = tmp5;
				img_ptr[6] = tmp6;

				img_ptr += (VIDEO_VFACTOR*pixels_per_line)/4;

#ifdef VIDEO_DOUBLEVLINE
				img_ptr2[0] = tmp0;
				img_ptr2[1] = tmp1;
				img_ptr2[2] = tmp2;
				img_ptr2[3] = tmp3;
				img_ptr2[4] = tmp4;
				img_ptr2[5] = tmp5;
				img_ptr2[6] = tmp6;

				img_ptr2 += (2*pixels_per_line)/4;
#endif
			}

			img_ptr = save_img_ptr + 7;
#ifdef VIDEO_DOUBLEVLINE
			img_ptr2 = save_img_ptr2 + 7;
#endif
		}
	}

	for(i = 0; i < (8 - st_line_mod8); i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*14);
		g_video.g_a2_line_right_edge[st_line + i] = (right*14);
	}

	if(left >= right || left < 0 || right < 0) {
		printf("line %d, 80: left >= right: %d >= %d\n",
			start_line, left, right);
	}

	g_video.g_need_redraw = 0;
	
}

void
redraw_changed_gr(int start_offset, int start_line, int num_lines, int reparse,
	byte *screen_data, int pixels_per_line)
{
	word32	*img_ptr;
	word32	*save_img_ptr;
	word32	*ch_ptr;
	byte	*b_ptr;
	byte	*slow_mem_ptr;
	word32	mask_per_line;
	word32	ch_mask;
	word32	ch_tmp;
	word32	mem_ptr;
	word32	line_mask;
	word32	val0, val1;
	word32	val0_wd, val1_wd;
	word32	palette_add;
	int	half;
	int	x1, x2;
	int	y;
	int	y2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line_mod8, st_line, eff_line, end_line;
	int	i;

	st_line_mod8 = start_line & 7;
	st_line = start_line;
	end_line = 8;	// st_line_mod8 + num_lines;

	start_line = start_line >> 3;

	y = start_line;
	line_mask = 1 << y;
	mem_ptr = 0x400 + g_screen_index[y] + start_offset;
	if(mem_ptr < 0x400 || mem_ptr >= 0xc00) {
		printf("redraw_changed_gr: mem_ptr: %08x\n", mem_ptr);
	}

	CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, (st_line_mod8 == 0));

	if(ch_mask == 0) {
		return;
	}

	shift_per = (1 << SHIFT_PER_CHANGE);

	g_video.g_a2_screen_buffer_changed |= line_mask;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	for(x1 = 0; x1 < 40; x1 += shift_per) {
		CH_LOOP_A2_VID(ch_mask, ch_tmp);

		left = MIN(x1, left);
		right = MAX(x1 + shift_per, right);

		slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
		b_ptr = &screen_data[(y*8 + st_line_mod8)*VIDEO_VFACTOR*pixels_per_line +
									x1*7*VIDEO_HFACTOR];
		img_ptr = (word32 *)b_ptr;

#ifndef VIDEO_DOUBLEHLINE
		for(x2 = 0; x2 < shift_per; x2 += 4) {
			byte val_even_main = slow_mem_ptr[0];
			byte val_odd_main = slow_mem_ptr[1];
			byte val_even_aux = slow_mem_ptr[2];
			byte  val_odd_aux = slow_mem_ptr[3];
			slow_mem_ptr += 4;

			save_img_ptr = img_ptr;

			for(half = 0; half < 2; half++) {
				val0 = val_even_aux & 0xf;
				val1 = val_even_main & 0xf;
			word32	val2 = val_odd_aux & 0xf;
			word32 val3 = val_odd_main & 0xf;

				/* Handle funny pattern of dbl gr aux mem */
				val0 = ((val0 << 1) & 0xf) + (val0 >> 3);
				val2 = ((val2 << 1) & 0xf) + (val2 >> 3);

				val0_wd = (val0 << 24) + (val0 << 16) +
						(val0 << 8) + val0;
				val1_wd = (val1 << 24) + (val1 << 16) +
						(val1 << 8) + val1;
				word32 val2_wd = (val2 << 24) + (val2 << 16) +
						(val2 << 8) + val2;
				word32 val3_wd = (val3 << 24) + (val3 << 16) +
						(val3 << 8) + val3;
#ifdef KEGS_LITTLE_ENDIAN
				word32 val01_wd = (val1_wd << 24) + (val0_wd&0xffffff);
				word32 val12_wd = (val2_wd << 16) + (val1_wd & 0xffff);
				word32 val23_wd = (val3_wd << 8) + (val2_wd & 0xff);
#else
				word32 val01_wd = (val0_wd << 8) + (val1_wd & 0xff);
				word32 val12_wd = (val1_wd << 16) + (val2_wd & 0xffff);
				word32 val23_wd = (val2_wd << 24) + (val3_wd&0xffffff);
#endif

				for(y2 = 0; y2 < 8; y2++) {
					eff_line = half*4 + (y2 >> 1);
					if((eff_line < st_line_mod8) ||
							(eff_line > end_line)) {
						continue;
					}
					img_ptr[0] = val0_wd + palette_add;
					img_ptr[1] = val01_wd + palette_add;
					img_ptr[2] = val1_wd + palette_add;
					img_ptr[3] = val12_wd + palette_add;
					img_ptr[4] = val2_wd + palette_add;
					img_ptr[5] = val23_wd + palette_add;
					img_ptr[6] = val3_wd + palette_add;
					img_ptr += (pixels_per_line)/4;
#ifndef VIDEO_DOUBLEVLINE
					y2++;	// skip line
#endif
				}

				val_even_aux = val_even_aux >> 4;
				val_even_main = val_even_main >> 4;
				val_odd_aux = val_odd_aux >> 4;
				val_odd_main = val_odd_main >> 4;
			}

			img_ptr = save_img_ptr + 7;
		}
	}
#else // VIDEO_DOUBLEHLINE

		for(x2 = 0; x2 < shift_per; x2 += 2) {
			byte val_even = *slow_mem_ptr++;
			byte val_odd = *slow_mem_ptr++;

			save_img_ptr = img_ptr;

			for(half = 0; half < 2; half++) {
				val0 = val_even & 0xf;
				val1 = val_odd & 0xf;
				val0_wd = (val0 << 24) + (val0 << 16) +
						(val0 << 8) + val0;
				val1_wd = (val1 << 24) + (val1 << 16) +
						(val1 << 8) + val1;
#ifdef KEGS_LITTLE_ENDIAN
				word32 val01_wd = (val1_wd << 16) + (val0_wd & 0xffff);
#else
				word32 val01_wd = (val0_wd << 16) + (val1_wd & 0xffff);
#endif

				for(y2 = 0; y2 < 8; y2++) {
								
					eff_line = half*4 + (y2 >> 1);
					if((eff_line < st_line_mod8) ||
						(eff_line > end_line)) {
						continue;
					}
					
					img_ptr[0] = val0_wd + palette_add;
					img_ptr[1] = val0_wd + palette_add;
					img_ptr[2] = val0_wd + palette_add;
					img_ptr[3] = val01_wd + palette_add;
					img_ptr[4] = val1_wd + palette_add;
					img_ptr[5] = val1_wd + palette_add;
					img_ptr[6] = val1_wd + palette_add;
					img_ptr += (pixels_per_line)/4;

#ifndef VIDEO_DOUBLEVLINE
					y2++; // skip line
#endif
				}

				val_even = val_even >> 4;
				val_odd = val_odd >> 4;
			}

			img_ptr = save_img_ptr + 7;
		}
	}
#endif // !VIDEO_DOUBLEHLINE
	for(i = 0; i < (8 - st_line_mod8); i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*7*VIDEO_HFACTOR);
		g_video.g_a2_line_right_edge[st_line + i] = (right*7*VIDEO_HFACTOR);
	}

	g_video.g_need_redraw = 0;
}

void
redraw_changed_dbl_gr(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int pixels_per_line)
{
	word32	*img_ptr;
	word32	*save_img_ptr;
	word32	*ch_ptr;
	byte	*b_ptr;
	byte	*slow_mem_ptr;
	word32	mask_per_line;
	word32	ch_mask;
	word32	ch_tmp;
	word32	mem_ptr;
	word32	line_mask;
	word32	val0, val1, val2, val3;
	word32	val0_wd, val1_wd, val2_wd, val3_wd;
	word32	val01_wd, val12_wd, val23_wd;
	word32	val_even_main, val_odd_main;
	word32	val_even_aux, val_odd_aux;
	word32	palette_add;
	int	half;
	int	x1, x2;
	int	y;
	int	y2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line_mod8, st_line, eff_line, end_line;
	int	i;

#ifndef VIDEO_DOUBLEHLINE
	return ;
#endif


	st_line_mod8 = start_line & 7;
	end_line = 8;	// st_line_mod8 + num_lines
	st_line = start_line;

	start_line = start_line >> 3;

	y = start_line;
	line_mask = 1 << y;
	mem_ptr = 0x400 + g_screen_index[y] + start_offset;
	if(mem_ptr < 0x400 || mem_ptr >= 0xc00) {
		printf("redraw_changed_dbl_gr: mem_ptr: %08x\n", mem_ptr);
	}

	CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, (st_line_mod8 == 0));

	if(ch_mask == 0) {
		return;
	}

	shift_per = (1 << SHIFT_PER_CHANGE);

	g_video.g_a2_screen_buffer_changed |= line_mask;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	for(x1 = 0; x1 < 40; x1 += shift_per) {
		CH_LOOP_A2_VID(ch_mask, ch_tmp);

		left = MIN(x1, left);
		right = MAX(x1 + shift_per, right);

		slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
		b_ptr = &screen_data[(y*8 + st_line_mod8)*VIDEO_VFACTOR*pixels_per_line +
									x1*14];
		img_ptr = (word32 *)b_ptr;

		for(x2 = 0; x2 < shift_per; x2 += 2) {
			val_even_main = slow_mem_ptr[0];
			val_odd_main = slow_mem_ptr[1];
			val_even_aux = slow_mem_ptr[0x10000];
			val_odd_aux = slow_mem_ptr[0x10001];
			slow_mem_ptr += 2;

			save_img_ptr = img_ptr;

			for(half = 0; half < 2; half++) {
				val0 = val_even_aux & 0xf;
				val1 = val_even_main & 0xf;
				val2 = val_odd_aux & 0xf;
				val3 = val_odd_main & 0xf;

				/* Handle funny pattern of dbl gr aux mem */
				val0 = ((val0 << 1) & 0xf) + (val0 >> 3);
				val2 = ((val2 << 1) & 0xf) + (val2 >> 3);

				val0_wd = (val0 << 24) + (val0 << 16) +
						(val0 << 8) + val0;
				val1_wd = (val1 << 24) + (val1 << 16) +
						(val1 << 8) + val1;
				val2_wd = (val2 << 24) + (val2 << 16) +
						(val2 << 8) + val2;
				val3_wd = (val3 << 24) + (val3 << 16) +
						(val3 << 8) + val3;
#ifdef KEGS_LITTLE_ENDIAN
				val01_wd = (val1_wd << 24) + (val0_wd&0xffffff);
				val12_wd = (val2_wd << 16) + (val1_wd & 0xffff);
				val23_wd = (val3_wd << 8) + (val2_wd & 0xff);
#else
				val01_wd = (val0_wd << 8) + (val1_wd & 0xff);
				val12_wd = (val1_wd << 16) + (val2_wd & 0xffff);
				val23_wd = (val2_wd << 24) + (val3_wd&0xffffff);
#endif

				for(y2 = 0; y2 < 8; y2++) {
					eff_line = half*4 + (y2 >> 1);
					if((eff_line < st_line_mod8) ||
							(eff_line > end_line)) {
						continue;
					}
					img_ptr[0] = val0_wd + palette_add;
					img_ptr[1] = val01_wd + palette_add;
					img_ptr[2] = val1_wd + palette_add;
					img_ptr[3] = val12_wd + palette_add;
					img_ptr[4] = val2_wd + palette_add;
					img_ptr[5] = val23_wd + palette_add;
					img_ptr[6] = val3_wd + palette_add;
					img_ptr += (pixels_per_line)/4;
#ifndef VIDEO_DOUBLEVLINE
					y2++;	// skip line
#endif
				}

				val_even_aux = val_even_aux >> 4;
				val_even_main = val_even_main >> 4;
				val_odd_aux = val_odd_aux >> 4;
				val_odd_main = val_odd_main >> 4;
			}

			img_ptr = save_img_ptr + 7;
		}
	}

	for(i = 0; i < (8 - st_line_mod8); i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*14);
		g_video.g_a2_line_right_edge[st_line + i] = (right*14);
	}

	g_video.g_need_redraw = 0;
}

void
redraw_changed_hires(int start_offset, int start_line, int num_lines,
	int color, int reparse, byte *screen_data, int pixels_per_line)
{
	if(!color)
	{
		redraw_changed_hires_color(start_offset, start_line, num_lines,
			reparse, screen_data, pixels_per_line);
	} else {
		redraw_changed_hires_bw(start_offset, start_line, num_lines,
			reparse, screen_data, pixels_per_line);
	}
}

void
redraw_changed_hires_bw(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int pixels_per_line)
{
	word32	*img_ptr;
#ifdef VIDEO_DOUBLEVLINE
	word32	*img_ptr2;
#endif
	word32	*ch_ptr;
	byte	*b_ptr;
	byte	*slow_mem_ptr;
	word32	mask_per_line;
	word32	ch_mask;
#ifndef ACTIVEGSKARATEKA
	word32	ch_tmp;
#endif
	
	word32	mem_ptr;
	word32	val0, val1;
	word32	val_whole;
	word32	line_mask;
	word32	palette_add;
	int	y;
	int	x1, x2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line;
	int	i;

	
	st_line = start_line;
	start_line = start_line >> 3;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	for(y = st_line; y < (st_line + num_lines); y++) {
		line_mask = 1 << (y >> 3);
		mem_ptr = 0x2000 + (((y & 7) * 0x400) +
				g_screen_index[y >> 3]) + start_offset;

		CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, 1);

#ifndef ACTIVEGSKARATEKA
		if(ch_mask == 0) {
			continue;
		}
#else
#pragma message( "TODO Find why BW scrolling is broken")
#endif

		

		shift_per = (1 << SHIFT_PER_CHANGE);

		g_video.g_a2_screen_buffer_changed |= line_mask;

		for(x1 = 0; x1 < 40; x1 += shift_per) 
		{
#ifndef ACTIVEGSKARATEKA
			CH_LOOP_A2_VID(ch_mask, ch_tmp);
#endif

			left = MIN(x1, left);
			right = MAX(x1 + shift_per, right);

			slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
			b_ptr = &screen_data[(y*VIDEO_VFACTOR)*pixels_per_line + x1*7*VIDEO_HFACTOR];
			img_ptr = (word32 *)b_ptr;
#ifdef VIDEO_DOUBLEVLINE
			img_ptr2 = (word32 *)(b_ptr + pixels_per_line);
#endif


#ifndef VIDEO_DOUBLEHLINE
				for(x2 = 0; x2 < shift_per; x2 += 2) 
				{
				
				//	word32	tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;

				val0 = slow_mem_ptr[0];
				val1 = slow_mem_ptr[1];
				word32 val2 = slow_mem_ptr[2];
				word32 val3 = slow_mem_ptr[3];
				slow_mem_ptr += 4;

				val_whole = ((val3 & 0x7f) << 21) +
						((val2 & 0x7f) << 14) +
						((val1 & 0x7f) << 7) +
						(val0 & 0x7f);

				word32 tmp0 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp1 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp2 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp3 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp4 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp5 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp6 = g_bw_dhires_convert[val_whole & 0xf];

				img_ptr[0] = tmp0 + palette_add;
				img_ptr[1] = tmp1 + palette_add;
				img_ptr[2] = tmp2 + palette_add;
				img_ptr[3] = tmp3 + palette_add;
				img_ptr[4] = tmp4 + palette_add;
				img_ptr[5] = tmp5 + palette_add;
				img_ptr[6] = tmp6 + palette_add;
				img_ptr += 7;
				}
#else // !VIDEO_DOUBLEHLINE
			for(x2 = 0; x2 < shift_per; x2 += 2) {
				val0 = *slow_mem_ptr++;
				val1 = *slow_mem_ptr++;

				val_whole = ((val1 & 0x7f) << 7) +(val0 & 0x7f);

				word32 tmp0 = g_bw_hires_convert[val_whole & 3];
				val_whole = val_whole >> 2;
				word32 tmp1 = g_bw_hires_convert[val_whole & 3];
				val_whole = val_whole >> 2;
				word32 tmp2 = g_bw_hires_convert[val_whole & 3];
				val_whole = val_whole >> 2;
				word32 tmp3 = g_bw_hires_convert[val_whole & 3];
				val_whole = val_whole >> 2;
				word32 tmp4 = g_bw_hires_convert[val_whole & 3];
				val_whole = val_whole >> 2;
				word32 tmp5 = g_bw_hires_convert[val_whole & 3];
				val_whole = val_whole >> 2;
				word32 tmp6 = g_bw_hires_convert[val_whole & 3];

				img_ptr[0] = tmp0 + palette_add;
				img_ptr[1] = tmp1 + palette_add;
				img_ptr[2] = tmp2 + palette_add;
				img_ptr[3] = tmp3 + palette_add;
				img_ptr[4] = tmp4 + palette_add;
				img_ptr[5] = tmp5 + palette_add;
				img_ptr[6] = tmp6 + palette_add;
				img_ptr += 7;
		
#ifdef VIDEO_DOUBLEVLINE
				img_ptr2[0] = tmp0 + palette_add;
				img_ptr2[1] = tmp1 + palette_add;
				img_ptr2[2] = tmp2 + palette_add;
				img_ptr2[3] = tmp3 + palette_add;
				img_ptr2[4] = tmp4 + palette_add;
				img_ptr2[5] = tmp5 + palette_add;
				img_ptr2[6] = tmp6 + palette_add;
				img_ptr2 += 7;
#endif 
			}
#endif // VIDEO_DOUBLEHLINE
		}

	}

	for(i = 0; i < num_lines; i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*7*VIDEO_HFACTOR);
		g_video.g_a2_line_right_edge[st_line + i] = (right*7*VIDEO_HFACTOR);
	}

	g_video.g_need_redraw = 0;
}

void
redraw_changed_hires_color(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int pixels_per_line)
{
	word32	*img_ptr;
#ifdef VIDEO_DOUBLEVLINE
	word32	*img_ptr2;
#endif
	word32	*ch_ptr;
	byte	*b_ptr;
	byte	*slow_mem_ptr;
	word32	mask_per_line;
	word32	ch_mask;
	word32	ch_tmp;
	word32	mem_ptr;
	word32	val0, val1;
	word32	val_whole;
	word32	pix_val;
	word32	line_mask;
	word32	prev_pixel;
	word32	prev_hi;
	word32	loc_hi;
	word32	val_hi;
	word32	tmp_val;
	word32	palette_add;
	int	y;
	int	x1, x2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line;
	int	i, j;

	st_line = start_line;
//	int st_line2 = start_line;

	start_line = start_line >> 3;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	for(y = st_line; y < (st_line + num_lines); y++) {
		line_mask = 1 << (y >> 3);
		mem_ptr = 0x2000 + (((y & 7) * 0x400) +
				g_screen_index[y >> 3]) + start_offset;

		CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, 1);

		if(ch_mask == 0) {
			continue;
		}

		/* Hires depends on adjacent bits, so also reparse adjacent */
		/*  regions so that if bits on the edge change, redrawing is */
		/*  correct */
		ch_mask = ch_mask | (ch_mask >> 1) | (ch_mask << 1);

		shift_per = (1 << SHIFT_PER_CHANGE);

		g_video.g_a2_screen_buffer_changed |= line_mask;

		for(x1 = 0; x1 < 40; x1 += shift_per) {

			CH_LOOP_A2_VID(ch_mask, ch_tmp);

			left = MIN(x1, left);
			right = MAX(x1 + shift_per, right);

			slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
			b_ptr = &screen_data[(y*VIDEO_VFACTOR)*pixels_per_line + x1*7*VIDEO_HFACTOR];
			img_ptr = (word32 *)b_ptr;
#ifdef VIDEO_DOUBLEVLINE
			img_ptr2 = (word32 *)(b_ptr + pixels_per_line);
#endif
			prev_pixel = 0;
			prev_hi = 0;

			if(x1 > 0) {
				tmp_val = slow_mem_ptr[-1];
				prev_pixel = (tmp_val >> 6) & 1;
				prev_hi = (tmp_val >> 7) & 0x1;
			}

#ifndef VIDEO_DOUBLEHLINE

#if 0
			// mode B&w
			for(x2 = 0; x2 < shift_per; x2 += 2) {

				val0 = slow_mem_ptr[0];
				val1 = slow_mem_ptr[1];
				word32 val2 = slow_mem_ptr[2];
				word32 val3 = slow_mem_ptr[3];
				slow_mem_ptr += 4;

				val_whole = ((val3 & 0x7f) << 21) +
						((val2 & 0x7f) << 14) +
						((val1 & 0x7f) << 7) +
						(val0 & 0x7f);

				word32 tmp0 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp1 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp2 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp3 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp4 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp5 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				word32 tmp6 = g_bw_dhires_convert[val_whole & 0xf];

				/*
				word32 tmp0 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				word32 tmp1 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				word32 tmp2 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				word32 tmp3 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				word32 tmp4 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				word32 tmp5 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				if((x1 + x2 + 2) < 40) {
					val_whole += (slow_mem_ptr[0]<<8);
				}
				word32 tmp6 = g_dhires_convert[val_whole & 0xfff];
				*/
				img_ptr[0] = tmp0 + palette_add;
				img_ptr[1] = tmp1 + palette_add;
				img_ptr[2] = tmp2 + palette_add;
				img_ptr[3] = tmp3 + palette_add;
				img_ptr[4] = tmp4 + palette_add;
				img_ptr[5] = tmp5 + palette_add;
				img_ptr[6] = tmp6 + palette_add;
				img_ptr += 7;
			}
		}
#endif
			word16* img_ptrw = (word16*)b_ptr;

			for(x2 = 0; x2 < shift_per; x2 += 2) {
				val0 = *slow_mem_ptr++;
				val1 = *slow_mem_ptr++;

				val_whole = ((val1 & 0x7f) << 8) +
							((val0 & 0x7f) << 1) +
						prev_pixel;

				loc_hi = prev_hi;
				if(((val1 >> 7) & 1) != 0) {
					loc_hi += 0x7f00;
				}
				if(((val0 >> 7) & 1) != 0) {
					loc_hi += 0xfe;
				}

				prev_pixel = (val1 >> 6) & 1;
				prev_hi = (val1 >> 7) & 1;
				if((x1 + x2 + 2) < 40) {
					tmp_val = slow_mem_ptr[0];
					if(tmp_val & 1) {
						val_whole |= 0x8000;
					}
					if(tmp_val & 0x80) {
						loc_hi |= 0x8000;
					}
				}

				loc_hi = loc_hi >> 1;

				for(j = 0; j < 7; j+=1) {
					tmp_val = val_whole & 0xf;
					val_hi = loc_hi & 0x3;

					pix_val = g_hires_convert[(val_hi<<4) + tmp_val];
				
					// OG To optimize
					word16 pix_16 = (pix_val >> 8 ) & 0xFFFF;
					*img_ptrw++ = pix_16;		
					
/*
#ifdef VIDEO_DOUBLEVLINE
					*img_ptr2++ = pix_val + palette_add;
#endif
					*/
					val_whole = val_whole >> 2;
					loc_hi = loc_hi >> 2;
				}
			}
		}

#else //!VIDEO_DOUBLEHLINE
			for(x2 = 0; x2 < shift_per; x2 += 2) {
				val0 = *slow_mem_ptr++;
				val1 = *slow_mem_ptr++;

				val_whole = ((val1 & 0x7f) << 8) +
							((val0 & 0x7f) << 1) +
						prev_pixel;

				loc_hi = prev_hi;
				if(((val1 >> 7) & 1) != 0) {
					loc_hi += 0x7f00;
				}
				if(((val0 >> 7) & 1) != 0) {
					loc_hi += 0xfe;
				}

				prev_pixel = (val1 >> 6) & 1;
				prev_hi = (val1 >> 7) & 1;
				if((x1 + x2 + 2) < 40) {
					tmp_val = slow_mem_ptr[0];
					if(tmp_val & 1) {
						val_whole |= 0x8000;
					}
					if(tmp_val & 0x80) {
						loc_hi |= 0x8000;
					}
				}

				loc_hi = loc_hi >> 1;

				for(j = 0; j < 7; j+=2/VIDEO_HFACTOR) {
					tmp_val = val_whole & 0xf;
					val_hi = loc_hi & 0x3;

					pix_val = g_hires_convert[(val_hi<<4) + tmp_val];
					*img_ptr++ = pix_val + palette_add;
#ifdef VIDEO_DOUBLEVLINE
					*img_ptr2++ = pix_val + palette_add;
#endif
					val_whole = val_whole >> 2;
					loc_hi = loc_hi >> 2;
				}
			}
		}
#endif // VIDEO_DOUBLEHLINE
	}

	for(i = 0; i < num_lines; i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*7*VIDEO_HFACTOR);
		g_video.g_a2_line_right_edge[st_line + i] = (right*7*VIDEO_HFACTOR);
		/* //OG
		for(int j=0;j<32;j++)
		{
			screen_data[(i+st_line2)*VIDEO_VFACTOR*pixels_per_line+j] = start_offset?0x00:0xFF	;	
			screen_data[((i+st_line2)*VIDEO_VFACTOR+1)*pixels_per_line+j] = start_offset?0x00:0xFF	;	
		}
		*/
	}

	g_video.g_need_redraw = 0;
}


void
redraw_changed_dbl_hires(int start_offset, int start_line, int num_lines,
	int color, int reparse, byte *screen_data, int pixels_per_line)
{
	if(!color) {
		redraw_changed_dbl_hires_color(start_offset, start_line,
			num_lines, reparse, screen_data, pixels_per_line);
	} else {
		redraw_changed_dbl_hires_bw(start_offset, start_line,
			num_lines, reparse, screen_data, pixels_per_line);
	}
}


void
redraw_changed_dbl_hires_bw(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int pixels_per_line)
{
	word32	*img_ptr;
#ifdef VIDEO_DOUBLEVLINE
	word32	*img_ptr2;
#endif
	word32	*ch_ptr;
	byte	*b_ptr;
	byte	*slow_mem_ptr;
	word32	mask_per_line;
	word32	ch_mask;
	word32	ch_tmp;
	word32	mem_ptr;
	word32	val0, val1, val2, val3;
	word32	tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	word32	val_whole;
	word32	line_mask;
	word32	palette_add;
	int	y;
	int	x1, x2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line;
	int	i;

	st_line = start_line;
	start_line = start_line >> 3;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	for(y = st_line; y < (st_line + num_lines); y++) {
		line_mask = 1 << (y >> 3);
		mem_ptr = 0x2000 + (((y & 7) * 0x400) + g_screen_index[y >> 3] +
			start_offset);

		CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, 1);

		if(ch_mask == 0) {
			continue;
		}

		/* Hires depends on adjacent bits, so also reparse adjacent */
		/*  regions so that if bits on the edge change, redrawing is */
		/*  correct */
		ch_mask = ch_mask | (ch_mask >> 1) | (ch_mask << 1);

		shift_per = (1 << SHIFT_PER_CHANGE);

		g_video.g_a2_screen_buffer_changed |= line_mask;

		for(x1 = 0; x1 < 40; x1 += shift_per) {

			CH_LOOP_A2_VID(ch_mask, ch_tmp);

			left = MIN(x1, left);
			right = MAX(x1 + shift_per, right);

			slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
			b_ptr = &screen_data[(y*VIDEO_VFACTOR)*pixels_per_line + x1*14];
			img_ptr = (word32 *)b_ptr;
#ifdef VIDEO_DOUBLEVLINE
			img_ptr2 = (word32 *)(b_ptr + pixels_per_line);
#endif
			for(x2 = 0; x2 < shift_per; x2 += 2) {
				val0 = slow_mem_ptr[0x10000];
				val1 = slow_mem_ptr[0];
				val2 = slow_mem_ptr[0x10001];
				val3 = slow_mem_ptr[1];
				slow_mem_ptr += 2;

				val_whole = ((val3 & 0x7f) << 21) +
						((val2 & 0x7f) << 14) +
						((val1 & 0x7f) << 7) +
						(val0 & 0x7f);

				tmp0 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				tmp1 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				tmp2 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				tmp3 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				tmp4 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				tmp5 = g_bw_dhires_convert[val_whole & 0xf];
				val_whole = val_whole >> 4;
				tmp6 = g_bw_dhires_convert[val_whole & 0xf];

				img_ptr[0] = tmp0 + palette_add;
				img_ptr[1] = tmp1 + palette_add;
				img_ptr[2] = tmp2 + palette_add;
				img_ptr[3] = tmp3 + palette_add;
				img_ptr[4] = tmp4 + palette_add;
				img_ptr[5] = tmp5 + palette_add;
				img_ptr[6] = tmp6 + palette_add;
				img_ptr += 7;

#ifdef VIDEO_DOUBLEVLINE
				img_ptr2[0] = tmp0 + palette_add;
				img_ptr2[1] = tmp1 + palette_add;
				img_ptr2[2] = tmp2 + palette_add;
				img_ptr2[3] = tmp3 + palette_add;
				img_ptr2[4] = tmp4 + palette_add;
				img_ptr2[5] = tmp5 + palette_add;
				img_ptr2[6] = tmp6 + palette_add;
				img_ptr2 += 7;
#endif
				
			}
		}
	}

	for(i = 0; i < num_lines; i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*14);
		g_video.g_a2_line_right_edge[st_line + i] = (right*14);
	}

	g_video.g_need_redraw = 0;
}

void
redraw_changed_dbl_hires_color(int start_offset, int start_line, int num_lines,
	int reparse, byte *screen_data, int pixels_per_line)
{
	word32	*ch_ptr;
	word32	*img_ptr;
#ifdef VIDEO_DOUBLEVLINE
	word32	*img_ptr2;
#endif
	byte	*slow_mem_ptr;
	byte	*b_ptr;
	word32	mask_per_line;
	word32	ch_mask;
	word32	ch_tmp;
	word32	mem_ptr;
	word32	val0, val1, val2, val3;
	word32	tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	word32	val_whole;
	word32	prev_val;
	word32	line_mask;
	word32	palette_add;
	int	y;
	int	x1, x2;
	int	ch_bitpos;
	int	bits_per_line;
	int	ch_shift_amount;
	int	shift_per;
	int	left, right;
	int	st_line;
	int	i;

	st_line = start_line;
	start_line = start_line >> 3;

	palette_add = (g_video.g_a2vid_palette << 4);
	palette_add = palette_add + (palette_add << 8) + (palette_add << 16) +
		(palette_add << 24);

	left = 40;
	right = 0;

	for(y = st_line; y < (st_line + num_lines); y++) {
		line_mask = 1 << (y >> 3);
		mem_ptr = 0x2000 + (((y & 7) * 0x400) + g_screen_index[y >> 3] +
			start_offset);

		CH_SETUP_A2_VID(mem_ptr, ch_mask, reparse, 1);

		if(ch_mask == 0) {
			continue;
		}

		/* dbl-hires also depends on adjacent bits, so reparse */
		/*  adjacent regions so that if bits on the edge change, */
		/*  redrawing is correct */
		ch_mask = ch_mask | (ch_mask >> 1) | (ch_mask << 1);
		ch_mask = -1;

		shift_per = (1 << SHIFT_PER_CHANGE);

		g_video.g_a2_screen_buffer_changed |= line_mask;

		for(x1 = 0; x1 < 40; x1 += shift_per) {

			CH_LOOP_A2_VID(ch_mask, ch_tmp);

			left = MIN(x1, left);
			right = MAX(x1 + shift_per, right);

			slow_mem_ptr = &(g_slow_memory_ptr[mem_ptr + x1]);
			b_ptr = &screen_data[(y*VIDEO_VFACTOR)*pixels_per_line + x1*14];
			img_ptr = (word32 *)b_ptr;
#ifdef VIDEO_DOUBLEVLINE
			img_ptr2 = (word32 *)(b_ptr + pixels_per_line);
#endif
			for(x2 = 0; x2 < shift_per; x2 += 2) {
				val0 = slow_mem_ptr[0x10000];
				val1 = slow_mem_ptr[0];
				val2 = slow_mem_ptr[0x10001];
				val3 = slow_mem_ptr[1];

				prev_val = 0;
				if((x1 + x2) > 0) {
					prev_val = (slow_mem_ptr[-1] >> 3) &0xf;
				}
				val_whole = ((val3 & 0x7f) << 25) +
						((val2 & 0x7f) << 18) +
						((val1 & 0x7f) << 11) +
						((val0 & 0x7f) << 4) + prev_val;

				tmp0 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				tmp1 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				tmp2 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				tmp3 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				tmp4 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				tmp5 = g_dhires_convert[val_whole & 0xfff];
				val_whole = val_whole >> 4;
				if((x1 + x2 + 2) < 40) {
					val_whole += (slow_mem_ptr[0x10002]<<8);
				}
				tmp6 = g_dhires_convert[val_whole & 0xfff];

				img_ptr[0] = tmp0 + palette_add;
				img_ptr[1] = tmp1 + palette_add;
				img_ptr[2] = tmp2 + palette_add;
				img_ptr[3] = tmp3 + palette_add;
				img_ptr[4] = tmp4 + palette_add;
				img_ptr[5] = tmp5 + palette_add;
				img_ptr[6] = tmp6 + palette_add;
				img_ptr += 7;

#ifdef VIDEO_DOUBLEVLINE
				img_ptr2[0] = tmp0 + palette_add;
				img_ptr2[1] = tmp1 + palette_add;
				img_ptr2[2] = tmp2 + palette_add;
				img_ptr2[3] = tmp3 + palette_add;
				img_ptr2[4] = tmp4 + palette_add;
				img_ptr2[5] = tmp5 + palette_add;
				img_ptr2[6] = tmp6 + palette_add;
				img_ptr2 += 7;
#endif
				slow_mem_ptr += 2;
				
				}
		}
	}

	for(i = 0; i < num_lines; i++) {
		g_video.g_a2_line_left_edge[st_line + i] = (left*14);
		g_video.g_a2_line_right_edge[st_line + i] = (right*14);
	}

	g_video.g_need_redraw = 0;
}

int
video_rebuild_super_hires_palette(word32 scan_info, int line, int reparse)
{
	word32	*word_ptr;
	word32	*ch_ptr;
	byte	*byte_ptr;
	word32	ch_mask, mask_per_line;
	word32	tmp;
	word32	scan, old_scan;
	int	palette_changed;
	int	diff0, diff1, diff2;
	int	val0, val1, val2;
	int	diffs;
	int	low_delta, low_color;
	int	delta;
	int	full;
	int	ch_bit_offset, ch_word_offset;
	int	bits_per_line;
	int	palette;
	int	j, k;

	palette_changed = 0;
	palette = scan_info & 0xf;

	ch_ptr = &(s_video.slow_mem_changed[0x9e00 >> CHANGE_SHIFT]);
	ch_bit_offset = (palette << 5) >> SHIFT_PER_CHANGE;
	ch_word_offset = ch_bit_offset >> 5;
	ch_bit_offset = ch_bit_offset & 0x1f;
	bits_per_line = (0x20 >> SHIFT_PER_CHANGE);
	mask_per_line = -(1 << (32 - bits_per_line));
	mask_per_line = mask_per_line >> ch_bit_offset;

	ch_mask = ch_ptr[ch_word_offset] & mask_per_line;
	ch_ptr[ch_word_offset] &= ~mask_per_line;	/* clear the bits */

	old_scan = g_video.g_superhires_scan_save[line];
	scan = (scan_info & 0xfaf) + (g_video.g_palette_change_cnt[palette] << 12);
	g_video.g_superhires_scan_save[line] = scan;

#if 0
	if(line == 1) {
		word_ptr = (word32 *)&(g_slow_memory_ptr[0x19e00+palette*0x20]);
		printf("y1vrshp, ch:%08x, s:%08x,os:%08x %d = %08x %08x %08x %08x %08x %08x %08x %08x\n",
			ch_mask, scan, old_scan, reparse,
			word_ptr[0], word_ptr[1], word_ptr[2], word_ptr[3],
			word_ptr[4], word_ptr[5], word_ptr[6], word_ptr[7]);
	}
#endif

	diffs = reparse | ((scan ^ old_scan) & 0xf0f);
		/* we must do full reparse if palette changed for this line */

	if(!diffs && (ch_mask == 0) && (((scan ^ old_scan) & (~0xf0)) == 0)) {
		/* nothing changed, get out fast */
		return 0;
	}

	if(ch_mask) {
		/* indicates the palette has changed, and other scan lines */
		/*  using this palette need to do a full 32-byte compare to */
		/*  decide if they need to update or not */
		g_video.g_palette_change_cnt[palette]++;
	}

	word_ptr = (word32 *)&(g_slow_memory_ptr[0x19e00 + palette*0x20]);
	for(j = 0; j < 8; j++) {
		if(word_ptr[j] != s_video.g_saved_line_palettes[line][j]) {
			diffs = 1;
			break;
		}
	}

	if(diffs == 0) {
		return 0;
	}

	/* first, save this word_ptr into saved_line_palettes */
	byte_ptr = (byte *)word_ptr;
	for(j = 0; j < 8; j++) {
		s_video.g_saved_line_palettes[line][j] = word_ptr[j];
	}

	full = s_video.g_installed_full_superhires_colormap;

	if(!full && palette == g_video.g_a2vid_palette) {
		/* construct new color approximations from lores */
		for(j = 0; j < 16; j++) {
			tmp = *byte_ptr++;
			val2 = (*byte_ptr++) & 0xf;
			val0 = tmp & 0xf;
			val1 = (tmp >> 4) & 0xf;
			low_delta = 0x1000;
			low_color = 0x0;
			for(k = 0; k < 16; k++) {
				diff0 = g_expanded_col_0[k] - val0;
				diff1 = g_expanded_col_1[k] - val1;
				diff2 = g_expanded_col_2[k] - val2;
				if(diff0 < 0) {
					diff0 = -diff0;
				}
				if(diff1 < 0) {
					diff1 = -diff1;
				}
				if(diff2 < 0) {
					diff2 = -diff2;
				}
				delta = diff0 + diff1 + diff2;
				if(delta < low_delta) {
					low_delta = delta;
					low_color = k;
				}
			}

			g_video.g_a2vid_palette_remap[j] = low_color;
		}
	}
	// OG
	//extern double last_vid_update_dcys;
	//printf("palette %d (%x) changed @line:%d (vidupd:%f) last_vbl:%d (%f) dcycs :%f\n",palette,*byte_ptr,line,last_vid_update_dcys,g_sim65816.g_vbl_count,g_sim65816.g_last_vbl_dcycs,g_sim65816.g_cur_dcycs);
	byte_ptr = (byte *)word_ptr;
	/* this palette has changed */
	for(j = 0; j < 16; j++) {
		val0 = *byte_ptr++;
		val1 = *byte_ptr++;
		video_update_color_array(palette*16 + j, (val1<<8) + val0);
	}

	g_video.g_palette_change_summary = 1;

	return 1;
}

#define SUPER_TYPE	redraw_changed_super_hires_oneline_nofill_8
#define SUPER_FILL		0
#define SUPER_PIXEL_SIZE	8
#include "superhires.h"
#undef	SUPER_TYPE
#undef	SUPER_FILL
#undef	SUPER_PIXEL_SIZE

#define SUPER_TYPE	redraw_changed_super_hires_oneline_nofill_16
#define SUPER_FILL		0
#define SUPER_PIXEL_SIZE	16
#include "superhires.h"
#undef	SUPER_TYPE
#undef	SUPER_FILL
#undef	SUPER_PIXEL_SIZE

#define SUPER_TYPE	redraw_changed_super_hires_oneline_nofill_32
#define SUPER_FILL		0
#define SUPER_PIXEL_SIZE	32
#include "superhires.h"
#undef	SUPER_TYPE
#undef	SUPER_FILL
#undef	SUPER_PIXEL_SIZE

#define SUPER_TYPE	redraw_changed_super_hires_oneline_fill_8
#define SUPER_FILL		1
#define SUPER_PIXEL_SIZE	8
#include "superhires.h"
#undef	SUPER_TYPE
#undef	SUPER_FILL
#undef	SUPER_PIXEL_SIZE

#define SUPER_TYPE	redraw_changed_super_hires_oneline_fill_16
#define SUPER_FILL		1
#define SUPER_PIXEL_SIZE	16
#include "superhires.h"
#undef	SUPER_TYPE
#undef	SUPER_FILL
#undef	SUPER_PIXEL_SIZE

#define SUPER_TYPE	redraw_changed_super_hires_oneline_fill_32
#define SUPER_FILL		1
#define SUPER_PIXEL_SIZE	32
#include "superhires.h"
#undef	SUPER_TYPE
#undef	SUPER_FILL
#undef	SUPER_PIXEL_SIZE



void
redraw_changed_super_hires(int start_offset, int start_line, int num_lines,
	int in_reparse, byte *screen_data)
{
	word32	*ch_ptr;
	word32	mask_per_line;
	word32	all_checks;
	word32	check0, check1, mask0, mask1;
	word32	this_check;
	word32	tmp;
	word32	line_mask;
	word32	pal;
	word32	scan, old_scan;
	word32	kd_tmp_debug;
	int	y;
	int	bits_per_line;
	int	a2vid_palette;
	int	type;
	int	left, right;
	int	st_line;
	int	check_bit_pos, check_word_off;
	int	pixel_size, pixel_size_type;
	int	use_a2vid_palette, mode_640;
	int	pixels_per_line;
	int	ret;
	int	i;

	st_line = start_line;
	start_line = start_line >> 3;

	pixel_size = s_video.g_kimage_superhires.mdepth;
	pixels_per_line = s_video.g_kimage_superhires.width_act;

	pixel_size_type = (pixel_size >> 3) - 1;
	/* pixel_size_type is now: 0=8bit, 1=16bit, 3=32bit */
	if(pixel_size_type >= 3) {
		pixel_size_type = 2;
	}

	kd_tmp_debug = g_video.g_a2_screen_buffer_changed;

	line_mask = 1 << (start_line);

	ch_ptr = &(s_video.slow_mem_changed[(0x2000) >> CHANGE_SHIFT]);
	bits_per_line = 160 >> SHIFT_PER_CHANGE;
	mask_per_line = -(1 << (32 - bits_per_line));

	if(SHIFT_PER_CHANGE != 3) {
		halt_printf("SHIFT_PER_CHANGE must be 3!\n");
		return;
	}

	a2vid_palette = g_video.g_a2vid_palette;
	if(s_video.g_installed_full_superhires_colormap) {
		a2vid_palette = -1;
	} else {
		/* handle palette counting for finding least-used palette */
		if(pixel_size == 8) {
			for(y = 8*start_line; y < 8*(start_line + 1); y++) {
				scan = g_slow_memory_ptr[0x19d00 + y];
				pal = scan & 0xf;
				g_video.g_shr_palette_used[pal]++;
			}
		}
	}

	all_checks = 0;
	check0 = 0;
	check1 = 0;
	for(y = st_line; y < (st_line + num_lines); y++) {
		scan = g_slow_memory_ptr[0x19d00 + y];
		check_bit_pos = bits_per_line * y;
		check_word_off = check_bit_pos >> 5;	/* 32 bits per word */
		check_bit_pos = check_bit_pos & 0x1f;	/* 5-bit bit_pos */
		check0 = ch_ptr[check_word_off];
		check1 = ch_ptr[check_word_off+1];
		mask0 = mask_per_line >> check_bit_pos;
		mask1 = 0;
		this_check = check0 << check_bit_pos;
				/* move indicated bit to MSbit position */
		if((check_bit_pos + bits_per_line) > 32) {
			this_check |= (check1 >> (32 - check_bit_pos));
			mask1 = mask_per_line << (32 - check_bit_pos);
		}

		ch_ptr[check_word_off] = check0 & ~mask0;
		ch_ptr[check_word_off+1] = check1 & ~mask1;

		this_check = this_check & mask_per_line;
		old_scan = g_video.g_superhires_scan_save[y];
		use_a2vid_palette = ((scan & 0xf) == (word32)a2vid_palette);
		scan = (scan + (a2vid_palette << 8)) & 0xfff;

		ret = video_rebuild_super_hires_palette(scan, y, in_reparse);
#if 0
		if(y == 1) {
			printf("y1, ch:%08x, ret:%d, scan:%03x, os:%03x\n",
				this_check, ret, scan, old_scan);
		}
#endif
		if(ret || in_reparse || ((scan ^ old_scan) & 0xa0)) {
					/* 0x80 == mode640, 0x20 = fill */
			this_check = -1;
		}

		if(this_check == 0) {
			continue;
		}

		mode_640 = (scan & 0x80);
		if(mode_640) {
			g_video.g_num_lines_superhires640++;
		}

		type = ((scan >> 5) & 1) + (pixel_size_type << 1);
		if(type & 1) {
			/* fill mode--redraw whole line */
			this_check = -1;
		}

		all_checks |= this_check;

		g_video.g_a2_screen_buffer_changed |= line_mask;


		switch(type) {
		case 0:	/* nofill, 8 bit pixels */
			redraw_changed_super_hires_oneline_nofill_8(
				screen_data, pixels_per_line, y, scan,
				this_check, use_a2vid_palette, mode_640);
			break;
		case 1:	/* fill, 8 bit pixels */
			redraw_changed_super_hires_oneline_fill_8(
				screen_data, pixels_per_line, y, scan,
				this_check, use_a2vid_palette, mode_640);
			break;
		case 2:	/* nofill, 16 bit pixels */
			redraw_changed_super_hires_oneline_nofill_16(
				screen_data, pixels_per_line, y, scan,
				this_check, use_a2vid_palette, mode_640);
			break;
		case 3:	/* fill, 16 bit pixels */
			redraw_changed_super_hires_oneline_fill_16(
				screen_data, pixels_per_line, y, scan,
				this_check, use_a2vid_palette, mode_640);
			break;
		case 4:	/* nofill, 32 bit pixels */
			redraw_changed_super_hires_oneline_nofill_32(
				screen_data, pixels_per_line, y, scan,
				this_check, use_a2vid_palette, mode_640);
			break;
		case 5:	/* fill, 32 byte pixels */
			redraw_changed_super_hires_oneline_fill_32(
				screen_data, pixels_per_line, y, scan,
				this_check, use_a2vid_palette, mode_640);
			break;
		default:
			halt_printf("type: %d bad!\n", type);
			break;
		}
	}

	left = 4*40;
	right = 0;

	tmp = all_checks;
	if(all_checks) {
		for(i = 0; i < 160; i += 8) {
			if(tmp & 0x80000000) {
				left = MIN(i, left);
				right = MAX(i + 8, right);
			}
			tmp = tmp << 1;
		}
	}

	for(i = 0; i < num_lines; i++) {
		g_video.g_a2_line_left_edge[st_line + i] = 2*VIDEO_HFACTOR*left;
		g_video.g_a2_line_right_edge[st_line + i] = 2*VIDEO_HFACTOR*right;
	}

#if 0
	if((g_video.g_a2_screen_buffer_changed & (1 << start_line)) != 0) {
		if(((g_video.g_full_refresh_needed & (1 << start_line)) == 0) &&
							left >= right) {
			halt_printf("shr: line: %d, left: %d, right:%d\n",
				start_line, left, right);
			printf("mask_per_line: %08x, all_checks: %08x\n",
				mask_per_line, all_checks);
			printf("check0,1 = %08x,%08x\n", check0, check1);
			printf("a2_screen_chang: %08x\n", kd_tmp_debug);
#ifdef HPUX
			U_STACK_TRACE();
#endif
		}
	}
#endif

	g_video.g_need_redraw = 0;
}

void
display_screen()
{
	

	video_update_through_line(262);
}
//double last_vid_update_dcys;	// OG

void
video_update_event_line(int line)
{
	int	new_line;

	video_update_through_line(line);

	new_line = line + g_sim65816.g_line_ref_amt;
	if(new_line < 200) {
		if(!g_config_control_panel) {
			add_event_vid_upd(new_line);
		}
	} else if(line >= 262) {
		video_update_through_line(0);
		if(!g_config_control_panel) {
			add_event_vid_upd(1);	/* add event for new screen */
		}
	}

	if(g_video.g_video_extra_check_inputs) {
		if(g_video.g_video_dcycs_check_input < g_sim65816.g_cur_dcycs)
		{
			video_check_input_events();
		}
	}
}

void
video_check_input_events()
{
	word32	start_time, end_time;

	g_video.g_video_dcycs_check_input = g_sim65816.g_cur_dcycs + 4000.0;

	GET_ITIMER(start_time);
	extern void should_decrement_fifo();
	should_decrement_fifo();
	check_input_events();
	GET_ITIMER(end_time);

	g_video.g_cycs_in_check_input += (end_time - start_time);
}

void
video_update_through_line(int line)
{
	register word32 start_time;
	register word32 end_time;
	int	*mode_ptr;
	word32	mask;
	int	last_line, num_lines;
	int	must_reparse;
	int	new_all_stat, prev_all_stat;
	int	new_stat, prev_stat;
	int	i;

#if 0
	vid_printf("\nvideo_upd for line %d, lines: %06x\n", line,
				get_lines_since_vbl(g_cur_dcycs));
#endif

	GET_ITIMER(start_time);

	video_update_all_stat_through_line(line);

	i = g_video.g_vid_update_last_line;

	last_line = MIN(200, line+1);	/* go through line, but not past 200 */

	prev_stat = -2;
	prev_all_stat = -2;
	num_lines = 0;
	must_reparse = 0;
	for(i = g_video.g_vid_update_last_line; i < last_line; i++) {
		new_all_stat = g_video.g_a2_new_all_stat[i];
		if(new_all_stat != g_video.g_a2_cur_all_stat[i]) {
			/* regen line_stat for this line */
			g_video.g_a2_cur_all_stat[i] = new_all_stat;
			if(new_all_stat == prev_all_stat) {
				/* save a lookup */
				new_stat = prev_stat;
			} else {
				new_stat = video_all_stat_to_line_stat(i,
								new_all_stat);
			}
			if(new_stat != g_video.g_a2_line_stat[i]) {
				/* status changed */
				g_video.g_a2_line_stat[i] = new_stat;
				mode_ptr = video_update_kimage_ptr(i, new_stat);
				if(mode_ptr[i] != new_stat) {
					must_reparse = 1;
					mode_ptr[i] = new_stat;
				}
				mask = 1 << (line >> 3);
				g_video.g_full_refresh_needed |= mask;
				g_video.g_a2_screen_buffer_changed |= mask;
			}
		}

		new_stat = g_video.g_a2_line_stat[i];

		if( ((new_stat == prev_stat) && ((i & 7) != 0)) ||
							(num_lines == 0) ) {
			/* merge prev and this together */
			prev_stat = new_stat;
			num_lines++;
			continue;
		}

		/* else, we must call refresh */
		video_refresh_lines(i - num_lines, num_lines, must_reparse);
		num_lines = 1;
		prev_all_stat = -1;
		prev_stat = new_stat;
		must_reparse =   0;
	}

	if(num_lines > 0) {

		video_refresh_lines(i - num_lines, num_lines, must_reparse);
	}

	g_video.g_vid_update_last_line = last_line;

	/* deal with border */
	if(line >= 262) {
		if(g_video.g_num_lines_prev_superhires != g_video.g_num_lines_superhires) {
			/* switched in/out from superhires--refresh borders */
			g_video.g_border_sides_refresh_needed = 1;
		}
		refresh_border();

		if(g_video.g_status_refresh_needed) {
			g_video.g_status_refresh_needed = 0;
			x_redraw_status_lines();
		}
	}
	GET_ITIMER(end_time);

	g_video.g_cycs_in_refresh_line += (end_time - start_time);

	if(line >= 262) {
		GET_ITIMER(start_time);
		if(g_video.g_palette_change_summary) {
			g_video.g_palette_change_summary = 0;
			video_update_colormap();
		}

		video_push_kimages();
		GET_ITIMER(end_time);
		g_video.g_cycs_in_refresh_ximage += (end_time - start_time);

		g_video.g_num_lines_prev_superhires = g_video.g_num_lines_superhires;
		g_video.g_num_lines_prev_superhires640 = g_video.g_num_lines_superhires640;
		g_video.g_num_lines_superhires = 0;
		g_video.g_num_lines_superhires640 = 0;
	}
}

void
video_refresh_lines(int st_line, int num_lines, int must_reparse)
{
	byte	*ptr;
	int	line;
	int	stat;
	int	mode;
	int	dbl, page, color;
	int	altchar, bg_color, text_color;
	int	pixels_per_line;
	int	i;

	vid_printf("video_refresh_lines %d %d\n",st_line,num_lines);

	line = st_line;

	/* do some basic checking, num_lines should be 1-8, and */
	/*  st_line+num_lines-1 cannot roll over 8 */
	if((num_lines < 1) || (num_lines > 8) ||
					(((st_line & 7) + num_lines) > 8) ) {
		halt_printf("video_refresh_lines called with %d, %d\n",
			st_line, num_lines);
		return;
	}

	stat = g_video.g_a2_line_stat[line];
	ptr = g_video.g_a2_line_kimage[line]->data_ptr;
	pixels_per_line = g_video.g_a2_line_kimage[line]->width_act;

	/* do not zero g_a2_line_left/right_edge here since text/gr routs */
	/*  need to leave stale values around for drawing to work correctly */
	/* all routs force in new left/right when there are screen changes */

	dbl = stat & 1;
	color = (stat >> 1) & 1;
	page = (stat >> 2) & 1;
	mode = (stat >> 4) & 7;

#if 0
	printf("refresh line: %d, stat: %04x\n", line, stat);
#endif

	switch(mode) {
	case MODE_TEXT:
		vid_printf("refresh line: %d, stat: %04x\n", line, stat);
		altchar = (stat >> 7) & 1;
		bg_color = (stat >> 8) & 0xf;
		text_color = (stat >> 12) & 0xf;
		if(dbl) {
			redraw_changed_text_80(0x000 + page*0x400, st_line,
				num_lines, must_reparse, ptr, altchar, bg_color,
				text_color, pixels_per_line);
		} else {
			redraw_changed_text_40(0x000 + page*0x400, st_line,
				num_lines, must_reparse, ptr, altchar, bg_color,
				text_color, pixels_per_line);
		}
		break;
	case MODE_GR:
		if(dbl) {
			redraw_changed_dbl_gr(0x000 + page*0x400, st_line,
				num_lines, must_reparse, ptr, pixels_per_line);
		} else {
			redraw_changed_gr(0x000 + page*0x400, st_line,
				num_lines, must_reparse, ptr, pixels_per_line);
		}
		break;
	case MODE_HGR:
		if(dbl) {
			redraw_changed_dbl_hires(0x000 + page*0x2000, st_line,
				num_lines, color, must_reparse, ptr,
				pixels_per_line);
		} else {
			redraw_changed_hires(0x000 + page*0x2000, st_line,
				num_lines, color, must_reparse, ptr,
				pixels_per_line);
		}
		break;
	case MODE_SUPER_HIRES:
		g_video.g_num_lines_superhires++;
		redraw_changed_super_hires(0, st_line, num_lines,
				must_reparse, ptr);
		break;
	case MODE_BORDER:
		if(line < 192) {
			halt_printf("Border line not 192: %d\n", line);
		}
		for(i = 0; i < num_lines; i++) {
			g_video.g_a2_line_left_edge[line + i] = 0;
			g_video.g_a2_line_right_edge[line + i] = 320*VIDEO_HFACTOR;
		}
		if(g_video.g_border_line24_refresh_needed) {
			g_video.g_border_line24_refresh_needed = 0;
			g_video.g_a2_screen_buffer_changed |= (1 << 24);
		}
		break;
	default:
		halt_printf("refresh screen: mode: 0x%02x unknown!\n", mode);
		x_exit(7);
		break;
	}
}

void
refresh_border()
{
	/**ZZZZ***/
}

// OG Added video_release_kimages proto
void video_release_kimages();

void
end_screen()
{
	printf("In end_screen\n");

	// OG Free up allocated images
	video_release_kimages();
	x_video_shut();
}

byte g_font_array[256][8] = {
#include "kegsfont.h"
};

void
read_a2_font()
{
	byte	*f40_e_ptr;
	byte	*f40_o_ptr;
	byte	*f80_0_ptr, *f80_1_ptr, *f80_2_ptr, *f80_3_ptr;
	int	char_num;
	int	j, k;
	int	val0;
	int	mask;
	int	pix;

	for(char_num = 0; char_num < 0x100; char_num++) {
		for(j = 0; j < 8; j++) {
			val0 = g_font_array[char_num][j];

			mask = 0x80;

			for(k = 0; k < 3; k++) {
				g_font80_off0_bits[char_num][j][k] = 0;
				g_font80_off1_bits[char_num][j][k] = 0;
				g_font80_off2_bits[char_num][j][k] = 0;
				g_font80_off3_bits[char_num][j][k] = 0;
				g_font40_even_bits[char_num][j][k] = 0;
				g_font40_odd_bits[char_num][j][k] = 0;
			}
			g_font40_even_bits[char_num][j][3] = 0;
			g_font40_odd_bits[char_num][j][3] = 0;

			f40_e_ptr = (byte *)&g_font40_even_bits[char_num][j][0];
			f40_o_ptr = (byte *)&g_font40_odd_bits[char_num][j][0];

			f80_0_ptr = (byte *)&g_font80_off0_bits[char_num][j][0];
			f80_1_ptr = (byte *)&g_font80_off1_bits[char_num][j][0];
			f80_2_ptr = (byte *)&g_font80_off2_bits[char_num][j][0];
			f80_3_ptr = (byte *)&g_font80_off3_bits[char_num][j][0];

			for(k = 0; k < 7; k++) {
				pix = 0;
				if(val0 & mask) {
					pix = 0xf;
				}

				f40_e_ptr[2*k] = pix;
				f40_e_ptr[2*k+1] = pix;

				f40_o_ptr[2*k+2] = pix;
				f40_o_ptr[2*k+3] = pix;

				f80_0_ptr[k] = pix;
				f80_1_ptr[k+1] = pix;
				f80_2_ptr[k+2] = pix;
				f80_3_ptr[k+3] = pix;

				mask = mask >> 1;
			}
		}
	}
}


/* Helper routine for the *driver.c files */

void init_kimage(Kimage *kimage_ptr, int extend_info, int depth, int mdepth)
{
	int width = A2_WINDOW_WIDTH;
	int height = A2_WINDOW_HEIGHT;
	if(extend_info & 1) {
		/* Border at top and bottom of screen */
		width = X_A2_WINDOW_WIDTH;
		height = X_A2_WINDOW_HEIGHT - A2_WINDOW_HEIGHT + 2*8;
	}
	if(extend_info & 2) {
		/* Border at sides of screen */
		width = BORDER_WIDTH + EFF_BORDER_WIDTH;
		height = A2_WINDOW_HEIGHT;
	}

	memset(kimage_ptr,0,sizeof(Kimage));

	kimage_ptr->width_req = width;
	kimage_ptr->width_act = width;
	kimage_ptr->height = height;
	kimage_ptr->depth = depth;
	kimage_ptr->mdepth = mdepth;

}

void video_get_kimage(Kimage *kimage_ptr, int extend_info, int _depth, int _mdepth)
{
	init_kimage(kimage_ptr,extend_info,_depth,_mdepth);

	byte	*ptr;

	int width = kimage_ptr->width_req;
	int height = kimage_ptr->height;
	//int depth = kimage_ptr->depth;
	int mdepth = kimage_ptr->mdepth;

	kimage_ptr->data_size = (width*height*mdepth) >> 3;
	ptr = (byte *)malloc(kimage_ptr->data_size);

	if(ptr == 0) {
		printf("malloc for data failed, mdepth: %d\n", mdepth);
		x_exit(2);
	}

	kimage_ptr->data_ptr = ptr;
	kimage_ptr->dev_handle = (void *)-1;
}


void
video_get_kimages()
{
    video_get_kimage(&s_video.g_kimage_border_special2, 0, 8, 8);
	video_get_kimage(&s_video.g_kimage_text[0], 0, 8, 8);
	video_get_kimage(&s_video.g_kimage_text[1], 0, 8, 8);
	video_get_kimage(&s_video.g_kimage_hires[0], 0, 8, 8);
	video_get_kimage(&s_video.g_kimage_hires[1], 0, 8, 8);
	video_get_kimage(&s_video.g_kimage_superhires, 0, s_video.g_screen_depth,
							s_video.g_screen_mdepth);
	video_get_kimage(&s_video.g_kimage_border_special, 1, s_video.g_screen_depth,
							s_video.g_screen_mdepth);
	video_get_kimage(&s_video.g_kimage_border_sides, 2, s_video.g_screen_depth,
							s_video.g_screen_mdepth);
}

// OG Added video_release_kimages (to match video_get_kimages)
void video_release_kimages()
{
	extern	void x_release_kimage(Kimage *kimage_ptr);

    x_release_kimage(&s_video.g_kimage_border_special2);
	x_release_kimage(&s_video.g_kimage_text[0]);
	x_release_kimage(&s_video.g_kimage_text[1]);
	x_release_kimage(&s_video.g_kimage_hires[0]);
	x_release_kimage(&s_video.g_kimage_hires[1]);
	x_release_kimage(&s_video.g_kimage_superhires);
	x_release_kimage(&s_video.g_kimage_border_special);
	x_release_kimage(&s_video.g_kimage_border_sides);
}


void
video_convert_kimage_depth(Kimage *kim_in, Kimage *kim_out, int startx,
		int starty, int width, int height)
{
	byte	*indata, *inptr;
	word32	*outdata32, *outptr32;
	word16	*outdata16, *outptr16;
	word32	*palptr;
	int	out_width, in_width;
	int	x, y;

	indata = (byte *)kim_in->data_ptr;
	outdata32 = (word32 *)kim_out->data_ptr;
	outdata16 = (word16 *)kim_out->data_ptr;
	
	// OG added safe check (to investigate further)
	if (!outdata32)
	{
		printf("video_convert_kimage_depth: kim_out* not yet initialized\n");
		return ;
	}

	if(kim_in == &s_video.g_kimage_superhires) {
		palptr = &(g_palette_8to1624[0]);
	} else {
		palptr = &(g_a2palette_8to1624[0]);
	}
	if(kim_in->depth != 8) {
		printf("x_convert_kimage_depth from non-8 bit depth: %p\n",
			kim_in);
		x_exit(1);
	}

	out_width = kim_out->width_act;
	in_width = kim_in->width_act;
	indata += (starty * in_width + startx);
	outdata32 += (starty * out_width + startx);
	outdata16 += (starty * out_width + startx);
	if(kim_out->mdepth == 16) {
		for(y = 0; y < height; y++) {
			outptr16 = outdata16;
			inptr = indata;
			for(x = 0; x < width; x++) {
				*outptr16++ = palptr[*inptr++];
			}
			outdata16 += out_width;
			indata += in_width;
		}
	} else {
		/* 32-bit depth */
		for(y = 0; y < height; y++) {
			outptr32 = outdata32;
			inptr = indata;
			for(x = 0; x < width; x++) {
				*outptr32++ = palptr[*inptr++];
			}
			outdata32 += out_width;
			indata += in_width;
		}
	}
}

void
video_push_lines(Kimage *kimage_ptr, int start_line, int end_line, int left_pix,
		int right_pix)
{
	int	mdepth_mismatch;
	int	srcy;
	int center = 0; // OG added variable to center screen

	//OG add null pointer check when emulator is restarted
	if (!kimage_ptr)	
	{
		printf("warning : video_push_lines(kimage_ptr=null)\n");
		return ;
	}

	if(left_pix >= right_pix || left_pix < 0 || right_pix <= 0) {
		halt_printf("video_push_lines: lines %d to %d, pix %d to %d\n",
			start_line, end_line, left_pix, right_pix);
		printf("a2_screen_buf_ch:%08x, g_full_refr:%08x\n",
			g_video.g_a2_screen_buffer_changed, g_video.g_full_refresh_needed);
	}

	srcy = VIDEO_VFACTOR*start_line;

	mdepth_mismatch = (kimage_ptr->mdepth != s_video.g_screen_mdepth);
	if(mdepth_mismatch) {
		/* translate from 8-bit pseudo to correct visual */
		video_convert_kimage_depth(kimage_ptr, &s_video.g_mainwin_kimage,
			left_pix, srcy, (right_pix - left_pix),
			VIDEO_VFACTOR*(end_line - start_line));
		kimage_ptr = &s_video.g_mainwin_kimage;
	}
	g_refresh_bytes_xfer += VIDEO_VFACTOR*(end_line - start_line) *(right_pix - left_pix);

	// OG Calculating new center
	if (g_video.g_cur_a2_stat & ALL_STAT_SUPER_HIRES)
		center=0;
	else
		center=EFF_BORDER_WIDTH - BORDER_WIDTH;

	// OG shifting image to the center
	x_push_kimage(kimage_ptr, g_video_act_margin_left + left_pix + center,	
			g_video_act_margin_top + srcy, left_pix, srcy,
			(right_pix - left_pix), VIDEO_VFACTOR*(end_line - start_line));
}

void
video_push_border_sides_lines(int src_x, int dest_x, int width, int start_line,
	int end_line)
{

	Kimage	*kimage_ptr;
	int	srcy;

	if(start_line < 0 || width < 0) {
		return;
	}

#if 0
	printf("push_border_sides lines:%d-%d from %d to %d\n",
		start_line, end_line, end_x - width, end_x);
#endif
	kimage_ptr = &s_video.g_kimage_border_sides;
	g_refresh_bytes_xfer += VIDEO_VFACTOR * (end_line - start_line) * width;

	srcy = VIDEO_VFACTOR * start_line;

	// Adjust dext_x to accound for changed margins
	dest_x = dest_x + g_video_act_margin_left - BASE_MARGIN_LEFT;
	if(dest_x < BASE_MARGIN_LEFT) {
		src_x = src_x + g_video_act_margin_left - BASE_MARGIN_LEFT;
		// Don't adjust src_x if doing right border
	}
	if(dest_x < 0) {
		width = width + dest_x;
		src_x = src_x - dest_x;
		dest_x = 0;
	}
	if(src_x < 0) {
		width = width + src_x;
		dest_x = dest_x - src_x;
		src_x = 0;
	}
	if(dest_x + width > g_video_act_width) {
		width = g_video_act_width - dest_x;
	}
	if(width > 0) {
		x_push_kimage(kimage_ptr, dest_x, g_video_act_margin_top + srcy,
			src_x, srcy, width, VIDEO_VFACTOR*(end_line - start_line));
	}
}

void
video_push_border_sides()
{
	int	old_width;
	int	prev_line;
	int	width;
	int	mode;
	int	i;

#if 0
	printf("refresh border sides!\n");
#endif

	/* redraw left sides */
	// OG Left side can alos be "jagged" as a2 screen is now being centered
	
	//video_push_border_sides_lines(0, 0, BORDER_WIDTH, 0, 200);

	prev_line = -1;
	old_width = -1;
	for(i = 0; i < 200; i++) {
		mode = (g_video.g_a2_line_stat[i] >> 4) & 7;
		width = EFF_BORDER_WIDTH;
		if(mode == MODE_SUPER_HIRES) {
			width = BORDER_WIDTH;
		}
		if(width != old_width) {
			video_push_border_sides_lines(BORDER_WIDTH,
				0, old_width,
				prev_line, i);
			prev_line = i;
			old_width = width;
		}
	}
	video_push_border_sides_lines(0/*BORDER_WIDTH*/,
		0, old_width, prev_line, 200);


	/* right side--can be "jagged" */
	prev_line = -1;
	old_width = -1;
	for(i = 0; i < 200; i++) {
		mode = (g_video.g_a2_line_stat[i] >> 4) & 7;
		width = EFF_BORDER_WIDTH;
		if(mode == MODE_SUPER_HIRES) {
			width = BORDER_WIDTH;
		}
		if(width != old_width) {
			video_push_border_sides_lines(BORDER_WIDTH,
				X_A2_WINDOW_WIDTH - old_width, old_width,
				prev_line, i);
			prev_line = i;
			old_width = width;
		}
	}
	

	video_push_border_sides_lines(0/*BORDER_WIDTH*/,
		X_A2_WINDOW_WIDTH - old_width, old_width, prev_line, 200);
}

void
video_push_border_special()
{

	Kimage	*kimage_ptr;
	int	width, height;
	int	src_x, src_y;
	int	dest_x, dest_y;

	kimage_ptr = &s_video.g_kimage_border_special;
	width = g_video_act_width;
	g_refresh_bytes_xfer += width * (BASE_MARGIN_TOP + BASE_MARGIN_BOTTOM);

	// First do bottom border: dest_x from 0 to 640+MARGIN_LEFT+MARGIN_RIGHT
	//	and dest_y of BASE_MARGIN_BOTTOM starting at TOP+A2_HEIGHT
	//	src_x is dest_x, and src_y is 0.
	dest_y = g_video_act_margin_top + A2_WINDOW_HEIGHT;
	height = g_video_act_margin_bottom;
	src_y = BASE_MARGIN_BOTTOM - height;

	dest_x = 0;
	src_x = BASE_MARGIN_LEFT - g_video_act_margin_left;

	if(width > 0 && height > 0) {
		x_push_kimage(kimage_ptr, dest_x, dest_y, src_x, src_y,
								width, height);
	}

	// Then fix top border: dest_x from 0 to 640+LEFT+RIGHT and
	//	dest_y from 0 to TOP.  src_x is dest_x, but src_y is
	//	BOTTOM to BOTTOM+TOP
	//	Just use src_x and dest_x from earlier.
	height = g_video_act_margin_top;
	dest_y = 0;
	src_y = BASE_MARGIN_BOTTOM;
	if(width > 0 && height > 0) {
		x_push_kimage(kimage_ptr, dest_x, dest_y, src_x, src_y,
								width, height);
	}
}

// OG Added window ratio support
extern int x_calc_ratio(float&,float&);

void
video_push_kimages()
{
	register word32 start_time;
	register word32 end_time;
	Kimage	*last_kim, *cur_kim;
	word32	mask;
	int	start;
	int	line;
	int	left_pix, right_pix;
	int	left, right;
	int	line_div8;

	if(g_video.g_border_sides_refresh_needed) {
		g_video.g_border_sides_refresh_needed = 0;
		video_push_border_sides();
	}
	if(g_video.g_border_special_refresh_needed) {
		g_video.g_border_special_refresh_needed = 0;
		video_push_border_special();
	}

	if(g_video.g_a2_screen_buffer_changed == 0) {
		return;
	}

	GET_ITIMER(start_time);

	float ratiox,ratioy;

	if (x_calc_ratio(ratiox,ratioy))
	{
		line = 0;		
		while (1)
		{
			start = line;
			cur_kim = g_video.g_a2_line_kimage[line];		
			while(line < 200 && g_video.g_a2_line_kimage[line] == cur_kim) line++;	
			if (cur_kim == &s_video.g_kimage_superhires ||cur_kim == &s_video.g_kimage_border_special2 )
				right = cur_kim->width_act/2*VIDEO_HFACTOR;
			else
				right = 560/2*VIDEO_HFACTOR;
			
			video_push_lines(cur_kim, start, line,0,right);
			if (line==200) break;
		}
	
	}
	else
	{
		start = -1;
		last_kim = (Kimage *)-1;
		cur_kim = (Kimage *)0;

		left_pix = 640/2*VIDEO_HFACTOR;
		right_pix = 0;

		for(line = 0; line < 200; line++) {
			line_div8 = line >> 3;
			mask = 1 << (line_div8);
			cur_kim = g_video.g_a2_line_kimage[line];
			if((g_video.g_full_refresh_needed & mask) != 0) 
            {
				left = 0;
				
                if (cur_kim == &s_video.g_kimage_superhires ||cur_kim == &s_video.g_kimage_border_special2 )
                    right = cur_kim->width_act/2*VIDEO_HFACTOR;
                else
                    right = 560/2*VIDEO_HFACTOR;		
			} 
            else 
            {
				left = g_video.g_a2_line_left_edge[line];
				right = g_video.g_a2_line_right_edge[line];
			}

			if(!(g_video.g_a2_screen_buffer_changed & mask) || (left > right)) {
				/* No need to update this line */
				/* Refresh previous chunks of lines, if any */
				if(start >= 0) {
					video_push_lines(last_kim, start, line,
						left_pix, right_pix);
					start = -1;
					left_pix = 640/2*VIDEO_HFACTOR;
					right_pix = 0;
				}
			} else {
				/* Need to update this line */
				if(start < 0) {
					start = line;
					last_kim = cur_kim;
				}
				if(cur_kim != last_kim) {
					/* do the refresh */
					video_push_lines(last_kim, start, line,
						left_pix, right_pix);
					last_kim = cur_kim;
					start = line;
					left_pix = left;
					right_pix = right;
				}
				left_pix = MIN(left, left_pix);
				right_pix = MAX(right, right_pix);
			}
		}

		if(start >= 0) {
			video_push_lines(last_kim, start, 200, left_pix, right_pix);
		}
	}

	g_video.g_a2_screen_buffer_changed = 0;
	g_video.g_full_refresh_needed = 0;

//	refresh_video(0);
	//	x_push_done();

	GET_ITIMER(end_time);

	g_video.g_cycs_in_xredraw += (end_time - start_time);
}

void video_update_palette_color()
{
	for(int i = 0; i < 256; i++) 
	{
		int lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);	
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}
}

void
video_update_color_raw(int col_num, int a2_color)
{
	word32	tmp;
	int	red, green, blue;
	int	newred, newgreen, newblue;

	red = (a2_color >> 8) & 0xf;
	green = (a2_color >> 4) & 0xf;
	blue = (a2_color) & 0xf;
	red = ((red << 4) + red);
	green = ((green << 4) + green);
	blue = ((blue << 4) + blue);

	float gray =  (float)red*0.3f + (float)green*0.59f+ (float)blue*0.11f ;

    int cm = r_sim65816.get_color_mode();
    
	switch(cm)
	{
		case COLORMODE_BW:
        case COLORMODE_MONO:
			green = (int)gray ;
			red = (int)gray ;
			blue = (int)gray;
			break;
		case COLORMODE_AMBER:
			// It is a color that is 75% yellow (R+G) and 25% red. = 1R 
			green = (int) ( gray );
			blue = 0;
			red = (int) ( gray );
			break;
		case COLORMODE_GREEN:
			green = (int)gray;
			blue = 0;
			red = 0;
			break;
		default:
			break;
	}
	
	newred = red >> s_video.g_red_right_shift;
	newgreen = green >> s_video.g_green_right_shift;
	newblue = blue >> s_video.g_blue_right_shift;

	tmp = ((newred & s_video.g_red_mask) << s_video.g_red_left_shift) +
			((newgreen & s_video.g_green_mask) << s_video.g_green_left_shift) +
			((newblue & s_video.g_blue_mask) << s_video.g_blue_left_shift);

	
	
	g_palette_8to1624[col_num] = tmp;

	x_update_color(col_num, red, green, blue, tmp);
}

void
video_update_color_array(int col_num, int a2_color)
{
	int	palette;
	int	full;

	if(col_num >= 256 || col_num < 0) {
		halt_printf("video_update_color_array: col: %03x\n", col_num);
		return;
	}

	full = s_video.g_installed_full_superhires_colormap;

	palette = col_num >> 4;
	if(!full && palette == g_video.g_a2vid_palette) {
		return;
	}

#if 0
	if(s_video.g_screen_depth != 8) {
		/* redraw whole superhires for now */
		g_video.g_full_refresh_needed = -1;
	}
#endif

	video_update_color_raw(col_num, a2_color);
}

void
video_update_colormap()
{
	int	palette;
	int	full;
	int	i;

	full = s_video.g_installed_full_superhires_colormap;

	if(!full) {
		palette = g_video.g_a2vid_palette << 4;
		for(i = 0; i < 16; i++) {
			video_update_color_raw(palette + i, g_lores_colors[i]);
		}
		x_update_physical_colormap();
	}
}

void
video_update_status_line(int line, const char *string)
{
	char	*buf;
	const char *ptr;
	int	i;

	if(line >= MAX_STATUS_LINES || line < 0) {
		printf("update_status_line: line: %d!\n", line);
		x_exit(1);
	}

	ptr = string;
	buf = &(g_status_buf[line][0]);
	g_status_ptrs[line] = buf;
	for(i = 0; i < STATUS_LINE_LENGTH; i++) {
		if(*ptr) {
			buf[i] = *ptr++;
		} else {
			buf[i] = ' ';
		}
	}

	buf[STATUS_LINE_LENGTH] = 0;
}

void
video_show_debug_info()
{
	word32	tmp1;

	printf("g_cur_dcycs: %f, last_vbl: %f\n", g_sim65816.g_cur_dcycs,
							g_sim65816.g_last_vbl_dcycs);
	tmp1 = get_lines_since_vbl(g_sim65816.g_cur_dcycs);
	printf("lines since vbl: %06x\n", tmp1);
	printf("Last line updated: %d\n", g_video.g_vid_update_last_line);
}

word32
float_bus(double dcycs)
{
	word32	val;
	int	lines_since_vbl;
	int	line, eff_line, line24;
	int	all_stat;
	int	byte_offset;
	int	hires, page2;
	int	addr;

	lines_since_vbl = get_lines_since_vbl(dcycs);

/* For floating bus, model hires style: Visible lines 0-191 are simply the */
/* data being displayed at that time.  Lines 192-255 are lines 0 - 63 again */
/*  and lines 256-261 are lines 58-63 again */
/* For each line, figure out starting byte at -25 mod 128 bytes from this */
/*  line's start */
/* This emulates an Apple II style floating bus.  A reall IIgs does not */
/*  drive anything meaningful during the 25 horizontal blanking lines, */
/*  nor during veritical blanking.  The data seems to be 0 or related to */
/*  the instruction fetches on a real IIgs during blankings */

	line = lines_since_vbl >> 8;
	byte_offset = lines_since_vbl & 0xff;
	/* byte offset is from 0 to 65, where the visible screen is drawn */
	/*  from 25 to 65 */

	eff_line = line;
	if(line >= 192) {
		eff_line = line - 192;
		if(line >= 256) {
			eff_line = line - 262 + 64;
		}
	}
	all_stat = g_video.g_cur_a2_stat;
	hires = all_stat & ALL_STAT_HIRES;
	if((all_stat & ALL_STAT_MIX_T_GR) && (line >= 160)) {
		hires = 0;
	}
	page2 = EXTRU(all_stat, 31 - BIT_ALL_STAT_PAGE2, 1);
	if(all_stat & ALL_STAT_ST80) {
		page2 = 0;
	}

	line24 = (eff_line >> 3) & 0x1f;
	addr = g_screen_index[line24] & 0x3ff;
	addr = (addr & 0x380) + (((addr & 0x7f) - 25 + byte_offset) & 0x7f);
	if(hires) {
		addr = 0x2000 + addr + ((eff_line & 7) << 10) + (page2 << 13);
	} else {
		addr = 0x400 + addr + (page2 << 10);
	}

	val = g_slow_memory_ptr[addr];
	if(byte_offset < 10) {
		/* Bob Bishop's sample program seems to get confused by */
		/*  these bytes--so mask some off to prevent seeing some */
		val = 0;
	}
#if 0
	printf("For %04x (%d) addr=%04x, val=%02x, dcycs:%9.2f\n",
		lines_since_vbl, eff_line, addr, val, dcycs - g_last_vbl_dcycs);
#endif
	return val;
}

void refresh_video(int _force)
{

	if (!r_sim65816.is_emulator_offscreen_available() || r_sim65816.should_emulator_terminate())
	{
		printf("refresh_video cancelled : offscreen not available\n");
		return ;
	}
	
	addFrameRate(3);
	
	
	if (_force ||   g_video_offscreen_has_been_modified )
	{

		addFrameRate(1);

		// pour avoir une synchro vbl
		g_video_offscreen_has_been_modified = 1;
			
#ifdef ENABLE_GRAPH
	static double lastvbl=0.0; 
	double d = get_dtime();
	double delay = 1/(d - lastvbl);
	lastvbl = d;
	g_graph.add_graph(0,(float)delay,g_sim65816.g_vbl_count);

#endif

		x_refresh_video();
	}

}

void SET_LINES()
{
	for(int i=0;i<200;i++)	
	{					
		Kimage* p = g_video.g_a2_line_kimage[i];	
		if (p==(Kimage*)1)			
			p  =&s_video.g_kimage_text[0];				
		else	
		if (p==(Kimage*)2)
			p  =&s_video.g_kimage_text[1];
		else
		if (p==(Kimage*)3)
			p  =&s_video.g_kimage_hires[0];
		else
		if (p==(Kimage*)4 )
			p  =&s_video.g_kimage_hires[1];
		else
		if (p==(Kimage*)5)
			p  =&s_video.g_kimage_superhires;
		else
		if (p==(Kimage*)6)
			p  =&s_video.g_kimage_border_special;
		else
		if (p==(Kimage*)7)
			p  =&s_video.g_kimage_border_special2;
		else
		if (p==(Kimage*)8)
			p  =&s_video.g_kimage_border_sides;
		else
		if (p==(Kimage*)9)
			p  =&s_video.g_mainwin_kimage;
		else
		{
			// to fix
			p  =&s_video.g_kimage_superhires;
		}
		 g_video.g_a2_line_kimage[i]=p;
	}
}
void RESET_LINES()
{
	for(int i=0;i<200;i++)	
	{					
		Kimage* p = g_video.g_a2_line_kimage[i];	
		if (p==&s_video.g_kimage_text[0])			
			p  =(Kimage*)1;					
		else	
		if (p==&s_video.g_kimage_text[1])
			p  =(Kimage*)2;
		else
		if (p==&s_video.g_kimage_hires[0])
			p  =(Kimage*)3;
		else
		if (p==&s_video.g_kimage_hires[1])
			p  =(Kimage*)4;
		else
		if (p==&s_video.g_kimage_superhires)
			p  =(Kimage*)5;
		else
		if (p==&s_video.g_kimage_border_special)
			p  =(Kimage*)6;
		else
		if (p==&s_video.g_kimage_border_special2)
			p  =(Kimage*)7;
		else
		if (p==&s_video.g_kimage_border_sides)
			p  =(Kimage*)8;
		else
		if (p==&s_video.g_mainwin_kimage)
			p  =(Kimage*)9;
		else
		{
			// to fix
			p=(Kimage*)5;
		}
		 g_video.g_a2_line_kimage[i]=p;
	}
}


#define NBSECSAMPLING 2.0f 
int fpsNbFrames[NBFPSTIMER];
double fpsLastSecond[NBFPSTIMER];
int fpsValues[NBFPSTIMER];

void addFrameRate(int _id)
{
	
	double timer = get_dtime();
	double delay = timer - fpsLastSecond[_id];
	if (delay>NBSECSAMPLING)
	{
		fpsValues[_id] = (int)((float)(fpsNbFrames[_id]*10.0)/NBSECSAMPLING);
		fpsNbFrames[_id]=0;
		fpsLastSecond[_id]=timer;
	}
	fpsNbFrames[_id]++;
//	printf("%d-",_id);
}

int getFrameRate(int _id)
{
	return fpsValues[_id];
}
