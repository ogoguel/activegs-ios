/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef _WIN32
#	define _WINSOCKAPI_  
#endif

#undef printf

#include <math.h>

#define INCLUDE_RCSID_C
#include "defc.h"
#undef INCLUDE_RCSID_C
#include "raster.h"
#include "protos_engine_c.h"
#include "driver.h"

// 2.6 = 60
// 2.49 = 63
// 2.45 = 64
// 2.43 = 64
// 2.41 = 65! ***

#define DEFAULT_GS_SPEED 2.5	//(2.41*(2.8/2.322)) 


#include "moremem.h"
#include "sim65816.h"
#include "iwm.h"
#include "config.h"
#include "sound.h"
#include "video.h"
#include "saveState.h"
#include "graphcounter.h"
#include "marinetti.h"


s_sim65816 g_sim65816;
run_sim65816 r_sim65816;

void run_sim65816::set_state(int _newstate)
{
	int s =  _newstate ;
	printf("state changed from %d to %d\n",r_sim65816.emulator_state,s);
	emulator_state = s;
}

int run_sim65816::get_state()  
{
	return emulator_state;
}

void run_sim65816::set_rewind_enable(int  _onoff)
{
	g_savestate_enable = _onoff;

}


//OG for regular exit, use quitEmulator()
void run_sim65816::quit_emulator()
{
	printf("*** quit_emulator ***\n");	
	quit_requested = 1;
}

void run_sim65816::restart_emulator()
{
	printf("*** quit_emulator ***\n");	
	restart_requested = 1;
}


void run_sim65816::reset_quit()
{
	printf("*** reset_quit ***\n");	
	quit_requested = 0;
}

void run_sim65816::reset_restart()
{
	printf("*** reset_restart ***\n");	
	restart_requested = 0;
}

int run_sim65816::should_emulator_terminate()
{
	return (quit_requested || restart_requested);
}

int run_sim65816::is_emulator_offscreen_available()
{
	return ( emulator_state & OFFSCREEN_AVAILABLE );
}

int run_sim65816::is_emulator_in_640mode()
{
	
	return g_video.g_a2_line_kimage[0]==&s_video.g_kimage_superhires && (g_slow_memory_ptr[0x19d00]&0x80) ;
}
void refresh_offscreen()
{
	    printf("*** forcing offscreen refresh! ***\n");
		// recalcul la palette
		r_sim65816.check_update_emulator_runtime_config();
		// regénère l'écran
        g_video.g_screen_redraw_skip_count = 0; // pour forcer le refresh indŽpendamment du frame rate effectif
		video_update();
		// force son rafraichissemnt
		// OG NE PAS FAIRE LE REFRESH ICI CAR ON NE SAIT PAS D'OU VIENT LA DEMANDE!!!
		// refresh_video(1);
}


void run_sim65816::set_color_mode(colormodeenum _cm)
{
	color_mode = _cm;
	printf("color_mode set to %d\n",_cm);
	g_video.g_needfullrefreshfornextframe = 1 ;
	request_update_emulator_runtime_config();

	if (r_sim65816.get_state()==IN_PAUSE)
		refresh_offscreen();
      
 
	
}


void run_sim65816::set_video_fx(videofxenum _vfx)
{
	video_fx = _vfx;
	g_video.g_needfullrefreshfornextframe = 1 ;
    printf("video fx set to %d\n",_vfx);
#ifdef DRIVER_IOS
    {
        extern  void x_set_video_fx(videofxenum _vfx);
        x_set_video_fx(_vfx);
    }
#endif
}


int  run_sim65816::is_emulator_initialized()
{
	int s = r_sim65816.get_state();
	return (s==RUNNING) || (s==IN_PAUSE);
}

void run_sim65816::request_update_emulator_runtime_config()
{
	emulator_config_requested=1;
}

void run_sim65816::check_update_emulator_runtime_config()
{
	
	if (!emulator_config_requested)
		return ;
    
	emulator_config_requested=0;

	extern void video_update_palette_color();

	if (color_mode == COLORMODE_BW)
		g_video.g_cur_a2_stat |= ALL_STAT_COLOR_C021;
	else
		g_video.g_cur_a2_stat &= ~ALL_STAT_COLOR_C021;

	// recalcul les palettes
	video_update_palette_color();

	for(int i=0;i<200;i++)
		for(int j=0;j<8;j++)
			s_video.g_saved_line_palettes[i][j] = -1;

	// remets à jour les borders (cf. change_border_color)
	g_video.g_border_last_vbl_changes = 1;
	for(int i=0;i<270;i++)
		g_video.g_cur_border_colors[i] = -1;
		
	
	// invalide la mémoire
	for(int i = 0;i<SLOW_MEM_CH_SIZE;i++)
		s_video.slow_mem_changed[i] = -1;

	// Disable GS Detection in pure 6502 mode

	extern byte *g_rom_fc_ff_ptr;
	if (g_rom_fc_ff_ptr && g_sim65816.g_rom_version == 1)
		{
		
			// replace LDA FB55 par LDA C044
			g_rom_fc_ff_ptr[0x03fe20]=0x44;
			g_rom_fc_ff_ptr[0x03fe21]=0xC0;

		if (g_sim65816.g_6502_emulation)
			g_rom_fc_ff_ptr[0x03fe28]=0x38;	// SEC
		else
			g_rom_fc_ff_ptr[0x03fe28]=0x18;	// CLC

		}
	
	g_video.g_needfullrefreshfornextframe = 1;

}

int run_sim65816::pause()
{
	if (get_state() == RUNNING)
	{
		set_state(PAUSE_REQUESTED);
		return 1;
	}
    else
    if (get_state() == PAUSE_REQUESTED)
        printf("pause already requested");
    else
        printf("cannot pause\n'");
	return 0;
}


int run_sim65816::resume()
{
	if (get_state() == IN_PAUSE || get_state() == PAUSE_REQUESTED)
	{
		set_state(RUNNING);
		return 1;
	}
	

	printf("cannot resume\n'");
	return 0;
}


int Verbose=0;

#ifdef UNDER_CE
#define vsnprintf _vsnprintf
#endif


#define PC_LOG_LEN	(8*1024)

char g_argv0_path[256] = "./";

const char *g_kegs_default_paths[] = { "", "./", "${HOME}/",
	"${HOME}/Library/KEGS/",
	"${0}/Contents/Resources/", "/usr/local/lib/",
	"/usr/local/kegs/", "/usr/local/lib/kegs/", "/usr/share/kegs/",
	"/var/lib/", "/usr/lib/kegs/", "${0}/", 0 };




//#define	EV_DELAY_WRITE_C023 8

extern int g_stepping;

//extern int g_cur_a2_stat;


extern int g_engine_c_mode;
extern int defs_instr_start_8;
extern int defs_instr_start_16;
extern int defs_instr_end_8;
extern int defs_instr_end_16;
extern int op_routs_start;
extern int op_routs_end;


void U_STACK_TRACE();

extern word32 g_cycs_in_sound1;
extern word32 g_cycs_in_sound2;
extern word32 g_cycs_in_sound3;
extern word32 g_cycs_in_sound4;
extern word32 g_cycs_in_start_sound;
extern word32 g_cycs_in_est_sound;
extern word32 g_refresh_bytes_xfer;

extern int g_num_snd_plays;
extern int g_num_doc_events;
extern int g_num_start_sounds;
extern int g_num_scan_osc;
extern int g_num_recalc_snd_parms;
extern float g_fvoices;



//extern int g_status_refresh_needed;

// OG Missing defined below
//extern int	g_screen_depth ;


//extern int g_screen_redraw_skip_amt;
#ifdef DRIVER_LINUX
int g_use_shmem = 0;
#endif
//extern int g_use_dhr140;
//extern int g_use_bw_hires;


#include "kegsversion.h"



#define MAX_FATAL_LOGS		20



byte *g_slow_memory_ptr = 0;
byte *g_memory_ptr = 0;
byte *g_dummy_memory1_ptr = 0;
byte *g_rom_fc_ff_ptr = 0;
byte *g_rom_cards_ptr = 0;

// OG Added allocated pointers
byte *g_slow_memory_ptr_allocated = 0;
byte *g_memory_ptr_allocated = 0;
byte *g_dummy_memory1_ptr_allocated = 0;
byte *g_rom_fc_ff_ptr_allocated = 0;
byte *g_rom_cards_ptr_allocated = 0;

void *g_memory_alloc_ptr = 0;		/* for freeing memory area */

//Page_info page_info_rd_wr[2*65536 + PAGE_INFO_PAD_SIZE];

// Not saved == Debug only

int	g_debug_file_fd = -1;
int	g_fatal_log = -1;
char *g_fatal_log_strs[MAX_FATAL_LOGS];


Pc_log g_pc_log_array[PC_LOG_LEN + 2];
Data_log g_data_log_array[PC_LOG_LEN + 2];

Pc_log	*g_log_pc_ptr = &(g_pc_log_array[0]);
Pc_log	*g_log_pc_start_ptr = &(g_pc_log_array[0]);
Pc_log	*g_log_pc_end_ptr = &(g_pc_log_array[PC_LOG_LEN]);

Data_log *g_log_data_ptr = &(g_data_log_array[0]);
Data_log *g_log_data_start_ptr = &(g_data_log_array[0]);
Data_log *g_log_data_end_ptr = &(g_data_log_array[PC_LOG_LEN]);

#define TOOLBOX_LOG_LEN		64

int g_toolbox_log_pos = 0;
word32 g_toolbox_log_array[TOOLBOX_LOG_LEN][8];



// OG Added sim65816_initglobals()
void sim65816_initglobals()
{

	g_sim65816.g_fcycles_stop = 0.0;
	g_sim65816.halt_sim = 0;
	g_sim65816.enter_debug = 0;
	g_sim65816.g_rom_version = -1;
//	g_sim65816.g_user_halt_bad = 0;
//	g_sim65816.g_halt_on_bad_read = 0;
//	g_sim65816.g_ignore_bad_acc = 1
//	g_sim65816.g_ignore_halts = 1;
	g_sim65816.g_code_red = 0;
	g_sim65816.g_code_yellow = 0;
	g_sim65816.g_use_alib = 0;
	g_sim65816.g_raw_serial = 1;
	g_sim65816.g_iw2_emul = 0;
	g_sim65816.g_serial_out_masking = 0;
	//g_sim65816.g_serial_modem[2] = { 0, 1 };

	g_sim65816.g_config_iwm_vbl_count = 0;

//	g_pause=0;	

	g_sim65816.g_last_vbl_dcycs = START_DCYCS;
	g_sim65816.g_cur_dcycs = START_DCYCS;

	g_sim65816.g_last_vbl_dadjcycs = 0.0;
	g_sim65816.g_dadjcycs = 0.0;


	g_sim65816.g_wait_pending = 0;
	g_sim65816.g_stp_pending = 0;

	g_sim65816.g_num_irq = 0;
	g_sim65816.g_num_brk = 0;
	g_sim65816.g_num_cop = 0;
	g_sim65816.g_num_enter_engine = 0;
	g_sim65816.g_io_amt = 0;
	g_sim65816.g_engine_action = 0;
	g_sim65816.g_engine_halt_event = 0;
	g_sim65816.g_engine_scan_int = 0;
	g_sim65816.g_engine_doc_int = 0;

	g_sim65816.g_testing = 0;
	g_sim65816.g_testing_enabled = 0;

	g_debug_file_fd = -1;
	g_fatal_log = -1;

	 g_sim65816.g_25sec_cntr = 0;
	g_sim65816.g_1sec_cntr = 0;

	 g_sim65816.g_dnatcycs_1sec = 0.0;
	g_sim65816.g_natcycs_lastvbl = 0;

//	 g_sim65816.Verbose = 0;	// OG Should not be reseted
	 g_sim65816.Halt_on = 0;

	 g_sim65816.g_mem_size_base = 256*1024;	/* size of motherboard memory */
	 g_sim65816.g_mem_size_total = 256*1024;	/* Total contiguous RAM from 0 */



	 // OG Added missing initializers 120710
//	g_sim65816.g_force_depth = -1; // OG Should be no reseted
//	g_screen_depth = 8; // OG Should be no reseted

	
	g_sim65816.g_vbl_count = 0;
    g_sim65816.g_vbl_count_since_lastvblank = 0;
    
#if ! (defined(DRIVER_IOS) || defined(DRIVER_OSX) ) || defined(ACTIVEGSPLUGIN) // AsyncEvent not yet available on Mac
	// OG reinitializing async events when emulator resets
	extern void reset_async_event();
	reset_async_event();
#endif
	
	g_sim65816.g_vbl_index_count = 0;
//	g_sim65816.g_limit_speed = 0;		// OG Should be no reseted
	g_sim65816.g_sim_sum = 0.0;

	g_sim65816.g_cur_sim_dtime = 0.0;
	g_sim65816.g_projected_pmhz = 1.0;
	g_sim65816.g_zip_pmhz = 8.0;
	g_sim65816.g_sim_mhz = 100.0;
	g_sim65816.g_line_ref_amt = 1;
	g_sim65816.g_video_line_update_interval = 0;

	g_sim65816.g_dtime_last_vbl = 0.0;
	g_sim65816.g_dtime_expected = (1.0/60.0);

	g_sim65816.g_scan_int_events = 0;


}

