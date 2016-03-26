/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_RCSID_C
const char rcsid_protos_engine_c_h[] = "@(#)$KmKId: protos_engine_c.h,v 1.11 2004-10-11 22:48:16-04 kentd Exp $";
#endif

/* END_HDR */

/* engine_c.c */
void check_breakpoints(word32 addr,enum mode_breakpoint);
word32 get_memory8_io_stub(word32 addr, byte *stat, double *fcycs_ptr, double fplus_x_m1);
word32 get_memory16_pieces_stub(word32 addr, byte *stat, double *fcycs_ptr, Fplus *fplus_ptr, int in_bank);
word32 get_memory24_pieces_stub(word32 addr, byte *stat, double *fcycs_ptr, Fplus *fplus_ptr, int in_bank);
void set_memory8_io_stub(word32 addr, word32 val, byte *stat, double *fcycs_ptr, double fplus_x_m1);
void set_memory16_pieces_stub(word32 addr, word32 val, double *fcycs_ptr, double fplus_1, double fplus_x_m1, int in_bank);
void set_memory24_pieces_stub(word32 addr, word32 val, double *fcycs_ptr, Fplus *fplus_ptr, int in_bank);
word32 get_memory_c(word32 addr, int cycs);
word32 get_memory16_c(word32 addr, int cycs);
word32 get_memory24_c(word32 addr, int cycs);
void set_memory_c(word32 addr, word32 val, int cycs);
void set_memory16_c(word32 addr, word32 val, int cycs);
void set_memory24_c(word32 addr, word32 val, int cycs);
word32 do_adc_sbc8(word32 in1, word32 in2, word32 psr, int sub);
word32 do_adc_sbc16(word32 in1, word32 in2, word32 psr, int sub);
void fixed_memory_ptrs_init(void);
word32 get_itimer(void);
void set_halt_act(int val);
void clr_halt_act(void);
word32 get_remaining_operands(word32 addr, word32 opcode, word32 psr, Fplus *fplus_ptr);
int enter_engine(Engine_reg *engine_ptr);
struct struct_breakpoint
{
	word32	addr;
	enum mode_breakpoint mode;
	const char* patch;
	const char* check;
} ;
extern int g_num_breakpoints;
extern struct_breakpoint g_breakpts[MAX_BREAK_POINTS];
