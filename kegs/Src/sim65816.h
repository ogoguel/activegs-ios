/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#define START_DCYCS	(0.0)
#define MAX_EVENTS	64

extern byte *g_memory_ptr;
extern byte *g_slow_memory_ptr;


enum {
    OFFSCREEN_AVAILABLE=256,
	NO_INITIALIZED = 0,
	INITIALIZING = 1,
	RUNNING = 2 + OFFSCREEN_AVAILABLE,
//	IN_REWIND = 3 + OFFSCREEN_AVAILABLE,
	IN_PAUSE = 4 + OFFSCREEN_AVAILABLE,
    PAUSE_REQUESTED = 5 + OFFSCREEN_AVAILABLE,
	SHUTTING_DOWN = 5
};

typedef enum _colormode
{
	COLORMODE_AUTO = 100,
	COLORMODE_BW,
	COLORMODE_AMBER,
	COLORMODE_GREEN,
    COLORMODE_MONO
} colormodeenum;

typedef enum _videofx
{
	VIDEOFX_LCD = 200,
	VIDEOFX_CRT
} videofxenum;


enum speedenum
{
	SPEED_UNLIMITED = 0,
	SPEED_1MHZ = 1,
	SPEED_GS = 2,		// 2.8 Mhz
	SPEED_ZIP = 3,		// 8 Mhz Zip speed	
	SPEED_ENUMSIZE=4,
	SPEED_AUTO = 4,	
};

class run_sim65816
{
	int g_savestate_enable;
	
public:
	int emulator_state;
	int quit_requested;
	int restart_requested;
	colormodeenum color_mode;
	videofxenum video_fx;
    
	int	g_user_halt_bad ;
	int	g_halt_on_bad_read ;
	int	g_ignore_bad_acc;
	int	g_ignore_halts ;
	
	Page_info page_info_rd_wr[2*65536 + PAGE_INFO_PAD_SIZE];

	run_sim65816()
	{
		memset(this,0,sizeof(*this));
		emulator_state = NO_INITIALIZED;
		color_mode = COLORMODE_AUTO;
		video_fx = VIDEOFX_LCD;
		g_ignore_bad_acc = 1;
		g_ignore_halts = 1;

	}
	
	// Emulator COnfig


	videofxenum	get_video_fx() { return video_fx; }
	void set_video_fx(videofxenum _vfx);
	colormodeenum get_color_mode() { return color_mode; }
	void set_color_mode(colormodeenum _cm);
	int	is_emulator_initialized();
	int	emulator_config_requested; 
	void request_update_emulator_runtime_config();
	void check_update_emulator_runtime_config();

	// Emulator State

	void set_rewind_enable(int _onoff);
	int	get_rewind_enable() { return g_savestate_enable; };

	void set_state(int _newstate);
	int get_state();
	void quit_emulator() ;
	void restart_emulator();
	void reset_quit();
	void reset_restart();

	int pause();
	int resume();

	// Emulator Info

	int is_emulator_in_640mode();
	int is_emulator_offscreen_available();
	int should_emulator_terminate();
	
};

extern run_sim65816 r_sim65816;
extern void unserialize_engine(Engine_reg& engine);
extern void serialize_engine(Engine_reg& engine);

#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif
class s_sim65816 : public serialized
{

public:
	enum speedenum	g_limit_speed ;

	int get_limit_speed() { return (int)g_limit_speed; }
	enum speedenum set_limit_speed(enum speedenum _speed);

	int g_speed_fast ;	// OG Expose fast parameter
	Engine_reg engine;
	_ALIGNED(8) double	g_fcycles_stop;
	volatile int	halt_sim ;
	int	enter_debug;
	int	g_rom_version ;
	int	g_code_red ;
	int	g_code_yellow ;
	int	g_use_alib ;
	int	g_raw_serial ;
	int	g_iw2_emul ;
	int	g_serial_out_masking ;
	int	g_serial_modem[2];

	int	g_config_iwm_vbl_count;

	_ALIGNED(8) double	g_last_vbl_dcycs ;
	_ALIGNED(8) double	g_cur_dcycs ;