void
show_pc_log()
{
	FILE *pcfile;
	Pc_log	*log_pc_ptr;
	Data_log *log_data_ptr;
	double	dcycs;
	double	start_dcycs;
	word32	instr;
	word32	psr;
	word32	acc, xreg, yreg;
	word32	stack, direct;
	word32	dbank;
	word32	kpc;
	int	data_wrap;
	int	accsize, xsize;
	int	num;
	int	i;

	pcfile = fopen("pc_log_out", "w");
	if(pcfile == 0) {
		fprintf(stderr,"fopen failed...errno: %d\n", errno);
		x_exit(2);
	}

	log_pc_ptr = g_log_pc_ptr;
	log_data_ptr = g_log_data_ptr;
#if 0
	fprintf(pcfile, "current pc_log_ptr: %p, start: %p, end: %p\n",
		log_pc_ptr, log_pc_start_ptr, log_pc_end_ptr);
#endif

	start_dcycs = log_pc_ptr->dcycs;
	dcycs = start_dcycs;

	data_wrap = 0;
	/* find first data entry */
	while(data_wrap < 2 && (log_data_ptr->dcycs < dcycs)) {
		log_data_ptr++;
		if(log_data_ptr >= g_log_data_end_ptr) {
			log_data_ptr = g_log_data_start_ptr;
			data_wrap++;
		}
	}
	fprintf(pcfile, "start_dcycs: %9.2f\n", start_dcycs);

	for(i = 0; i < PC_LOG_LEN; i++) {
		dcycs = log_pc_ptr->dcycs;
		while((data_wrap < 2) && (log_data_ptr->dcycs <= dcycs) &&
					(log_data_ptr->dcycs >= start_dcycs)) {
			fprintf(pcfile, "DATA set %06x = %06x (%d) %9.2f\n",
				log_data_ptr->addr, log_data_ptr->val,
				log_data_ptr->size,
				log_data_ptr->dcycs - start_dcycs);
			log_data_ptr++;
			if(log_data_ptr >= g_log_data_end_ptr) {
				log_data_ptr = g_log_data_start_ptr;
				data_wrap++;
			}
		}
		dbank = (log_pc_ptr->dbank_kpc >> 24) & 0xff;
		kpc = log_pc_ptr->dbank_kpc & 0xffffff;
		instr = log_pc_ptr->instr;
		psr = (log_pc_ptr->psr_acc >> 16) & 0xffff;;
		acc = log_pc_ptr->psr_acc & 0xffff;;
		xreg = (log_pc_ptr->xreg_yreg >> 16) & 0xffff;;
		yreg = log_pc_ptr->xreg_yreg & 0xffff;;
		stack = (log_pc_ptr->stack_direct >> 16) & 0xffff;;
		direct = log_pc_ptr->stack_direct & 0xffff;;

		num = log_pc_ptr - g_log_pc_start_ptr;

		accsize = 2;
		xsize = 2;
		if(psr & 0x20) {
			accsize = 1;
		}
		if(psr & 0x10) {
			xsize = 1;
		}

		fprintf(pcfile, "%04x: A:%04x X:%04x Y:%04x P:%03x "
			"S:%04x D:%04x B:%02x %9.2f ", i,
			acc, xreg, yreg, psr, stack, direct, dbank,
			(dcycs-start_dcycs));

		do_dis(pcfile, kpc, accsize, xsize, 1, instr);
		log_pc_ptr++;
		if(log_pc_ptr >= g_log_pc_end_ptr) {
			log_pc_ptr = g_log_pc_start_ptr;
		}
	}

	fclose(pcfile);
}


word32
toolbox_debug_4byte(word32 addr)
{
	word32	part1, part2;

	/* If addr looks safe, use it */
	if(addr > 0xbffc) {
		return (word32)-1;
	}

	part1 = get_memory16_c(addr, 0);
	part1 = (part1 >> 8) + ((part1 & 0xff) << 8);
	part2 = get_memory16_c(addr+2, 0);
	part2 = (part2 >> 8) + ((part2 & 0xff) << 8);

	return (part1 << 16) + part2;
}

void
toolbox_debug_c(word32 xreg, word32 stack, double *cyc_ptr)
{
	int	pos;

	pos = g_toolbox_log_pos;

	stack += 9;
	g_toolbox_log_array[pos][0] = (word32)(g_sim65816.g_last_vbl_dcycs + *cyc_ptr);
	g_toolbox_log_array[pos][1] = stack+1;
	g_toolbox_log_array[pos][2] = xreg;
	g_toolbox_log_array[pos][3] = toolbox_debug_4byte(stack+1);
	g_toolbox_log_array[pos][4] = toolbox_debug_4byte(stack+5);
	g_toolbox_log_array[pos][5] = toolbox_debug_4byte(stack+9);
	g_toolbox_log_array[pos][6] = toolbox_debug_4byte(stack+13);
	g_toolbox_log_array[pos][7] = toolbox_debug_4byte(stack+17);

	pos++;
	if(pos >= TOOLBOX_LOG_LEN) {
		pos = 0;
	}

	g_toolbox_log_pos = pos;
}

void
show_toolbox_log()
{
	int	pos;
	int	i;

	pos = g_toolbox_log_pos;

	for(i = TOOLBOX_LOG_LEN - 1; i >= 0; i--) {
		printf("%2d:%2d: %08x %06x  %04x: %08x %08x %08x %08x %08x\n",
			i, pos,
			g_toolbox_log_array[pos][0],
			g_toolbox_log_array[pos][1],
			g_toolbox_log_array[pos][2],
			g_toolbox_log_array[pos][3],
			g_toolbox_log_array[pos][4],
			g_toolbox_log_array[pos][5],
			g_toolbox_log_array[pos][6],
			g_toolbox_log_array[pos][7]);
		pos++;
		if(pos >= TOOLBOX_LOG_LEN) {
			pos = 0;
		}
	}
}

#if 0
/* get_memory_c is not used, get_memory_asm is, but this does what the */
/*  assembly language would do */
word32
get_memory_c(word32 loc, int diff_cycles)
{
	byte	*addr;
	word32	result;
	int	index;

#ifdef CHECK_BREAKPOINTS
	check_breakpoints_c(loc);
#endif

	index = loc >> 8;
	result = page_info[index].rd;
	if(result & BANK_IO_BIT) {
		return get_memory_io(loc, diff_cycles);
	}

	addr = (byte *)((result & 0xffffff00) + (loc & 0xff));

	return *addr;
}
#endif


word32
get_memory_io(word32 loc, double *cyc_ptr)
{
	int	tmp;

	if(loc > 0xffffff) {
		halt_printf("get_memory_io:%08x out of range==halt!\n", loc);
		return 0;
	}

	tmp = loc & 0xfef000;
	if(tmp == 0xc000 || tmp == 0xe0c000) {
		return(io_read(loc & 0xfff, cyc_ptr));
	}

	/* Else it's an illegal addr...skip if memory sizing */
	if(loc >= g_sim65816.g_mem_size_total) {
		if((loc & 0xfffe) == 0) {
#if 0
			printf("get_io assuming mem sizing, not halting\n");
#endif
			return 0;
		}
	}

	/* Skip reads to f80000 and f00000, just return 0 */
	if((loc & 0xf70000) == 0xf00000) {
		return 0;
	}

	if((loc & 0xff0000) == 0xef0000) {
		/* DOC RAM */
		return (g_sound.doc_ram[loc & 0xffff]);
	}

	g_sim65816.g_code_yellow++;
	if(r_sim65816.g_ignore_bad_acc && !r_sim65816.g_user_halt_bad) {
		/* print no message, just get out.  User doesn't want */
		/*  to be bothered by buggy programs */
		return 0;
	}

	printf("get_memory_io for addr: %06x\n", loc);
	printf("stat for addr: %06x = %p\n", loc,
				GET_PAGE_INFO_RD((loc >> 8) & 0xffff));
	set_halt(r_sim65816.g_halt_on_bad_read | r_sim65816.g_user_halt_bad);

	return 0;
}

#if 0
word32
get_memory16_pieces(word32 loc, int diff_cycles)
{
	return(get_memory_c(loc, diff_cycles) +
		(get_memory_c(loc+1, diff_cycles) << 8));
}

word32
get_memory24(word32 loc, int diff_cycles)
{
	return(get_memory_c(loc, diff_cycles) +
		(get_memory_c(loc+1, diff_cycles) << 8) +
		(get_memory_c(loc+2, diff_cycles) << 16));
}
#endif

#if 0
void
set_memory(word32 loc, int val, int diff_cycles)
{
	byte *ptr;
	word32	new_addr;
	word32	tmp;
	word32	or_val;
	int	or_pos;
	int	old_slow_val;

#ifdef CHECK_BREAKPOINTS
	check_breakpoints_c(loc);
#endif

	tmp = GET_PAGE_INFO_WR((loc>>8) & 0xffff);
	if(tmp & BANK_IO) {
		set_memory_io(loc, val, diff_cycles);
		return;
	}

	if((loc & 0xfef000) == 0xe0c000) {
		printf("set_memory_special: non-io for addr %08x, %02x, %d\n",
			loc, val, diff_cycles);
		halt_printf("tmp: %08x\n", tmp);
	}

	ptr = (byte *)(tmp & (~0xff));

	new_addr = loc & 0xffff;
	old_slow_val = val;

	if(tmp & BANK_SHADOW) {
		old_slow_val = g_slow_memory_ptr[new_addr];
	} else if(tmp & BANK_SHADOW2) {
		new_addr += 0x10000;
		old_slow_val = g_slow_memory_ptr[new_addr];
	}

	if(old_slow_val != val) {
		g_slow_memory_ptr[new_addr] = val;
		or_pos = (new_addr >> SHIFT_PER_CHANGE) & 0x1f;
		or_val = DEP1(1, or_pos, 0);
		if((new_addr >> CHANGE_SHIFT) >= SLOW_MEM_CH_SIZE) {
			printf("new_addr: %08x\n", new_addr);
			x_exit(12);
		}
		g_video.slow_mem_changed[(new_addr & 0xffff) >> CHANGE_SHIFT] |= or_val;
	}

	ptr[loc & 0xff] = val;

}
#endif

void
set_memory_io(word32 loc, int val, double *cyc_ptr)
{
	word32	tmp;

	tmp = loc & 0xfef000;
	if(tmp == 0xc000 || tmp == 0xe0c000) {
		io_write(loc, val, cyc_ptr);
		return;
	}

	/* Else it's an illegal addr */
	if(loc >= g_sim65816.g_mem_size_total) {
		if((loc & 0xfffe) == 0) {
#if 0
			printf("set_io assuming mem sizing, not halting\n");
#endif
			return;
		}
	}

	/* ignore writes to ROM */
	if((loc & 0xfc0000) == 0xfc0000) {
		return;
	}

	if((loc & 0xff0000) == 0xef0000) {
		/* DOC RAM */
		g_sound.doc_ram[loc & 0xffff] = val;
		return;
	}

	if(r_sim65816.g_ignore_bad_acc && !r_sim65816.g_user_halt_bad) {
		/* print no message, just get out.  User doesn't want */
		/*  to be bothered by buggy programs */
		return;
	}

	if((loc & 0xffc000) == 0x00c000) {
		printf("set_memory %06x = %02x, warning\n", loc, val);
		return;
	}

	halt_printf("set_memory %06x = %02x, stopping\n", loc, val);

	return;
}


