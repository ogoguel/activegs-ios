/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

extern void RESET_LINES();
extern void SET_LINES();

#define MAX_BORDER_CHANGES	16384

#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

STRUCT(Border_changes)
{
	float	fcycs;
	int	val;		
} XCODE_PRAGMA_PACK;;


class sd_video : public serialized
{
public:
	int g_a2_line_stat[200];
	int g_a2_line_left_edge[200];
	int g_a2_line_right_edge[200];
	Kimage *g_a2_line_kimage[200];

	int g_mode_text[2][200];
	int g_mode_hires[2][200];
	int g_mode_superhires[200];
	int g_mode_border[200];

	byte g_cur_border_colors[270];
	byte g_new_special_border[64][64];
	byte g_cur_special_border[64][64];

	word32	g_a2_screen_buffer_changed ;
	word32	g_full_refresh_needed;

	word32 g_cycs_in_40col;
	word32 g_cycs_in_xredraw ;

	


	word32 g_superhires_scan_save[256];

	double	g_video_dcycs_check_input ;
	int	g_video_extra_check_inputs ;	
	
	/*
	int	g_video_act_margin_left;
	int	g_video_act_margin_right;
	int	g_video_act_margin_top;
	int	g_video_act_margin_bottom ;
	int	g_video_act_width;
	int	g_video_act_height ;
	 */
	int	g_need_redraw ;
	int	g_palette_change_summary ;
	word32	g_palette_change_cnt[16];
	int	g_border_sides_refresh_needed ;
	int	g_border_special_refresh_needed ;
	int	g_border_line24_refresh_needed ;
	int	g_status_refresh_needed ;

	int	g_vbl_border_color ;
	int	g_border_last_vbl_changes ;

	int	g_use_dhr140;

	int	g_a2_new_all_stat[200];
	int	g_a2_cur_all_stat[200];
	int	g_new_a2_stat_cur_line ;
	int	g_vid_update_last_line ;



	int g_cur_a2_stat ;

	int	g_a2vid_palette ;
	
	int	Max_color_size ;


	
	int		g_saved_a2vid_palette ;
	word32	g_a2vid_palette_remap[16];

	word32 g_cycs_in_refresh_line ;
	word32 g_cycs_in_refresh_ximage ;

	int	g_num_lines_superhires ;
	int	g_num_lines_superhires640;
	int	g_num_lines_prev_superhires ;
	int	g_num_lines_prev_superhires640 ;

	int	g_flash_count ;

	int	g_screen_redraw_skip_count ;
	int	g_screen_redraw_skip_amt ;

	word32	g_cycs_in_check_input ;

	int g_needfullrefreshfornextframe  ;	

	int g_num_a2vid_palette_checks ;
	int g_shr_palette_used[16];

	int g_border_color;

	Border_changes g_border_changes[MAX_BORDER_CHANGES];
	int	g_num_border_changes;


#define RESET_KIMAGE(X,N)	byte* old_##N = X.data_ptr; X.data_ptr=NULL;
#define SET_KIMAGE(X,N)	X.data_ptr =  old_##N ;


#define RESET_KIMAGES	\
	RESET_KIMAGE(g_kimage_text[0],g_kimage_text0);	\
	RESET_KIMAGE(g_kimage_text[1],g_kimage_text1);	\
	RESET_KIMAGE(g_kimage_hires[0],g_kimage_hires0);	\
	RESET_KIMAGE(g_kimage_hires[1],g_kimage_hires1);	\
	RESET_KIMAGE(g_kimage_superhires,g_kimage_superhires);	\
	RESET_KIMAGE(g_kimage_border_special,g_kimage_border_special);	\
	RESET_KIMAGE(g_kimage_border_special2,g_kimage_border_special2);	\
	RESET_KIMAGE(g_kimage_border_sides,g_kimage_border_sides); \
	RESET_KIMAGE(g_mainwin_kimage,g_mainwin_kimage);
	


#define SET_KIMAGES	\
	SET_KIMAGE(g_kimage_text[0],g_kimage_text0);	\
	SET_KIMAGE(g_kimage_text[1],g_kimage_text1);	\
	SET_KIMAGE(g_kimage_hires[0],g_kimage_hires0);	\
	SET_KIMAGE(g_kimage_hires[1],g_kimage_hires1);	\
	SET_KIMAGE(g_kimage_superhires,g_kimage_superhires);	\
	SET_KIMAGE(g_kimage_border_special,g_kimage_border_special);	\
	SET_KIMAGE(g_kimage_border_special2,g_kimage_border_special2);	\
	SET_KIMAGE(g_kimage_border_sides,g_kimage_border_sides); \
	SET_KIMAGE(g_mainwin_kimage,g_mainwin_kimage);



