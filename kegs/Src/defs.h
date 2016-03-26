/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_RCSID_S
	.data
	.export rcsdif_defs_h,data
rcsdif_defs_h
	.stringz "@(#)$KmKId: defs.h,v 1.22 2002-11-19 03:10:38-05 kadickey Exp $"
	.code
#endif

#include "defcomm.h"

link		.reg	%r2
acc		.reg	%r3
xreg		.reg	%r4
yreg		.reg	%r5
stack		.reg	%r6
dbank		.reg	%r7
direct		.reg	%r8
neg		.reg	%r9
zero		.reg	%r10
psr		.reg	%r11
kpc		.reg	%r12
const_fd	.reg	%r13
instr		.reg	%r14
#if 0
cycles		.reg	%r13
kbank		.reg	%r14
#endif

page_info_ptr	.reg	%r15
inst_tab_ptr	.reg	%r16
fcycles_stop_ptr .reg	%r17
addr_latch	.reg	%r18

scratch1	.reg	%r19
scratch2	.reg	%r20
scratch3	.reg	%r21
scratch4	.reg	%r22
;instr		.reg	%r23		; arg3

fcycles		.reg	%fr12
fr_plus_1	.reg	%fr13
fr_plus_2	.reg	%fr14
fr_plus_3	.reg	%fr15
fr_plus_x_m1	.reg	%fr16
fcycles_stop	.reg	%fr17
fcycles_last_dcycs .reg	%fr18

ftmp1		.reg	%fr4
ftmp2		.reg	%fr5
fscr1		.reg	%fr6

#define LDC(val,reg) ldil L%val,reg ! ldo R%val(reg),reg