#if 0
void
check_breakpoints_c(word32 loc)
{
	int	index;
	int	count;
	int	i;

	index = (loc & (MAX_BP_INDEX-1));
	count = breakpoints[index].count;
	if(count) {
		for(i = 0; i < count; i++) {
			if(loc == breakpoints[index].addrs[i]) {
				halt_printf("Write hit breakpoint %d!\n", i);
			}
		}
	}
}
#endif


void
show_regs_act(Engine_reg *eptr)
{
	int	tmp_acc, tmp_x, tmp_y, tmp_psw;
	int	kpc;
	int	direct_page, dbank;
	int	stack;
	
	kpc = eptr->kpc;
	tmp_acc = eptr->acc;
	direct_page = eptr->direct;
	dbank = eptr->dbank;
	stack = eptr->stack;

	tmp_x = eptr->xreg;
	tmp_y = eptr->yreg;

	tmp_psw = eptr->psr;

	printf("  PC=%02x.%04x A=%04x X=%04x Y=%04x P=%03x",
		kpc>>16, kpc & 0xffff ,tmp_acc,tmp_x,tmp_y,tmp_psw);
	printf(" S=%04x D=%04x B=%02x,cyc:%.3f\n", stack, direct_page,
		dbank, g_sim65816.g_cur_dcycs);
}

void
show_regs()
{
	show_regs_act(&g_sim65816.engine);
}


//OG change exit() to x_exit()
extern void x_exit(int);

void my_exit(int ret)
{
	end_screen();
	printf("exiting (ret=%d)\n",ret);
	x_exit(ret);
}


void
do_reset()
{

	g_moremem.g_c068_statereg = 0x08 + 0x04 + 0x01; /* rdrom, lcbank2, intcx */
	g_moremem.g_c035_shadow_reg = 0;

	g_moremem.g_c08x_wrdefram = 1;
	g_moremem.g_c02d_int_crom = 0;
	g_moremem.g_c023_val = 0;
	g_moremem.g_c041_val = 0;

	g_sim65816.engine.psr = (g_sim65816.engine.psr | 0x134) & ~(0x08);
	g_sim65816.engine.stack = 0x100 + (g_sim65816.engine.stack & 0xff);
	g_sim65816.engine.dbank = 0;
	g_sim65816.engine.direct = 0;
	g_sim65816.engine.xreg &= 0xff;
	g_sim65816.engine.yreg &= 0xff;
	g_sim65816.g_wait_pending = 0;
	g_sim65816.g_stp_pending = 0;


	video_reset();
	adb_reset();
	iwm_reset();
	scc_reset();
	sound_reset(g_sim65816.g_cur_dcycs);
	setup_pageinfo();
	change_display_mode(g_sim65816.g_cur_dcycs);

	g_moremem.g_irq_pending = 0;

	g_sim65816.engine.kpc = get_memory16_c(0x00fffc, 0);
    
	//set_bp(g_sim65816.engine.kpc);
//	set_bp(0x666,(enum mode_breakpoint)(BRK_X|BRK_PATCH),"067B:60");

	g_stepping = 0;

	// OG Cleared remaining IRQS on RESET
	{

/*
	extern	int g_scan_int_events ;
	extern int g_c023_val;


		#define REMOVEIRQ(ADR) { extern int ADR; if (ADR) { remove_irq(); ADR=0; } }	
		#define CLEARIRQ(ADR) { extern int ADR; ADR=0;  }	
		
		REMOVEIRQ(g_adb_kbd_srq_sent);
		REMOVEIRQ(g_adb_data_int_sent);
		REMOVEIRQ(g_adb_mouse_int_sent);
		REMOVEIRQ(c046_vbl_irq_pending);
		REMOVEIRQ(c046_25sec_irq_pend);
		REMOVEIRQ(c023_scan_int_irq_pending);
		REMOVEIRQ(c023_1sec_int_irq_pending);
		REMOVEIRQ(c046_vbl_irq_pending);

		CLEARIRQ(c041_en_25sec_ints);
		CLEARIRQ(c041_en_vbl_ints);
		CLEARIRQ(c041_en_switch_ints);
		CLEARIRQ(c041_en_move_ints);
		CLEARIRQ(c041_en_mouse);
		
		g_sim65816.g_scan_int_events = 0;

		initialize_events();
*/
		if (g_moremem.g_irq_pending) 
			halt_printf("*** irq remainings...\n");

/*
		g_moremem.g_irq_pending = 0;    	    
		// reinitiliase zip speed (not sure)???
		// g_moremem.g_zipgs_reg_c05a = 0x80; 
		*/
	}	

}

#define CHECK(start, var, value, var1, var2)				\
	var2 = PTR2WORD(&(var));					\
	var1 = PTR2WORD((start));					\
	if((var2 - var1) != value) {					\
		printf("CHECK: " #var " is 0x%x, but " #value " is 0x%x\n", \
			(var2 - var1), value);				\
		x_exit(5);						\
	}

void
check_engine_asm_defines()
{
	Fplus	fplus;
	Fplus	*fplusptr;
	Pc_log	pclog;
	Pc_log	*pcptr;
	Engine_reg ereg;
	Engine_reg *eptr;
	word32	val1;
	word32	val2;

	eptr = &ereg;
	CHECK(eptr, eptr->fcycles, ENGINE_FCYCLES, val1, val2);
	CHECK(eptr, eptr->fplus_ptr, ENGINE_FPLUS_PTR, val1, val2);
	CHECK(eptr, eptr->acc, ENGINE_REG_ACC, val1, val2);
	CHECK(eptr, eptr->xreg, ENGINE_REG_XREG, val1, val2);
	CHECK(eptr, eptr->yreg, ENGINE_REG_YREG, val1, val2);
	CHECK(eptr, eptr->stack, ENGINE_REG_STACK, val1, val2);
	CHECK(eptr, eptr->dbank, ENGINE_REG_DBANK, val1, val2);
	CHECK(eptr, eptr->direct, ENGINE_REG_DIRECT, val1, val2);
	CHECK(eptr, eptr->psr, ENGINE_REG_PSR, val1, val2);
	CHECK(eptr, eptr->kpc, ENGINE_REG_KPC, val1, val2);

	pcptr = &pclog;
	CHECK(pcptr, pcptr->dbank_kpc, LOG_PC_DBANK_KPC, val1, val2);
	CHECK(pcptr, pcptr->instr, LOG_PC_INSTR, val1, val2);
	CHECK(pcptr, pcptr->psr_acc, LOG_PC_PSR_ACC, val1, val2);
	CHECK(pcptr, pcptr->xreg_yreg, LOG_PC_XREG_YREG, val1, val2);
	CHECK(pcptr, pcptr->stack_direct, LOG_PC_STACK_DIRECT, val1, val2);
	if(LOG_PC_SIZE != sizeof(pclog)) {
		printf("LOG_PC_SIZE: %d != sizeof=%d\n", LOG_PC_SIZE,
			(int)sizeof(pclog));
		x_exit(2);
	}

	fplusptr = &fplus;
	CHECK(fplusptr, fplusptr->plus_1, FPLUS_PLUS_1, val1, val2);
	CHECK(fplusptr, fplusptr->plus_2, FPLUS_PLUS_2, val1, val2);
	CHECK(fplusptr, fplusptr->plus_3, FPLUS_PLUS_3, val1, val2);
	CHECK(fplusptr, fplusptr->plus_x_minus_1, FPLUS_PLUS_X_M1, val1, val2);
}


byte *
memalloc_align(int size, int skip_amt, void **alloc_ptr)
{
	byte	*bptr;
	word32	addr;
	word32	offset;

	skip_amt = MAX(256, skip_amt);
	bptr = (byte*)calloc(size + skip_amt + 256, 1);	// OG Added cast 
	if(alloc_ptr) {
		/* Save allocation address */
		*alloc_ptr = bptr;
	}

	addr = PTR2WORD(bptr) & 0xff;

	/* must align bptr to be 256-byte aligned */
	/* this code should work even if ptrs are > 32 bits */

	offset = ((addr + skip_amt - 1) & (~0xff)) - addr;

	return (bptr + offset);
}


void
memory_ptr_init()
{
	word32	mem_size;

	/* This routine may be called several times--each time the ROM file */
	/*  changes this will be called */
	mem_size = MIN(0xdf0000, g_sim65816.g_mem_size_base + g_sim65816.g_mem_size_exp);
	g_sim65816.g_mem_size_total = mem_size;

	// OG using memory_ptr_shut() instead
	memory_ptr_shut();
	
	g_memory_ptr = memalloc_align(mem_size, 256, &g_memory_alloc_ptr);

	printf("RAM size is 0 - %06x (%.2fMB)\n", mem_size,	(double)mem_size/(1024.0*1024.0));
}

// OG Added memory_ptr_shut
void
memory_ptr_shut()
{
	if(g_memory_alloc_ptr) 
	{
		free(g_memory_alloc_ptr);
		g_memory_alloc_ptr = 0;
	}
	g_memory_ptr = 0;
}
	



	