	_ALIGNED(8) double	g_last_vbl_dadjcycs ;
	_ALIGNED(8) double	g_dadjcycs;

	int	g_wait_pending ;
	int	g_stp_pending ;

	int	g_num_irq ;
	int	g_num_brk ;
	int	g_num_cop ;
	int	g_num_enter_engine ;
	int	g_io_amt ;
	int	g_engine_action ;
	int	g_engine_halt_event;
	int	g_engine_scan_int ;
	int	g_engine_doc_int ;

	int	g_testing ;
	int	g_testing_enabled ;

	word32 stop_run_at;

	int g_25sec_cntr ;
	int g_1sec_cntr ;

	_ALIGNED(8) double g_dnatcycs_1sec ;
	word32 g_natcycs_lastvbl ;

//	int Verbose ;
	int Halt_on ;

	word32 g_mem_size_base ;
	word32 g_mem_size_exp ;
	word32 g_mem_size_total ;


	word32	g_vbl_count;
	word32	g_vbl_count_since_lastvblank;
	int		g_fixed_vblank;

	void set_fixed_vblank(int _nbframe,speedenum _defaultspeed);

	int	g_vbl_index_count;
	_ALIGNED(8) double	dtime_array[60];
	_ALIGNED(8) double	g_dadjcycs_array[60];
	_ALIGNED(8) double	g_dtime_diff3_array[60];
	_ALIGNED(8) double	g_dtime_this_vbl_array[60];
	_ALIGNED(8) double	g_dtime_exp_array[60];
	_ALIGNED(8) double	g_dtime_pmhz_array[60];
	_ALIGNED(8) double	g_dtime_eff_pmhz_array[60];
	_ALIGNED(8) double	sim_time[60];
	_ALIGNED(8) double	g_sim_sum ;

	_ALIGNED(8) double	g_cur_sim_dtime ;
	_ALIGNED(8) double	g_projected_pmhz;
	_ALIGNED(8) double	g_zip_pmhz ;
	_ALIGNED(8) double	g_sim_mhz ;
	int	g_line_ref_amt ;
	int	g_video_line_update_interval ;

	Fplus	g_recip_projected_pmhz_slow;
	Fplus	g_recip_projected_pmhz_fast;
	Fplus	g_recip_projected_pmhz_zip;
	Fplus	g_recip_projected_pmhz_unl;

	_ALIGNED(8) double	g_dtime_last_vbl ;
	_ALIGNED(8) double	g_dtime_expected ;

	int g_scan_int_events ;

	char g_display_env[512];
	int	g_force_depth;

	Event g_event_list[MAX_EVENTS];
	Event g_event_free;
	Event g_event_start;

//	Page_info page_info_rd_wr[2*65536 + PAGE_INFO_PAD_SIZE];

	// OG unsupported 6502 opcodes are turned into NOP
	int g_6502_emulation ;
	int g_use_apple2e_rom ;
    int g_enable_disk_sound;


	s_sim65816()
	{
		INIT_SERIALIZED();

		g_rom_version = -1;
	//	g_ignore_bad_acc = 1;
	//	g_ignore_halts = 1;
		g_raw_serial = 1;
		g_serial_modem[1] = 1;

		g_last_vbl_dcycs = START_DCYCS;
		g_cur_dcycs = START_DCYCS;

		g_mem_size_base = 256*1024;	/* size of motherboard memory */
		g_mem_size_exp = 2*1024*1024;	/* size of expansion RAM card */
		g_mem_size_total = 256*1024;	/* Total contiguous RAM from 0 */

		g_projected_pmhz = 1.0;
		g_zip_pmhz = 8.0;
		g_sim_mhz = 100.0;
		g_line_ref_amt = 1;

		g_dtime_expected = (1.0/60.0);

		g_force_depth = -1;
        g_limit_speed = SPEED_GS;

		
	}
	#define RECALC_BASE_EVENT_PTR(X,BASE) \
		if (X)	\
		{ \
			X  = (Event*)(  (intptr_t)X +  (intptr_t)BASE )  ; \
		}