	sd_video()
	{
		INIT_SERIALIZED();

		g_video_extra_check_inputs=1;

		g_a2_screen_buffer_changed = (word32)-1;
		g_full_refresh_needed = (word32)-1;
/*
		g_video_act_margin_left = BASE_MARGIN_LEFT;
		g_video_act_margin_right = BASE_MARGIN_RIGHT;
		g_video_act_margin_top = BASE_MARGIN_TOP;
		g_video_act_margin_bottom = BASE_MARGIN_BOTTOM;
		g_video_act_width = X_A2_WINDOW_WIDTH;
		g_video_act_height = X_A2_WINDOW_HEIGHT;
*/
		g_need_redraw = 1;

		g_border_sides_refresh_needed = 1;
		g_border_special_refresh_needed = 1;
		g_border_line24_refresh_needed = 1;
		g_status_refresh_needed = 1;
	
		g_cur_a2_stat = ALL_STAT_TEXT | ALL_STAT_ANNUNC3 | (0xf << BIT_ALL_STAT_TEXT_COLOR);
		g_a2vid_palette = 0xe;
		Max_color_size = 256;
		g_saved_a2vid_palette = -1;

		g_screen_redraw_skip_amt = -1;
		g_needfullrefreshfornextframe = 1 ;	
		g_num_a2vid_palette_checks = 1;
	}
	
	void in(serialize* _s)
	{
	
		RESET_LINES();

		byte* ptr = (byte*)_s->data;

		int ssize = *(int*)ptr;
		if (ssize!=sizeof(*this))
			x_fatal_exit("alignment mismatch");
		ptr+=sizeof(ssize);
		memcpy(this,ptr,sizeof(*this));

		SET_LINES();
		g_needfullrefreshfornextframe = 1 ;	

	}

	int out(serialize* _s, int _fastalloc)
	{
		int ssize= sizeof(*this);
		int size = ssize + sizeof(ssize);
		_s->size = size;
		_s->fastalloc = _fastalloc;
		_s->data = (void*)x_malloc(size,_fastalloc);

		RESET_LINES();
		byte* ptr = (byte*)_s->data;
		
		memcpy(ptr,&ssize,sizeof(ssize));
		ptr+=sizeof(ssize);
		memcpy(ptr,this,sizeof(*this));
		SET_LINES();
		
	
		return size;
	}
} XCODE_PRAGMA_PACK;


#ifdef _WIN32
#pragma pack(pop)
#endif


extern sd_video g_video;

extern int g_video_act_margin_left ;
extern int g_video_act_margin_right;
extern int g_video_act_margin_top ;
extern int g_video_act_margin_bottom ;
extern int g_video_act_width ;
extern int g_video_act_height ;


class ss_video
{
public:
	int g_screen_mdepth;
	int	g_screen_depth;
	word32	g_red_mask;
	word32	g_green_mask ;
	word32	g_blue_mask ;
	int	g_red_left_shift;
	int	g_green_left_shift ;
	int	g_blue_left_shift;
	int	g_red_right_shift ;
	int	g_green_right_shift ;
	int	g_blue_right_shift ;
	int	g_installed_full_superhires_colormap ;
	word32	g_saved_line_palettes[200][8];
	word32 slow_mem_changed[SLOW_MEM_CH_SIZE];


	Kimage g_kimage_text[2];
 	Kimage g_kimage_hires[2];
	Kimage g_kimage_superhires;
    Kimage g_kimage_border_special;
    Kimage g_kimage_border_special2;
	Kimage g_kimage_border_sides;

	Kimage g_mainwin_kimage;
     

	ss_video()
	{	
		memset(this,0,sizeof(*this));
		g_screen_depth = 8;
		g_red_mask = 0xff;
		g_green_mask = 0xff;
		g_blue_mask = 0xff;
		g_red_left_shift = 16;
		g_green_left_shift = 8;
		g_blue_left_shift = 0;
		g_red_right_shift = 0;
		g_green_right_shift = 0;
		g_blue_right_shift = 0;
	}
} ;

extern	ss_video s_video;

extern Kimage g_kimage_offscreen;
extern int g_video_offscreen_has_been_modified ;

void refresh_video(int _forcerefresh);

void init_kimage(Kimage *kimage_ptr, int extend_info, int depth, int mdepth);

#define NBFPSTIMER 5
extern void addFrameRate(int );
extern int getFrameRate(int );