void
kegsmain_init(int argc, char **argv)
{
	int	diff;
	printf("kegsmain");

	r_sim65816.set_state(INITIALIZING);
	
	//initSavedStateModule();

	// OG Restoring globals
	sim65816_initglobals();
	moremem_init();

//OG Disabling argument parsing
#ifndef ACTIVEGS

	int	skip_amt;
	int	tmp1;

	/* parse args */
	for(int i = 1; i < argc; i++) {
		if(!strcmp("-badrd", argv[i])) {
			printf("Halting on bad reads\n");
			r_sim65816.g_halt_on_bad_read = 2;
		} else if(!strcmp("-noignbadacc", argv[i])) {
			printf("Not ignoring bad memory accesses\n");
			r_sim65816.g_ignore_bad_acc = 0;
		} else if(!strcmp("-noignhalt", argv[i])) {
			printf("Not ignoring code red halts\n");
			r_sim65816.g_ignore_halts = 0;
		} else if(!strcmp("-test", argv[i])) {
			printf("Allowing testing\n");
			g_sim65816.g_testing_enabled = 1;
		} else if(!strcmp("-hpdev", argv[i])) {
			printf("Using /dev/audio\n");
			g_sim65816.g_use_alib = 0;
		} else if(!strcmp("-alib", argv[i])) {
			printf("Using Aserver audio server\n");
			g_sim65816.g_use_alib = 1;
		} else if(!strcmp("-24", argv[i])) {
			printf("Using 24-bit visual\n");
			g_sim65816.g_force_depth = 24;
		} else if(!strcmp("-16", argv[i])) {
			printf("Using 16-bit visual\n");
			g_sim65816.g_force_depth = 16;
		} else if(!strcmp("-15", argv[i])) {
			printf("Using 15-bit visual\n");
			g_sim65816.g_force_depth = 15;
		} else if(!strcmp("-mem", argv[i])) {
			if((i+1) >= argc) {
				printf("Missing argument\n");
				x_exit(1);
			}
			g_sim65816.g_mem_size_exp = strtol(argv[i+1], 0, 0) & 0x00ff0000;
			printf("Using %d as memory size\n", g_sim65816.g_mem_size_exp);
			i++;
		} else if(!strcmp("-skip", argv[i])) {
			if((i+1) >= argc) {
				printf("Missing argument\n");
				x_exit(1);
			}
			skip_amt = strtol(argv[i+1], 0, 0);
			printf("Using %d as skip_amt\n", skip_amt);
			g_video.g_screen_redraw_skip_amt = skip_amt;
			i++;
		} else if(!strcmp("-audio", argv[i])) {
			if((i+1) >= argc) {
				printf("Missing argument\n");
				x_exit(1);
			}
			tmp1 = strtol(argv[i+1], 0, 0);
			printf("Using %d as audio enable val\n", tmp1);
			g_sound.g_audio_enable = tmp1;
			i++;
		} 
		/*
		else if(!strcmp("-arate", argv[i])) {
			if((i+1) >= argc) {
				printf("Missing argument\n");
				x_exit(1);
			}
			tmp1 = strtol(argv[i+1], 0, 0);
			printf("Using %d as preferred audio rate\n", tmp1);
			g_preferred_rate = tmp1;
			i++;
		}*/ 
		else if(!strcmp("-v", argv[i])) {
			if((i+1) >= argc) {
				printf("Missing argument\n");
				x_exit(1);
			}
			tmp1 = strtol(argv[i+1], 0, 0);
			printf("Setting g_sim65816.Verbose = 0x%03x\n", tmp1);
			Verbose = tmp1;
			i++;
#ifndef __NeXT__
		} else if(!strcmp("-display", argv[i])) {
			if((i+1) >= argc) {
				printf("Missing argument\n");
				x_exit(1);
			}
			printf("Using %s as display\n", argv[i+1]);
			sprintf(g_sim65816.g_display_env, "DISPLAY=%s", argv[i+1]);
			putenv(&g_sim65816.g_display_env[0]);
			i++;
#endif
		}
#ifdef DRIVER_LINUX
        else if(!strcmp("-noshm", argv[i])) {
			printf("Not using X shared memory\n");
			g_use_shmem = 0;
		}
#endif
        else if(!strcmp("-joystick", argv[i])) {
			printf("Ignoring -joystick option\n");
		} else if(!strcmp("-dhr140", argv[i])) {
			printf("Using simple dhires color map\n");
			g_video.g_use_dhr140 = 1;
		} else if(!strcmp("-bw", argv[i])) {
			printf("Forcing black-and-white hires modes\n");
			g_video.g_cur_a2_stat |= ALL_STAT_COLOR_C021;
			r_sim65816.set_color_mode(COLORMODE_BW);
		//	s_video.g_use_bw_hires = 1;
		} else {
			printf("Bad option: %s\n", argv[i]);
			x_exit(3);
		}
	}
#endif

	check_engine_asm_defines();
	fixed_memory_ptrs_init();

	if(sizeof(word32) != 4) {
		printf("sizeof(word32) = %d, must be 4!\n",
							(int)sizeof(word32));
		x_exit(1);
	}

	if(!g_engine_c_mode) {
		diff = &defs_instr_end_8 - &defs_instr_start_8;
		if(diff != 1) {
			printf("defs_instr_end_8 - start is %d\n",diff);
			x_exit(1);
		}

		diff = &defs_instr_end_16 - &defs_instr_start_16;
		if(diff != 1) {
			printf("defs_instr_end_16 - start is %d\n", diff);
			x_exit(1);
		}

		diff = &op_routs_end - &op_routs_start;
		if(diff != 1) {
			printf("op_routs_end - start is %d\n", diff);
			x_exit(1);
		}
	}

	
	iwm_init();
	config_init();

	load_roms_init_memory();

	init_reg();
	clear_halt();
	
	initialize_events();

	video_init();
	
	sound_init();
    marinetti_init();
	scc_init();
	adb_init();
	joystick_init();
	if(g_sim65816.g_rom_version >= 3) {
		g_moremem.g_c036_val_speed |= 0x40;	/* set power-on bit */
	}

    
    g_num_breakpoints = 0;
#ifdef ACTIVEGS
    extern void apply_breakpoints();
    apply_breakpoints();
#endif
    
	do_reset();
	g_stepping = 0;

	

	r_sim65816.request_update_emulator_runtime_config();
	r_sim65816.set_state(RUNNING);
	if (g_driver.x_post_event)
        g_driver.x_post_event("emulator_launched");
}

void kegsmain_shut()
{


	/* If we get here, we hit a breakpoint, call debug intfc */
	do_debug_intfc();

	r_sim65816.set_state(SHUTTING_DOWN);

    marinetti_shutdown();
	sound_shutdown();
    
	adb_shut();
	iwm_shut();
	fixed_memory_ptrs_shut();
	load_roms_shut_memory();
	clear_fatal_logs();

	end_screen();
	
	r_sim65816.set_state(NO_INITIALIZED);

	//return 0;
}


int
kegsmain(int argc, char **argv)
{

	kegsmain_init(argc,argv);

	#ifdef ACTIVEGS
	{
		extern void disableConsole();
		disableConsole();
	}
	#endif

// do go
extern int g_config_control_panel;
extern void run_prog_init();
extern void run_prog_shut();
extern int run_prog_loop();

	g_config_control_panel = 0;
	clear_halt();

	run_prog_init();

	while(1)
	{
		int ret = run_prog_loop();
		if (!ret)
			break;
	}

	run_prog_shut();

	show_regs();
	g_config_control_panel = 1;

	kegsmain_shut();
	return 0;
}


void
load_roms_init_memory()
{
	config_load_roms();
	memory_ptr_init();
	clk_setup_bram_version();	/* Must be after config_load_roms */
	if(g_sim65816.g_rom_version >= 3) {
		g_moremem.g_c036_val_speed |= 0x40;	/* set power-on bit */
	} else {
		g_moremem.g_c036_val_speed &= (~0x40);	/* clear the bit */
	}
	do_reset();

	/* if user booted ROM 01, switches to ROM 03, then switches back */
	/*  to ROM 01, then the reset routines call to Tool $0102 looks */
	/*  at uninitialized $e1/15fe and if it is negative it will JMP */
	/*  through $e1/1688 which ROM 03 left pointing to fc/0199 */
	/* So set e1/15fe = 0 */
	set_memory16_c(0xe115fe, 0, 0);
}

// OG Added load_roms_shut_memory
void load_roms_shut_memory()
{
	memory_ptr_shut();
}

#ifndef ACTIVEGS

void
kegs_expand_path(char *out_ptr, const char *in_ptr, int maxlen)
{
	char	name_buf[256];
	const char	*tmp_ptr;
	int	name_len;
	int	in_char;
	int	state;

	out_ptr[0] = 0;

	name_len = 0;
	state = 0;

	/* See if in_ptr has ${} notation, replace with getenv or argv0 */
	while(maxlen > 0) {
		in_char = *in_ptr++;
		*out_ptr++ = in_char;
		maxlen--;
		if(state == 0) {
			/* No $ seen yet, look for it */
			if(in_char == '$') {
				state = 1;
			}
		} else if(state == 1) {
			/* See if next char is '{' (dummy }) */
			if(in_char == '{') {		/* add dummy } */
				state = 2;
				name_len = 0;
				out_ptr -= 2;
			} else {
				state = 0;
			}
		} else if(state == 2) {
			/* fill name_buf ... dummy '{' */
			out_ptr--;
			if(in_char == '}') {
				name_buf[name_len] = 0;

				/* got token, now look it up */
				tmp_ptr = "";
				if(!strncmp("0", name_buf, 128)) {
					/* Replace ${0} with g_argv0_path */
					tmp_ptr = &(g_argv0_path[0]);
				} else {
					tmp_ptr = getenv(name_buf);
					if(tmp_ptr == 0) {
						tmp_ptr = "";
					}
				}
				strncpy(out_ptr, tmp_ptr, maxlen);
				out_ptr += strlen(tmp_ptr);
				maxlen -= strlen(tmp_ptr);
				state = 0;
			} else {
				name_buf[name_len++] = in_char;
			}
		}
		if(in_char == 0) {
			/* make sure its null terminated */
			*out_ptr++ = 0;
			break;
		}
	}
}

void
setup_kegs_file(char *outname, int maxlen, int ok_if_missing,
		int can_create_file, const char **name_ptr)
{
	char	local_path[256];
	struct stat stat_buf;
	const char **path_ptr;
	const char **cur_name_ptr, **save_path_ptr;
	int	ret;

	outname[0] = 0;

	path_ptr = &g_kegs_default_paths[0];

	save_path_ptr = path_ptr;
	while(*path_ptr) {
		kegs_expand_path(&(local_path[0]), *path_ptr, 250);
		cur_name_ptr = name_ptr;
		while(*cur_name_ptr) {
			strcpy(outname, &(local_path[0]));
			strncat(outname, *cur_name_ptr, 255-strlen(outname));
			if(!ok_if_missing) {
				printf("Trying '%s'\n", outname);
			}
			ret = stat(outname, &stat_buf);
			if(ret == 0) {
				/* got it! */
				return;
			}
			cur_name_ptr++;
		}
		path_ptr++;
	}

	outname[0] = 0;
	if(ok_if_missing > 0) {
		return;
	}

	/* couldn't find it, print out all the attempts */
	path_ptr = save_path_ptr;
	fatal_printf("Could not find required file \"%s\" in any of these "
						"directories:\n", *name_ptr);
	while(*path_ptr) {
		fatal_printf("  %s\n", *path_ptr++);
	}

	if(can_create_file) {
		// Ask user if it's OK to create the file
		x_dialog_create_kegs_conf(*name_ptr);
		can_create_file = 0;

		// But clear out the fatal_printfs first
		clear_fatal_logs();
		setup_kegs_file(outname, maxlen, ok_if_missing,
						can_create_file, name_ptr);
		// It's one-level of recursion--it cannot loop since we
		//  clear can_create_file.
		// If it returns, then there was succes and we should get out
		return;
	} else if(ok_if_missing) {
		/* Just show an alert and return if ok_if_missing < 0 */
		x_show_alert(0, 0);
		return;
	}

	system("pwd");

	my_exit(2);
}

#endif


void
initialize_events()
{
	int	i;

	for(i = 1; i < MAX_EVENTS; i++) {
		g_sim65816.g_event_list[i-1].next = &g_sim65816.g_event_list[i];
	}
	g_sim65816.g_event_free.next = &g_sim65816.g_event_list[0];
	g_sim65816.g_event_list[MAX_EVENTS-1].next = 0;

	g_sim65816.g_event_start.next = 0;
	g_sim65816.g_event_start.dcycs = 0.0;

	add_event_entry(DCYCS_IN_16MS, EV_60HZ);
}

void
check_for_one_event_type(int type)
{
	Event	*ptr;
	int	count;
	int	depth;

	count = 0;
	depth = 0;
	ptr = g_sim65816.g_event_start.next;
	while(ptr != 0) {
		depth++;
		if(ptr->type == type) {
			count++;
			if(count != 1) {
				halt_printf("in check_for_1, type %d found at "
					"depth: %d, count: %d, at %f\n",
					type, depth, count, ptr->dcycs);
			}
		}
		ptr = ptr->next;
	}
}


void
add_event_entry(double dcycs, int type)
{
	Event	*this_event;
	Event	*ptr, *prev_ptr;
	int	tmp_type;
	int	done;

	this_event = g_sim65816.g_event_free.next;
	if(this_event == 0) {
		halt_printf("Out of queue entries!\n");
		show_all_events();
		return;
	}
	g_sim65816.g_event_free.next = this_event->next;

	this_event->type = type;

	tmp_type = type & 0xff;
	if((dcycs < 0.0) || (dcycs > (g_sim65816.g_cur_dcycs + 50*1000*1000.0)) ||
			((dcycs < g_sim65816.g_cur_dcycs) && (tmp_type != EV_SCAN_INT))) {
		halt_printf("add_event: dcycs: %f, type:%05x, cur_dcycs: %f!\n",
			dcycs, type, g_sim65816.g_cur_dcycs);
		dcycs = g_sim65816.g_cur_dcycs + 1000.0;
	}

	ptr = g_sim65816.g_event_start.next;
	if(ptr && (dcycs < ptr->dcycs)) {
		/* create event before next expected event */
		/* do this by setting HALT_EVENT */
		set_halt(HALT_EVENT);
	}

	prev_ptr = &g_sim65816.g_event_start;
	ptr = g_sim65816.g_event_start.next;

	done = 0;
	while(!done) {
		if(ptr == 0) {
			this_event->next = ptr;
			this_event->dcycs = dcycs;
			prev_ptr->next = this_event;
			return;
		} else {
			if(ptr->dcycs < dcycs) {
				/* step across this guy */
				prev_ptr = ptr;
				ptr = ptr->next;
			} else {
				/* go in front of this guy */
				this_event->dcycs = dcycs;
				this_event->next = ptr;
				prev_ptr->next = this_event;
				return;
			}
		}
	}
}