	void in(serialize* _s)
	{
		extern byte* g_dummy_memory1_ptr;

		byte* ptr = (byte*)_s->data;
		int ssize = *(int*)ptr;
		if (ssize!=sizeof(*this))
			ALIGNMENT_ERROR("sim65816",ssize,sizeof(*this));
		ptr+=4;
		memcpy(this,_s->data,sizeof(*this));

		unserialize_engine(engine);
		ptr += sizeof(*this);

		
		memcpy(g_slow_memory_ptr,ptr,128*1024);
		ptr+=128*1024;
		memcpy(g_dummy_memory1_ptr,ptr,256);
		ptr+=256;
		memcpy(g_memory_ptr,ptr,g_mem_size_total);


		intptr_t base = (intptr_t)g_event_list +1;	// pour ne pas commencer à zero!
		for(int i = 1; i < MAX_EVENTS; i++) 
		{
			RECALC_BASE_EVENT_PTR(g_event_list[i-1].next,+base);
		}
		RECALC_BASE_EVENT_PTR(g_event_free.next,+base);
		RECALC_BASE_EVENT_PTR(g_event_start.next,+base);

		g_dtime_last_vbl += get_dtime();
		g_dtime_expected  += get_dtime();

		
	}


	int out(serialize* _s,int _fastalloc)
	{
extern byte* g_dummy_memory1_ptr;
	
		int ssize = sizeof(*this);
		int size = sizeof(ssize);
		size += ssize;
		size += 128*1024;	// slow memory
		size += 256; // dummy
		size += g_mem_size_total;	
		

		_s->size = size;
		_s->fastalloc = _fastalloc;
		_s->data = (void*)x_malloc(size,_fastalloc);

		intptr_t base = (intptr_t)g_event_list +1;	// pour ne pas commencer à zero!

		// reinitialize les event avant de les copier
		for(int i = 1; i < MAX_EVENTS; i++) 
		{
			RECALC_BASE_EVENT_PTR(g_event_list[i-1].next,-base);
		}
		RECALC_BASE_EVENT_PTR(g_event_free.next,-base);
		RECALC_BASE_EVENT_PTR(g_event_start.next,-base);

		serialize_engine(engine);
		double old_g_dtime_last_vbl = g_dtime_last_vbl;
		g_dtime_last_vbl -= get_dtime();
		double old_g_dtime_expected = g_dtime_expected;
		g_dtime_expected -= get_dtime();

		byte* ptr = (byte*)_s->data;
		memcpy(ptr,&ssize,sizeof(ssize));
		ptr+=sizeof(ssize);
		memcpy(_s->data,this,sizeof(*this));
		ptr += sizeof(*this);

		unserialize_engine(engine);
		g_dtime_last_vbl = old_g_dtime_last_vbl;
		g_dtime_expected = old_g_dtime_expected;

		
		memcpy(ptr,g_slow_memory_ptr,128*1024);
		ptr+= 128*1024;
		memcpy(ptr,g_dummy_memory1_ptr,256);
		ptr+=256;
		memcpy(ptr,g_memory_ptr,g_mem_size_total);
	
		// reinitialize les event avant de les copier
		for(int i = 1; i < MAX_EVENTS; i++) 
		{
			RECALC_BASE_EVENT_PTR(g_event_list[i-1].next,base);
		}
		RECALC_BASE_EVENT_PTR(g_event_free.next,base);
		RECALC_BASE_EVENT_PTR(g_event_start.next,base);

		g_dtime_last_vbl += get_dtime();		

		return size;

	}
} XCODE_PRAGMA_PACK;
#ifdef _WIN32
#pragma pack(pop)
#endif


extern void x_update_info(const char* total_mhz_ptr);

extern s_sim65816 g_sim65816;
extern int Verbose;
//extern int g_pause;


/* All EV_* must be less than 256, since upper bits reserved for other use */
/*  e.g., DOC_INT uses upper bits to encode oscillator */
#define EV_60HZ		1
#define EV_STOP		2
#define EV_SCAN_INT	3
#define EV_DOC_INT	4
#define EV_VBL_INT	5
#define EV_SCC		6
#define EV_VID_UPD	7
#define EV_SCC_ENABLE	8