double
remove_event_entry(int type)
{
	Event	*ptr, *prev_ptr;
	Event	*next_ptr;

	ptr = g_sim65816.g_event_start.next;
	prev_ptr = &g_sim65816.g_event_start;

	while(ptr != 0) {
		if((ptr->type & 0xffff) == type) {
			/* got it, remove it */
			next_ptr = ptr->next;
			prev_ptr->next = next_ptr;

			/* Add ptr to free list */
			ptr->next = g_sim65816.g_event_free.next;
			g_sim65816.g_event_free.next = ptr;

			return ptr->dcycs;
		}
		prev_ptr = ptr;
		ptr = ptr->next;
	}

	halt_printf("remove event_entry: %08x, but not found!\n", type);
	if((type & 0xff) == EV_DOC_INT) {
		printf("DOC, g_doc_saved_ctl = %02x\n", g_sound.g_doc_saved_ctl);
	}
#ifdef HPUX
	U_STACK_TRACE();
#endif
	show_all_events();

	return 0.0;
}

void
add_event_stop(double dcycs)
{
	add_event_entry(dcycs, EV_STOP);
}

void
add_event_doc(double dcycs, int osc)
{
	if(dcycs < g_sim65816.g_cur_dcycs) {
		dcycs = g_sim65816.g_cur_dcycs;
#if 0
		halt_printf("add_event_doc: dcycs: %f, cur_dcycs: %f\n",
			dcycs, g_sim65816.g_cur_dcycs);
#endif
	}

	add_event_entry(dcycs, EV_DOC_INT + (osc << 8));
}

void
add_event_scc(double dcycs, int type)
{
	if(dcycs < g_sim65816.g_cur_dcycs) {
		dcycs = g_sim65816.g_cur_dcycs;
	}

	add_event_entry(dcycs, EV_SCC + (type << 8));
}

void
add_event_vbl()
{
	double	dcycs;

	dcycs = g_sim65816.g_last_vbl_dcycs + (DCYCS_IN_16MS * (192.0/262.0));
	add_event_entry(dcycs, EV_VBL_INT);
}

void
add_event_vid_upd(int line)
{
	double	dcycs;

	dcycs = g_sim65816.g_last_vbl_dcycs + ((DCYCS_IN_16MS * line) / 262.0);
	add_event_entry(dcycs, EV_VID_UPD + (line << 8));
}

double
remove_event_doc(int osc)
{
	return remove_event_entry(EV_DOC_INT + (osc << 8));
}

double
remove_event_scc(int type)
{
	return remove_event_entry(EV_SCC + (type << 8));
}

void
show_all_events()
{
	Event	*ptr;
	int	count;
	double	dcycs;

	count = 0;
	ptr = g_sim65816.g_event_start.next;
	while(ptr != 0) {
		dcycs = ptr->dcycs;
		printf("Event: %02x: type: %05x, dcycs: %f (%f)\n",
			count, ptr->type, dcycs, dcycs - g_sim65816.g_cur_dcycs);
		ptr = ptr->next;
		count++;
	}

}

void
show_pmhz()
{
	printf("Pmhz: %f, c036:%02x, limit: %d\n",
		g_sim65816.g_projected_pmhz, g_moremem.g_c036_val_speed, g_sim65816.get_limit_speed());

}

void
setup_zip_speeds()
{
	double	frecip;
	double	fmhz;
	int	mult;

	mult = 16 - ((g_moremem.g_zipgs_reg_c05a >> 4) & 0xf);
		// 16 = full speed, 1 = 1/16th speed
	fmhz = (8.0 * mult) / 16.0;
#if 0
	if(mult == 16) {
		/* increase full speed by 19% to make zipgs freq measuring */
		/* programs work correctly */
		fmhz = fmhz * 1.19;
	}
#endif
	frecip = 1.0 / fmhz;
	g_sim65816.g_zip_pmhz = fmhz;
	g_sim65816.g_recip_projected_pmhz_zip.plus_1 = frecip;
	g_sim65816.g_recip_projected_pmhz_zip.plus_2 = 2.0 * frecip;
	g_sim65816.g_recip_projected_pmhz_zip.plus_3 = 3.0 * frecip;
	if(frecip >= 0.5) {
		g_sim65816.g_recip_projected_pmhz_zip.plus_x_minus_1 = 1.01;
	} else {
		g_sim65816.g_recip_projected_pmhz_zip.plus_x_minus_1 = 1.01 - frecip;
	}
}


void
run_prog_init()
{

	Fplus	*fplus_ptr;
	Event	*this_event;
	Event	*db1;
	double	dcycs;
	double	now_dtime;
	double	prev_dtime;
	double	prerun_fcycles;
	double	fspeed_mult;
	double	fcycles_stop;
	word32	ret;
	word32	zip_speed_0tof, zip_speed_0tof_new;
	int	zip_en, zip_follow_cps;
	int	type;
	int	motor_on;
	int	iwm_1;
	int	iwm_25;
	int	limit_speed;
	int	apple35_sel;
	int	fast, zip_speed, faster_than_28, unl_speed;
	int	this_type;

	fflush(stdout);

	g_sim65816.g_cur_sim_dtime = 0.0;

	g_sim65816.g_recip_projected_pmhz_slow.plus_1 = 1.0;
	g_sim65816.g_recip_projected_pmhz_slow.plus_2 = 2.0;
	g_sim65816.g_recip_projected_pmhz_slow.plus_3 = 3.0;
	g_sim65816.g_recip_projected_pmhz_slow.plus_x_minus_1 = 0.9;

	g_sim65816.g_recip_projected_pmhz_fast.plus_1 = (1.0 / DEFAULT_GS_SPEED);
	g_sim65816.g_recip_projected_pmhz_fast.plus_2 = (2.0 / DEFAULT_GS_SPEED);
	g_sim65816.g_recip_projected_pmhz_fast.plus_3 = (3.0 / DEFAULT_GS_SPEED);
	g_sim65816.g_recip_projected_pmhz_fast.plus_x_minus_1 = (1.98 - (1.0/DEFAULT_GS_SPEED));

	 zip_speed_0tof = g_moremem.g_zipgs_reg_c05a & 0xf0;
	setup_zip_speeds();

	if(g_sim65816.engine.fplus_ptr == 0) {
		g_sim65816.g_recip_projected_pmhz_unl = g_sim65816.g_recip_projected_pmhz_slow;
	}

	
}

int
run_prog_loop()
{

	Fplus	*fplus_ptr;
	Event	*this_event;
	Event	*db1;
	double	dcycs;
	double	now_dtime;
	double	prev_dtime;
	double	prerun_fcycles;
	double	fspeed_mult;
	double	fcycles_stop;
	word32	ret;
	word32	zip_speed_0tof, zip_speed_0tof_new;
	int	zip_en, zip_follow_cps;
	int	type;
	int	motor_on;
	int	iwm_1;
	int	iwm_25;
	int	limit_speed;
	int	apple35_sel;
	int	fast, zip_speed, faster_than_28, unl_speed;
	int	this_type;

	int	isnextframe=0;
	 zip_speed_0tof = g_moremem.g_zipgs_reg_c05a & 0xf0;

	while(!r_sim65816.should_emulator_terminate()) 
	{

		fflush(stdout);

		r_sim65816.check_update_emulator_runtime_config();

// OG Disabling control panel
#ifndef ACTIVEGS
        extern int g_config_control_panel;
		if(g_config_control_panel) {
			config_control_panel();
		}
#endif
		if(g_moremem.g_irq_pending && !(g_sim65816.engine.psr & 0x4)) {
			irq_printf("taking an irq!\n");
			take_irq(0);
			/* Interrupt! */
		}

		motor_on = g_iwm.g_iwm_motor_on;
		limit_speed = g_sim65816.get_limit_speed();
		apple35_sel = g_moremem.g_c031_disk35 & 0x40;
		zip_en = ((g_moremem.g_zipgs_reg_c05b & 0x10) == 0);
		zip_follow_cps = ((g_moremem.g_zipgs_reg_c059 & 0x8) != 0);
		zip_speed_0tof_new = g_moremem.g_zipgs_reg_c05a & 0xf0;
		fast = (g_moremem.g_c036_val_speed & 0x80) || (zip_en && !zip_follow_cps);
		// OG Make fast parameter public
		g_sim65816.g_speed_fast = fast;
		if(zip_speed_0tof_new != zip_speed_0tof) {
			zip_speed_0tof = zip_speed_0tof_new;
			setup_zip_speeds();
		}

		iwm_1 = motor_on && !apple35_sel &&
				(g_moremem.g_c036_val_speed & 0x4) &&
				(g_iwm.g_slow_525_emul_wr || !g_iwm.g_fast_disk_emul);
		iwm_25 = (motor_on && apple35_sel) && !g_iwm.g_fast_disk_emul;
		faster_than_28 = fast && (!iwm_1 && !iwm_25) && zip_en &&
			((limit_speed == 0) || (limit_speed == 3));
		zip_speed = faster_than_28 &&
			((zip_speed_0tof != 0) || (limit_speed == 3) ||
							(g_moremem.g_zipgs_unlock >= 4) );

		// OG unlimited speed should not be affected by zip.	
		// unl_speed = faster_than_28 && !zip_speed;
		unl_speed = (limit_speed == 0) && faster_than_28;

		if(unl_speed) {
			/* use unlimited speed */
			fspeed_mult = g_sim65816.g_projected_pmhz;
			fplus_ptr = &g_sim65816.g_recip_projected_pmhz_unl;
		} else if(zip_speed) {
			fspeed_mult = g_sim65816.g_zip_pmhz;
			fplus_ptr = &g_sim65816.g_recip_projected_pmhz_zip;
		} else if(fast && !iwm_1 && !(limit_speed == 1)) {
			fspeed_mult = DEFAULT_GS_SPEED; 
 			fplus_ptr = &g_sim65816.g_recip_projected_pmhz_fast;
		} else {
			/* else run slow */
			fspeed_mult = 1.0;
			fplus_ptr = &g_sim65816.g_recip_projected_pmhz_slow;
		}

		g_sim65816.engine.fplus_ptr = fplus_ptr;

		this_type = g_sim65816.g_event_start.next->type;

		prerun_fcycles = g_sim65816.g_cur_dcycs - g_sim65816.g_last_vbl_dcycs;
		g_sim65816.engine.fcycles = prerun_fcycles;
		fcycles_stop = (g_sim65816.g_event_start.next->dcycs - g_sim65816.g_last_vbl_dcycs) +
							0.001;
		if(g_stepping) {
			fcycles_stop = prerun_fcycles;
		}
		g_sim65816.g_fcycles_stop = fcycles_stop;

#if 0
		printf("Enter g_sim65816.engine, fcycs: %f, stop: %f\n",
			prerun_fcycles, fcycles_stop);
		printf("g_sim65816.g_cur_dcycs: %f, last_vbl_dcyc: %f\n", g_cur_dcycs,
			g_sim65816.g_last_vbl_dcycs);
#endif

		g_sim65816.g_num_enter_engine++;
		prev_dtime = get_dtime();

		ret = enter_engine(&g_sim65816.engine);

		now_dtime = get_dtime();

		g_sim65816.g_cur_sim_dtime += (now_dtime - prev_dtime);

		dcycs = g_sim65816.g_last_vbl_dcycs + (double)(g_sim65816.engine.fcycles);

		g_sim65816.g_dadjcycs += (g_sim65816.engine.fcycles - prerun_fcycles) *
					fspeed_mult;

#if 0
		printf("...back, g_sim65816.engine.fcycles: %f, dcycs: %f\n",
			(double)g_sim65816.engine.fcycles, dcycs);
#endif

		g_sim65816.g_cur_dcycs = dcycs;

		if(ret != 0) {
			g_sim65816.g_engine_action++;
			handle_action(ret);
		}

		if(g_sim65816.halt_sim == HALT_EVENT) {
			g_sim65816.g_engine_halt_event++;
			/* if we needed to stop to check for interrupts, */
			/*  clear halt */
			g_sim65816.halt_sim = 0;
		}

#if 0
		if(!g_sim65816.g_testing && run_cycles < -2000000) {
			halt_printf("run_cycles: %d, cycles: %d\n", run_cycles,
								cycles);
			printf("this_type: %05x\n", this_type);
			printf("duff_cycles: %d\n", duff_cycles);
			printf("start.next->rel_time: %d, type: %05x\n",
				g_sim65816.g_event_start.next->rel_time,
				g_sim65816.g_event_start.next->type);
		}
#endif

		this_event = g_sim65816.g_event_start.next;
		while(dcycs >= this_event->dcycs) {
			/* Pop this guy off of the queue */
			g_sim65816.g_event_start.next = this_event->next;

			type = this_event->type;
			this_event->next = g_sim65816.g_event_free.next;
			g_sim65816.g_event_free.next = this_event;
			switch(type & 0xff) {
			case EV_60HZ:
				update_60hz(dcycs, now_dtime);
				isnextframe=1;
				break;
			case EV_STOP:
				printf("type: EV_STOP\n");
				printf("next: %p, dcycs: %f\n",
						g_sim65816.g_event_start.next, dcycs);
				db1 = g_sim65816.g_event_start.next;
				halt_printf("next.dcycs: %f\n", db1->dcycs);
				break;
			case EV_SCAN_INT:
				g_sim65816.g_engine_scan_int++;
				irq_printf("type: scan int\n");
				do_scan_int(dcycs, type >> 8);
				break;
			case EV_DOC_INT:
				g_sim65816.g_engine_doc_int++;
				doc_handle_event(type >> 8, dcycs);
				break;
			case EV_SCC_ENABLE:
			{
				extern void scc_delayed_enable();
				scc_delayed_enable();
			}
				break;
			case EV_VBL_INT:
				do_vbl_int();
				break;
			case EV_SCC:
				do_scc_event(type >> 8, dcycs);
				break;
			case EV_VID_UPD:
				{
					/*
				extern double last_vid_update_dcys;	// OG
				last_vid_update_dcys = this_event->dcycs;
				*/
				video_update_event_line(type >> 8);
				}
				break;
			/*
			// OG Added DelayWriteC023 Event
			case EV_DELAY_WRITE_C023:
				{
					extern int g_c023_val;
					printf("clearing c023 for line(%d)\n",type>>8);
					g_moremem.g_c023_val = g_c023_val & 0xDF & 0x7F;
				}
				break;
			*/
			default:
				printf("Unknown event: %d!\n", type);
				x_exit(3);
				break;
			}

			this_event = g_sim65816.g_event_start.next;

		}

		if(g_sim65816.g_event_start.next == 0) {
			halt_printf("ERROR...run_prog, event_start.n=0!\n");
		}

#if 0
		if(!g_sim65816.g_testing && g_sim65816.g_event_start.next->rel_time > 2000000) {
			printf("Z:start.next->rel_time: %d, duff_cycles: %d\n",
				g_sim65816.g_event_start.next->rel_time, duff_cycles);
			halt_printf("Zrun_cycles:%d, cycles:%d\n", run_cycles,
						cycles);

			show_all_events();
		}
#endif
		
		if (isnextframe)
		{
			isnextframe=0;
			if (g_driver.x_handle_state!=NULL)
				g_driver.x_handle_state();
			else
				g_savestate.handleState();

			return 1;
		}

		if(g_sim65816.halt_sim != 0 && g_sim65816.halt_sim != HALT_EVENT)
		{
			break;
		}
		if(g_stepping) 
		{
			break;
		}

	}

	return 0; // quit	
}





void
run_prog_shut()
{

		

	if(!g_sim65816.g_testing) {
		printf("leaving run_prog, g_sim65816.halt_sim:%d\n", g_sim65816.halt_sim);
	}

	x_auto_repeat_on(0);
}


void run_prog()
{
	run_prog_init();
	while(1)
	{
		int ret = run_prog_loop();
		if (!ret)
			break;
	}

	run_prog_shut(); 
}


void
add_irq(word32 irq_mask)
{
	if(g_moremem.g_irq_pending & irq_mask) {
		/* Already requested, just get out */
		return;
	}
	g_moremem.g_irq_pending |= irq_mask;
	set_halt(HALT_EVENT);
}

void
remove_irq(word32 irq_mask)
{
	g_moremem.g_irq_pending = g_moremem.g_irq_pending & (~irq_mask);
}

void
take_irq(int is_it_brk)
{
	word32	new_kpc;
	word32	va;

	irq_printf("Taking irq, at: %02x/%04x, psw: %02x, dcycs: %f\n",
			g_sim65816.engine.kpc>>16, g_sim65816.engine.kpc & 0xffff, g_sim65816.engine.psr,
			g_sim65816.g_cur_dcycs);

	g_sim65816.g_num_irq++;
	if(g_sim65816.g_wait_pending) {
		/* step over WAI instruction */
		g_sim65816.engine.kpc++;
		g_sim65816.g_wait_pending = 0;
	}

	if(g_sim65816.engine.psr & 0x100) {
		/* Emulation */
		set_memory_c(g_sim65816.engine.stack, (g_sim65816.engine.kpc >> 8) & 0xff, 0);
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xff) + 0x100;

		set_memory_c(g_sim65816.engine.stack, g_sim65816.engine.kpc & 0xff, 0);
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xff) + 0x100;

		set_memory_c(g_sim65816.engine.stack,
					(g_sim65816.engine.psr & 0xef)|(is_it_brk<<4),0);
			/* Clear B bit in psr on stack */
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xff) + 0x100;

		va = 0xfffffe;
		if(g_moremem.g_c035_shadow_reg & 0x40) {
			/* I/O shadowing off...use ram locs */
			va = 0x00fffe;
		}

	} else {
		/* native */
		set_memory_c(g_sim65816.engine.stack, (g_sim65816.engine.kpc >> 16) & 0xff, 0);
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xffff);

		set_memory_c(g_sim65816.engine.stack, (g_sim65816.engine.kpc >> 8) & 0xff, 0);
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xffff);

		set_memory_c(g_sim65816.engine.stack, g_sim65816.engine.kpc & 0xff, 0);
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xffff);

		set_memory_c(g_sim65816.engine.stack, g_sim65816.engine.psr & 0xff, 0);
		g_sim65816.engine.stack = ((g_sim65816.engine.stack -1) & 0xffff);

		if(is_it_brk) {
			/* break */
			va = 0xffffe6;
			if(g_moremem.g_c035_shadow_reg & 0x40) {
				va = 0xffe6;
			}
		} else {
			/* irq */
			va = 0xffffee;
			if(g_moremem.g_c035_shadow_reg & 0x40) {
				va = 0xffee;
			}
		}

	}

	new_kpc = get_memory_c(va, 0);
	new_kpc = new_kpc + (get_memory_c(va+1, 0) << 8);

	g_sim65816.engine.psr = ((g_sim65816.engine.psr & 0x1f3) | 0x4);

	g_sim65816.engine.kpc = new_kpc;
	HALT_ON(HALT_ON_IRQ, "Halting on IRQ\n");

}



void
show_dtime_array()
{
	double	dfirst_time;
	double	first_total_cycs;
	int	i;
	int	pos;

	dfirst_time = 0.0;
	first_total_cycs = 0.0;


	for(i = 0; i < 60; i++) {
		pos = (g_sim65816.g_vbl_index_count + i) % 60;
		printf("%2d:%2d dt:%.5f adjc:%9.1f this_vbl:%.6f "
			"exp:%.5f p:%2.2f ep:%2.2f\n",
			i, pos,
			g_sim65816.dtime_array[pos] - dfirst_time,
			g_sim65816.g_dadjcycs_array[pos] - first_total_cycs,
			g_sim65816.g_dtime_this_vbl_array[pos],
			g_sim65816.g_dtime_exp_array[pos] - dfirst_time,
			g_sim65816.g_dtime_pmhz_array[pos],
			g_sim65816.g_dtime_eff_pmhz_array[pos]);
		dfirst_time = g_sim65816.dtime_array[pos];
		first_total_cycs = g_sim65816.g_dadjcycs_array[pos];
	}
}



void
update_60hz(double dcycs, double dtime_now)
{
	register word32 end_time;
	char	status_buf[1024];
	char	sim_mhz_buf[128];
	char	total_mhz_buf[128];
	const char	*sim_mhz_ptr, *total_mhz_ptr;
	const char	*code_str1, *code_str2/*,*sp_str*/;
	double	eff_pmhz;
	double	planned_dcycs;
	double	predicted_pmhz;
	double	recip_predicted_pmhz;
	double	dtime_this_vbl_sim;
	double	dtime_diff_1sec;
	double	dratio;
	double	dtime_till_expected;
	double	dtime_diff;
	double	dtime_this_vbl;
	double	dadjcycs_this_vbl;
	double	dadj_cycles_1sec;
	double	dtmp1, dtmp2, dtmp3, dtmp4, dtmp5;
	double	dnatcycs_1sec;
	int	tmp;
	int	doit_3_persec;
	int	cur_vbl_index;
	int	prev_vbl_index;

	g_sim65816.g_vbl_count++;
    
    /*
    if (g_driver.x_post_event)
        g_driver.x_post_event("60hz");
*/
    
	/* NOTE: this event is defined to occur before line 0 */
	/* It's actually happening at the start of the border for line (-1) */
	/* All other timings should be adjusted for this */

	irq_printf("vbl_60hz: vbl: %d, dcycs: %f, last_vbl_dcycs: %f\n",
		g_sim65816.g_vbl_count, dcycs, g_sim65816.g_last_vbl_dcycs);

	planned_dcycs = DCYCS_IN_16MS;

	g_sim65816.g_last_vbl_dcycs = g_sim65816.g_last_vbl_dcycs + planned_dcycs;

	add_event_entry(g_sim65816.g_last_vbl_dcycs + planned_dcycs, EV_60HZ);
	check_for_one_event_type(EV_60HZ);

	cur_vbl_index = g_sim65816.g_vbl_index_count;

	/* figure out dtime spent running SIM, not all the overhead */
	dtime_this_vbl_sim = g_sim65816.g_cur_sim_dtime;
	g_sim65816.g_cur_sim_dtime = 0.0;
	g_sim65816.g_sim_sum = g_sim65816.g_sim_sum - g_sim65816.sim_time[cur_vbl_index] + dtime_this_vbl_sim;
	g_sim65816.sim_time[cur_vbl_index] = dtime_this_vbl_sim;

	dadj_cycles_1sec = g_sim65816.g_dadjcycs - g_sim65816.g_dadjcycs_array[cur_vbl_index];

	/* dtime_diff_1sec is dtime total spent over the last 60 ticks */
	dtime_diff_1sec = dtime_now - g_sim65816.dtime_array[cur_vbl_index];

	g_sim65816.dtime_array[cur_vbl_index] = dtime_now;
	g_sim65816.g_dadjcycs_array[cur_vbl_index] = g_sim65816.g_dadjcycs;

	prev_vbl_index = cur_vbl_index;
	cur_vbl_index = prev_vbl_index + 1;
	if(cur_vbl_index >= 60) {
		cur_vbl_index = 0;
	}
	g_sim65816.g_vbl_index_count = cur_vbl_index;

	GET_ITIMER(end_time);
	g_sim65816.g_dnatcycs_1sec += (double)(end_time - g_sim65816.g_natcycs_lastvbl);
	g_sim65816.g_natcycs_lastvbl = end_time;

	if(prev_vbl_index == 0) {
		if(g_sim65816.g_sim_sum < (1.0/250.0)) {
			sim_mhz_ptr = "???";
			g_sim65816.g_sim_mhz = 250.0;
		} else {
			g_sim65816.g_sim_mhz = (dadj_cycles_1sec / g_sim65816.g_sim_sum) /
							(1000.0*1000.0);
			sprintf(sim_mhz_buf, "%6.2f", g_sim65816.g_sim_mhz);
			sim_mhz_ptr = sim_mhz_buf;
		}
		if(dtime_diff_1sec < (1.0/250.0)) {
			total_mhz_ptr = "???";
		} else {
			sprintf(total_mhz_buf, "%6.2f",
				(dadj_cycles_1sec / dtime_diff_1sec) /
								(1000000.0));
			total_mhz_ptr = total_mhz_buf;
		}

		x_update_info(total_mhz_ptr);

		if(g_sim65816.g_video_line_update_interval == 0) {
			if(g_sim65816.g_sim_mhz > 12.0) {
				/* just set video line_ref_amt to 1 */
				g_sim65816.g_line_ref_amt = 1;
			} else if(g_sim65816.g_line_ref_amt == 1 && g_sim65816.g_sim_mhz < 4.0) {
				g_sim65816.g_line_ref_amt = 8;
			}
		} else {
			g_sim65816.g_line_ref_amt = g_sim65816.g_video_line_update_interval;
		}

		if(g_sim65816.g_dnatcycs_1sec < (1000.0*1000.0)) {
			/* make it so large that all %'s become 0 */
			g_sim65816.g_dnatcycs_1sec = 800.0*1000.0*1000.0*1000.0;
		}
		dnatcycs_1sec = g_sim65816.g_dnatcycs_1sec / 100.0; /* eff mult by 100 */

		dtmp2 = (double)(g_video.g_cycs_in_check_input) / dnatcycs_1sec;
		dtmp3 = (double)(g_video.g_cycs_in_refresh_line) / dnatcycs_1sec;
		dtmp4 = (double)(g_video.g_cycs_in_refresh_ximage) / dnatcycs_1sec;
		sprintf(status_buf, "xfer:%08x, %5.1f ref_amt:%d "
			"ch_in:%4.1f%% ref_l:%4.1f%% ref_x:%4.1f%%",
			g_refresh_bytes_xfer, g_sim65816.g_dnatcycs_1sec/(1000.0*1000.0),
			g_sim65816.g_line_ref_amt, dtmp2, dtmp3, dtmp4);
		video_update_status_line(1, status_buf);

		sprintf(status_buf, "Ints:%3d I/O:%4dK BRK:%3d COP:%2d "
			"Eng:%3d act:%3d hev:%3d esi:%3d edi:%3d",
			g_sim65816.g_num_irq, g_sim65816.g_io_amt>>10, g_sim65816.g_num_brk, g_sim65816.g_num_cop,
			g_sim65816.g_num_enter_engine, g_sim65816.g_engine_action,
			g_sim65816.g_engine_halt_event, g_sim65816.g_engine_scan_int,
			g_sim65816.g_engine_doc_int);
		video_update_status_line(2, status_buf);

		dtmp1 = (double)(g_cycs_in_sound1) / dnatcycs_1sec;
		dtmp2 = (double)(g_cycs_in_sound2) / dnatcycs_1sec;
		dtmp3 = (double)(g_cycs_in_sound3) / dnatcycs_1sec;
		dtmp4 = (double)(g_cycs_in_start_sound) / dnatcycs_1sec;
		dtmp5 = (double)(g_cycs_in_est_sound) / dnatcycs_1sec;
		sprintf(status_buf, "snd1:%4.1f%%, 2:%4.1f%%, "
			"3:%4.1f%%, st:%4.1f%% est:%4.1f%% %4.2f",
			dtmp1, dtmp2, dtmp3, dtmp4, dtmp5, g_fvoices);
		video_update_status_line(3, status_buf);

		code_str1 = "";
		code_str2 = "";
		if(g_sim65816.g_code_yellow) {
			code_str1 = "Code: Yellow";
			code_str2 = "Emulated system state suspect, save work";
		}
		if(g_sim65816.g_code_red) {
			code_str1 = "Code: RED";
			code_str2 = "Emulated system state probably corrupt";
		}
		sprintf(status_buf, "snd_plays:%4d, doc_ev:%4d, st_snd:%4d "
			"snd_parms: %4d %s",
			g_num_snd_plays, g_num_doc_events, g_num_start_sounds,
			g_num_recalc_snd_parms, code_str1);
		video_update_status_line(4, status_buf);

		draw_iwm_status(5, status_buf);

		sprintf(status_buf, "KEGS v%-6s BUILD v%s      "
			"Press F4 for Config Menu    %s",
			g_kegs_version_str, g_kegs_version_build,code_str2);
		video_update_status_line(6, status_buf);

		g_video.g_status_refresh_needed = 1;

		g_sim65816.g_num_irq = 0;
		g_sim65816.g_num_brk = 0;
		g_sim65816.g_num_cop = 0;
		g_sim65816.g_num_enter_engine = 0;
		g_sim65816.g_io_amt = 0;
		g_sim65816.g_engine_action = 0;
		g_sim65816.g_engine_halt_event = 0;
		g_sim65816.g_engine_scan_int = 0;
		g_sim65816.g_engine_doc_int = 0;

		g_video.g_cycs_in_40col = 0;
		g_video.g_cycs_in_xredraw = 0;
		g_video.g_cycs_in_check_input = 0;
		g_video.g_cycs_in_refresh_line = 0;
		g_video.g_cycs_in_refresh_ximage = 0;
		g_iwm.g_cycs_in_io_read = 0;
		g_cycs_in_sound1 = 0;
		g_cycs_in_sound2 = 0;
		g_cycs_in_sound3 = 0;
		g_cycs_in_sound4 = 0;
		g_cycs_in_start_sound = 0;
		g_cycs_in_est_sound = 0;
		g_sim65816.g_dnatcycs_1sec = 0.0;
		g_refresh_bytes_xfer = 0;

		g_num_snd_plays = 0;
		g_num_doc_events = 0;
		g_num_start_sounds = 0;
		g_num_scan_osc = 0;
		g_num_recalc_snd_parms = 0;

		g_fvoices = (float)0.0;
	}

	dtime_this_vbl = dtime_now - g_sim65816.g_dtime_last_vbl;
	if(dtime_this_vbl < 0.001) {
		dtime_this_vbl = 0.001;
	}

	g_sim65816.g_dtime_last_vbl = dtime_now;

	dadjcycs_this_vbl = g_sim65816.g_dadjcycs - g_sim65816.g_last_vbl_dadjcycs;
	g_sim65816.g_last_vbl_dadjcycs = g_sim65816.g_dadjcycs;

	g_sim65816.g_dtime_expected += (1.0/60.0);

	eff_pmhz = ((dadjcycs_this_vbl) / (dtime_this_vbl)) /
							DCYCS_1_MHZ;

	/* using eff_pmhz, predict how many cycles can be run by */
	/*   g_sim65816.g_dtime_expected */

	dtime_till_expected = g_sim65816.g_dtime_expected - dtime_now;

	dratio = 60.0 * dtime_till_expected;

	predicted_pmhz = eff_pmhz * dratio;

	if(! (predicted_pmhz < (1.4 * g_sim65816.g_projected_pmhz))) {
		predicted_pmhz = 1.4 * g_sim65816.g_projected_pmhz;
	}

	if(! (predicted_pmhz > (0.7 * g_sim65816.g_projected_pmhz))) {
		predicted_pmhz = 0.7 * g_sim65816.g_projected_pmhz;
	}

	if(!(predicted_pmhz >= 1.0)) {
		irq_printf("predicted: %f, setting to 1.0\n", predicted_pmhz);
		predicted_pmhz = 1.0;
	}

	if(!(predicted_pmhz < 250.0)) {
		irq_printf("predicted: %f, setting to 250.0\n", predicted_pmhz);
		predicted_pmhz = 250.0;
	}

	recip_predicted_pmhz = 1.0/predicted_pmhz;
	g_sim65816.g_projected_pmhz = predicted_pmhz;

	g_sim65816.g_recip_projected_pmhz_unl.plus_1 = 1.0*recip_predicted_pmhz;
	g_sim65816.g_recip_projected_pmhz_unl.plus_2 = 2.0*recip_predicted_pmhz;
	g_sim65816.g_recip_projected_pmhz_unl.plus_3 = 3.0*recip_predicted_pmhz;
	g_sim65816.g_recip_projected_pmhz_unl.plus_x_minus_1 = 1.01 - recip_predicted_pmhz;


	if(dtime_till_expected < -0.125) {
		/* If we were way off, get back on track */
		/* this happens because our sim took much longer than */
		/* expected, so we're going to skip some VBL */
		irq_printf("adj1: dtexp:%f, dt_new:%f\n",
				   g_sim65816.g_dtime_expected, dtime_now);
		
		dtime_diff = -dtime_till_expected;
		
		irq_printf("dtime_till_exp: %f, dtime_diff: %f, dcycs: %f\n",
				   dtime_till_expected, dtime_diff, dcycs);
		
		g_sim65816.g_dtime_expected += dtime_diff;
	}
	
	double w = dtime_till_expected - (1.0/60.0); // OG was 3/60 !!!
	
#ifdef USE_RASTER
	x_wait_for_last_refresh(w);
#else


	if(w>0.0) 
	{
		/* we're running fast, usleep */
		double s = get_dtime();
		if (w>0.01)
			micro_sleep(w);
		// sur windows le micro_sleep est légèrement en retard!
		while( get_dtime() < s+w);
#ifdef ENABLE_GRAPH
		/*
		double d = get_dtime() - s;
		double d2 = d - w;
		g_graph.add_graph(3,(float)d,g_sim65816.g_vbl_count);
		g_graph.add_graph(2,(float)w,g_sim65816.g_vbl_count);
		*/
#endif
	}

#endif

	CHANGE_BORDER(0,0xFF0000);
	CHANGE_BORDER(2,0xFF0000);

	g_sim65816.g_dtime_this_vbl_array[prev_vbl_index] = dtime_this_vbl;
	g_sim65816.g_dtime_exp_array[prev_vbl_index] = g_sim65816.g_dtime_expected;
	g_sim65816.g_dtime_pmhz_array[prev_vbl_index] = predicted_pmhz;
	g_sim65816.g_dtime_eff_pmhz_array[prev_vbl_index] = eff_pmhz;


	if(g_moremem.g_c041_val & C041_EN_VBL_INTS) {
		add_event_vbl();
	}

	g_sim65816.g_25sec_cntr++;
	if(g_sim65816.g_25sec_cntr >= 16) {
		g_sim65816.g_25sec_cntr = 0;
		if(g_moremem.g_c041_val & C041_EN_25SEC_INTS) {
			add_irq(IRQ_PENDING_C046_25SEC);
			g_moremem.g_c046_val |= 0x10;
			irq_printf("Setting c046 .25 sec int, g_irq_pend:%d\n",
						g_moremem.g_irq_pending);
		}
	}

	g_sim65816.g_1sec_cntr++;
	if(g_sim65816.g_1sec_cntr >= 60) {
		g_sim65816.g_1sec_cntr = 0;
		tmp = g_moremem.g_c023_val;
		tmp |= 0x40;	/* set 1sec int */
		if(tmp & 0x04) {
			tmp |= 0x80;
			add_irq(IRQ_PENDING_C023_1SEC);
			irq_printf("Setting c023 to %02x irq_pend: %d\n",
				tmp, g_moremem.g_irq_pending);
		}
		irq_printf("1s irq: %X new: %X\n",g_moremem.g_c023_val,tmp);
		g_moremem.g_c023_val = tmp;
	}

	/*
	{
		extern int 	g_delayed_c023_val;
		if (g_delayed_c023_val)
		{
				g_moremem.g_c023_val = g_c023_val & 0xDF;
				g_delayed_c023_val = 0;
				printf("clearing c023 val\n");
		}
	}
	*/
	
	if(!g_sim65816.g_scan_int_events) {
		check_scan_line_int(dcycs, 0);
	}

	doit_3_persec = 0;
	if(g_sim65816.g_config_iwm_vbl_count > 0) {
		g_sim65816.g_config_iwm_vbl_count--;
	} else {
		g_sim65816.g_config_iwm_vbl_count = 20;
		doit_3_persec = 1;
	}

	iwm_vbl_update(doit_3_persec);

// OG Disabling config update
#ifndef ACTIVEGS
	config_vbl_update(doit_3_persec);
#else
// OG Added disk update
	{
		extern void checkImages();
		checkImages();
	}
#endif

/*
	
#if 0 //def USE_RASTER
	
	static double lastraster = 0;	
	double curraster = get_dtime();
	double target = lastraster+1/60.0;
	double wait = target - curraster;

	// attend le rendu de la frame précédente (ou la synchro)
	
	x_wait_for_vbl();
	
	curraster = get_dtime();

	double delay = curraster - lastraster;
	double frame = 1/delay;

#ifdef ENABLE_GRAPH
//	g_graph.add_graph(2,frame,g_sim65816.g_vbl_count);
#endif
	lastraster = curraster;
#endif
 */
	g_video_offscreen_has_been_modified = 0;


	CHANGE_BORDER(0,0xFF00FF);
	CHANGE_BORDER(2,0xFF00FF);

	video_update();

	DRAW_BORDER();

	CHANGE_BORDER(0,0);
	CHANGE_BORDER(2,0);

#ifdef USE_RASTER
	x_wait_for_vbl();
#endif
	refresh_video(0);

	CHANGE_BORDER(0,0x888888);
	CHANGE_BORDER(2,0x888888);
	sound_update(dcycs);
	clock_update();
	scc_update(dcycs);
	paddle_update_buttons();

	CHANGE_BORDER(0,0);
	CHANGE_BORDER(2,0xFF0000);

}

void
do_vbl_int()
{
	if(g_moremem.g_c041_val & C041_EN_VBL_INTS) {
		g_moremem.g_c046_val |= 0x08;
		add_irq(IRQ_PENDING_C046_VBL);
		irq_printf("Setting c046 vbl_int_status to 1, irq_pend: %d\n",
			g_moremem.g_irq_pending);
	}
}


void
do_scan_int(double dcycs, int line)
{
	int	c023_val;
	g_sim65816.g_scan_int_events = 0;

	c023_val = g_moremem.g_c023_val;
	if(c023_val & 0x20) {
		halt_printf("c023 scan_int and another on line %03x\n", line);
	}

	/* make sure scan int is still enabled for this line */
	if((g_slow_memory_ptr[0x19d00 + line] & 0x40) &&
				(g_video.g_cur_a2_stat & ALL_STAT_SUPER_HIRES)) {
		/* valid interrupt, do it */
		c023_val |= 0xa0;	/* vgc_int and scan_int */
		if(c023_val & 0x02) {
			add_irq(IRQ_PENDING_C023_SCAN);
			irq_printf("Setting c023 to %02x, irq_pend: %d\n", c023_val, g_moremem.g_irq_pending);
		}
		
		g_moremem.g_c023_val = c023_val;
		HALT_ON(HALT_ON_SCAN_INT, "In do_scan_int\n");
	} else {
		/* scan int bit cleared on scan line control byte */
		/* look for next line, if any */
		check_scan_line_int(dcycs, line+1);
	}
}

void
check_scan_line_int(double dcycs, int cur_video_line)
{
	int	delay;
	int	start;
	int	line;
	int	i;
	/* Called during VBL interrupt phase */

	if(!(g_video.g_cur_a2_stat & ALL_STAT_SUPER_HIRES)) {
		return;
	}

	if(g_moremem.g_c023_val & 0x20) {
		/* don't check for any more */
		return;
	}

	start = cur_video_line;
	if(start < 0) {
		halt_printf("check_scan_line_int: cur_video_line: %d\n",
			cur_video_line);
		start = 0;
	}
	
	for(line = start; line < 200; line++) {
		i = line;

		if(i < 0 || i >= 200) {
			halt_printf("check_new_scan_int:i:%d, line:%d, st:%d\n",
				i, line, start);
			i = 0;
		}
		if(g_slow_memory_ptr[0x19d00+i] & 0x40) {
			//printf("Adding scan_int for line %d\n", i);
			delay = (int)( (DCYCS_IN_16MS/262.0) * ((double)line) );
			add_event_entry(g_sim65816.g_last_vbl_dcycs + delay, EV_SCAN_INT +
					(line << 8));
			//add_event_entry(g_sim65816.g_last_vbl_dcycs + delay + (DCYCS_IN_16MS/262.0)*4/5 , EV_DELAY_WRITE_C023+	(line << 8));
			g_sim65816.g_scan_int_events = 1;
			check_for_one_event_type(EV_SCAN_INT);
			break;
		}
	}
}

void
check_for_new_scan_int(double dcycs)
{
	int	cur_video_line;
	
	cur_video_line = get_lines_since_vbl(dcycs) >> 8;

	check_scan_line_int(dcycs, cur_video_line);
}

void
init_reg()
{
	g_sim65816.engine.acc = 0;
	g_sim65816.engine.xreg = 0;
	g_sim65816.engine.yreg = 0;
	g_sim65816.engine.stack = 0x1ff;
	g_sim65816.engine.direct = 0;
	g_sim65816.engine.psr = 0x134;
	g_sim65816.engine.fplus_ptr = 0;

}


void
handle_action(word32 ret)
{
	int	type;

	type = EXTRU(ret,3,4);
	switch(type) {
	case RET_BREAK:
		do_break(ret & 0xff);
		break;
	case RET_COP:
		do_cop(ret & 0xff);
		break;
#if 0
	case RET_MVN:
		do_mvn(ret & 0xffff);
		break;
#endif
	case RET_C700:
		do_c700(ret);
		break;
	case RET_C70A:
		do_c70a(ret);
		break;
	case RET_C70D:
		do_c70d(ret);
		break;
#if 0
	case RET_ADD_DEC_8:
		do_add_dec_8(ret);
		break;
	case RET_ADD_DEC_16:
		do_add_dec_16(ret);
		break;
#endif
	case RET_IRQ:
		irq_printf("Special fast IRQ response.  irq_pending: %x\n",
			g_moremem.g_irq_pending);
		break;
	case RET_WDM:
		do_wdm(ret & 0xff);
		break;
	case RET_STP:
		do_stp();
		break;
	default:
		halt_printf("Unknown special action: %08x!\n", ret);
		break;
	}

}

#if 0
void
do_add_dec_8(word32 ret)
{
	halt_printf("do_add_dec_8 called, ret: %08x\n", ret);
}

void
do_add_dec_16(word32 ret)
{
	halt_printf("do_add_dec_16 called, ret: %08x\n", ret);
}
#endif

void
do_break(word32 ret)
{
	if(!g_sim65816.g_testing) {
		printf("I think I got a break, second byte: %02x!\n", ret);
		printf("kpc: %06x\n", g_sim65816.engine.kpc);
	}

	halt_printf("do_break, kpc: %06x\n", g_sim65816.engine.kpc);
	g_sim65816.enter_debug = 1;
}

void
do_cop(word32 ret)
{
	halt_printf("COP instr %02x!\n", ret);
	fflush(stdout);
}

#if 0
void
do_mvn(word32 banks)
{
	int	src_bank, dest_bank;
	int	dest, src;
	int	num;
	int	i;
	int	val;

	halt_printf("In MVN...just quitting\n");
	return;
	printf("MVN instr with %04x, cycles: %08x\n", banks, g_sim65816.engine.cycles);
	src_bank = banks >> 8;
	dest_bank = banks & 0xff;
	printf("psr: %03x\n", g_sim65816.engine.psr);
	if((g_sim65816.engine.psr & 0x30) != 0) {
		halt_printf("MVN in non-native mode unimplemented!\n");
	}

	dest = dest_bank << 16 | g_sim65816.engine.yreg;
	src = src_bank << 16 | g_sim65816.engine.xreg;
	num = g_sim65816.engine.acc;
	printf("Moving %08x+1 bytes from %08x to %08x\n", num, src, dest);

	for(i = 0; i <= num; i++) {
		val = get_memory_c(src, 0);
		set_memory_c(dest, val, 0);
		src = (src_bank << 16) | ((src + 1) & 0xffff);
		dest = (dest_bank << 16) | ((dest + 1) & 0xffff);
	}
	g_sim65816.engine.dbank = dest_bank;
	g_sim65816.engine.acc = 0xffff;
	g_sim65816.engine.yreg = dest & 0xffff;
	g_sim65816.engine.xreg = src & 0xffff;
	g_sim65816.engine.kpc = (engine.kpc + 3);
	printf("move done. db: %02x, acc: %04x, y: %04x, x: %04x, num: %08x\n",
		g_sim65816.engine.dbank, engine.acc, engine.yreg, engine.xreg, num);
}
#endif

void
do_wdm(word32 arg)
{
	switch(arg) {
	case 0x8d: /* Bouncin Ferno does WDM 8d */
		break;
	default:
		halt_printf("do_wdm: %02x!\n", arg);
		break;
	}
}

void
do_wai()
{
	halt_printf("do_wai!\n");
}

void
do_stp()
{
	if(!g_sim65816.g_stp_pending) {
		g_sim65816.g_stp_pending = 1;
		halt_printf("Hit STP instruction at: %06x, press RESET to "
				"continue\n", g_sim65816.engine.kpc);
	}
}

void
size_fail(int val, word32 v1, word32 v2)
{
	halt_printf("Size failure, val: %08x, %08x %08x\n", val, v1, v2);
}

int
fatal_printf(const char *fmt, ...)
{
	va_list	ap;
	int	ret;

	va_start(ap, fmt);

	if(g_fatal_log < 0) {
		g_fatal_log = 0;
	}
	ret = kegs_vprintf(fmt, ap);
	va_end(ap);

	return ret;
}

int
kegs_vprintf(const char *fmt, va_list ap)
{
	char	*bufptr, *buf2ptr;
	int	len;
	int	ret;

	bufptr = (char*)malloc(4096); // OG Added Cast
	ret = vsnprintf(bufptr, 4090, fmt, ap);

	// OG Display warning
	printf("Warning:%s",bufptr);

	len = strlen(bufptr);
	if(g_fatal_log >= 0 && g_fatal_log < MAX_FATAL_LOGS) {
		buf2ptr = (char*)malloc(len+1); // OG Added Cast
		memcpy(buf2ptr, bufptr, len+1);
		g_fatal_log_strs[g_fatal_log++] = buf2ptr;
	}
	must_write(1, bufptr, len);
	if(g_debug_file_fd >= 0) {
		must_write(g_debug_file_fd, bufptr, len);
	}
	free(bufptr);

	return ret;
}

void
must_write(int fd, char *bufptr, int len)
{
	int	ret;

	while(len > 0) {
		ret = write(fd, bufptr, len);
		if(ret >= 0) {
			len -= ret;
			bufptr += ret;
		} else if(errno != EAGAIN && errno != EINTR) {
			return;		// just get out
		}
	}
}

void
clear_fatal_logs()
{
	int	i;

	for(i = 0; i < g_fatal_log; i++) {
		free(g_fatal_log_strs[i]);
		g_fatal_log_strs[i] = 0;
	}
	g_fatal_log = -1;
}

 char *
kegs_malloc_str(const char *in_str)
{
	char	*str;
	size_t	len = strlen(in_str) + 1;
	str = (char*)malloc(len); // OG Added cast
	memcpy(str, in_str, len);

	return str;
}

enum speedenum s_sim65816:: set_limit_speed(enum speedenum _speed)
{
	extern unsigned char transwarpcode[][32];

	if ( (_speed<0) || (_speed>=SPEED_ENUMSIZE) )
		_speed = SPEED_GS;

	if (_speed == SPEED_ZIP || _speed ==SPEED_UNLIMITED)
	{
		// enable transwarp hardware
		transwarpcode[0][0]='T';
	}
	else
	{
		// disable transwarp hardware
		transwarpcode[0][0]=0;
	}

	x_refresh_panel(PANEL_SPEED|PANEL_RUNTIME);
	printf("speed:%d\n",_speed);
	g_limit_speed = _speed;
	return _speed;
}

void serialize_engine(Engine_reg& engine)
{
	int fpp = -1;
	if (engine.fplus_ptr == &g_sim65816.g_recip_projected_pmhz_unl)
		fpp = 0;
	else
	if (engine.fplus_ptr == &g_sim65816.g_recip_projected_pmhz_zip)
		fpp = 1;
	else
	if (engine.fplus_ptr == &g_sim65816.g_recip_projected_pmhz_fast)
		fpp = 2;
	else
//	if (engine.fplus_ptr == &g_sim65816.g_recip_projected_pmhz_slow)
		fpp = 3;
	engine.fplus_ptr = (Fplus*)fpp;
}

void unserialize_engine(Engine_reg& engine)
{
	intptr_t fpp = (intptr_t)engine.fplus_ptr;
	if (fpp==0)
		engine.fplus_ptr = &g_sim65816.g_recip_projected_pmhz_unl;
	else
	if (fpp==1)
		engine.fplus_ptr = &g_sim65816.g_recip_projected_pmhz_zip;
	else
	if (fpp==2)
		engine.fplus_ptr =&g_sim65816.g_recip_projected_pmhz_fast;
	else
//	if (fpp ==3)
		engine.fplus_ptr = &g_sim65816.g_recip_projected_pmhz_slow;
}

