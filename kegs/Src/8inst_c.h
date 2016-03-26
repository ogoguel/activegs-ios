/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef ASM
# ifdef INCLUDE_RCSID_S
	.stringz "@(#)$KmKId: instable.h,v 1.104 2004-10-05 20:12:08-04 kentd Exp $"
# endif
#endif

#ifdef _DEBUG
#define ENTER_DEBUGGER() set_halt(HALT_WANTTOBRK)
#else 
#define	ENTER_DEBUGGER()
#endif

#define IF_OPCODE_SUPPORTED(_INC) \
	if (g_sim65816.g_6502_emulation && kpc < 0x00C000) \
	{ \
	printf("unsupported instruction '%0x' @%0x : replacing by %d NOP\n",opcode,kpc,_INC); \
	ENTER_DEBUGGER(); \
	kpc = (kpc & 0xff0000) + ((kpc + _INC) & 0xffff); \
	}	\
	else 

case 0x00:			/*  brk */
#ifdef ASM
	ldb	1(scratch1),ret0
	ldil	l%g_sim65816.g_testing,arg3
	ldil	l%g_sim65816.g_num_brk,arg1
	ldw	r%g_sim65816.g_testing(arg3),arg3
	INC_KPC_2;
	ldw	r%g_sim65816.g_num_brk(arg1),arg2
	comib,<> 0,arg3,brk_testing_8
	extru	kpc,31,16,arg0
	addi	1,arg2,arg2
	bb,>=	psr,23,brk_native_8
	stw	arg2,r%g_sim65816.g_num_brk(arg1)

	bl	push_16,link
	nop

	bl	push_8,link
	extru	psr,31,8,arg0		;B bit already on in PSR

	ldil	l%0xfffe,arg0
	bl	get_mem_long_16,link
	ldo	r%0xfffe(arg0),arg0

	zdep	ret0,31,16,kpc		;set kbank to 0

#if 0
	bl	set_halt_act,link
	ldi	3,arg0
#endif


	ldi	0,dbank			;clear dbank in emul mode
	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off


brk_native_8
	stw	arg0,STACK_SAVE_COP_ARG0(sp)
	bl	push_8,link
	extru	kpc,15,8,arg0

	bl	push_16,link
	ldw	STACK_SAVE_COP_ARG0(sp),arg0

	bl	push_8,link
	extru	psr,31,8,arg0

	ldil	l%0xffe6,arg0
	bl	get_mem_long_16,link
	ldo	r%0xffe6(arg0),arg0

	zdep	ret0,31,16,kpc		;zero kbank in kpc

#if 0
#endif
	bl	set_halt_act,link
	ldi	3,arg0

	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off

brk_testing_8
	DEC_KPC2;
	CYCLES_PLUS_2
	b	dispatch_done
	depi	RET_BREAK,3,4,ret0

#else
	GET_1BYTE_ARG;
	if(g_sim65816.g_testing) {
		CYCLES_PLUS_2;
		FINISH(RET_BREAK, arg);
	}
	g_sim65816.g_num_brk++;
	INC_KPC_2;
	if(psr & 0x100) {
		PUSH16(kpc & 0xffff);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xfffe, kpc, 0);
		dbank = 0;
	} else {
		PUSH8(kpc >> 16);
		PUSH16(kpc);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xffe6, kpc, 0);
		halt_printf("Halting for native break!\n");
	}
	kpc = kpc & 0xffff;
	psr |= 0x4;
	psr &= ~(0x8);
#endif

	break;
case 0x01:			/*  ORA (Dloc,X) */
/*  called with arg = val to ORA in */
	GET_DLOC_X_IND_RD();
	ORA_INST();

	break;
case 0x02:			/*  COP */
#ifdef ASM
	ldil	l%g_sim65816.g_num_cop,arg1
	INC_KPC_2;
	ldw	r%g_sim65816.g_num_cop(arg1),arg2
	extru	kpc,31,16,arg0
	addi	1,arg2,arg2
	bb,>=	psr,23,cop_native_8
	stw	arg2,r%g_sim65816.g_num_cop(arg1)

	bl	push_16,link
	nop

	bl	push_8,link
	extru	psr,31,8,arg0

	ldil	l%0xfff4,arg0
	bl	get_mem_long_16,link
	ldo	r%0xfff4(arg0),arg0

	ldi	0,dbank			;clear dbank in emul mode
	zdep	ret0,31,16,kpc		;clear kbank

	bl	set_halt_act,link
	ldi	3,arg0

	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off

cop_native_8
	stw	arg0,STACK_SAVE_COP_ARG0(sp)
	bl	push_8,link
	extru	kpc,15,8,arg0

	bl	push_16,link
	ldw	STACK_SAVE_COP_ARG0(sp),arg0

	bl	push_8,link
	extru	psr,31,8,arg0

	ldil	l%0xffe4,arg0
	bl	get_mem_long_16,link
	ldo	r%0xffe4(arg0),arg0

	zdep	ret0,31,16,kpc		;clear kbank
	b	dispatch
	depi	1,29,2,psr		;ints masked, decimal off


#else
	IF_OPCODE_SUPPORTED(1)
	{
	g_sim65816.g_num_cop++;
	INC_KPC_2;
	if(psr & 0x100) {
		halt_printf("Halting for emul COP at %04x\n", kpc);
		PUSH16(kpc & 0xffff);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xfff4, kpc, 0);
		dbank = 0;
	} else {
		PUSH8(kpc >> 16);
		PUSH16(kpc & 0xffff);
		PUSH8(psr & 0xff);
		GET_MEMORY16(0xffe4, kpc, 0);
	}
	kpc = kpc & 0xffff;
	psr |= 4;
	psr &= ~(0x8);
	}
#endif

	break;
case 0x03:			/*  ORA Disp8,S */
	IF_OPCODE_SUPPORTED(1)
	{
		GET_DISP8_S_RD();
		ORA_INST();
	}
	break;
case 0x04:			/*  TSB Dloc */
	GET_DLOC_RD();
	TSB_INST(1);

	break;
case 0x05:			/*  ORA Dloc */
	GET_DLOC_RD();
	ORA_INST();

	break;
case 0x06:			/*  ASL Dloc */
	GET_DLOC_RD();
	ASL_INST(1);

	break;
case 0x07:			/*  ORA [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	ORA_INST();
	}
	break;
case 0x08:			/*  PHP */
#ifdef ASM
	dep	neg,24,1,psr
	ldil	l%dispatch,link
	INC_KPC_1
	depi	0,30,1,psr
	comiclr,<> 0,zero,0
	depi	1,30,1,psr
	ldo	r%dispatch(link),link
	b	push_8
	extru	psr,31,8,arg0
#else
	INC_KPC_1;
	psr = (psr & ~0x82) | ((neg & 1) << 7) | ((!zero) << 1);
	PUSH8(psr);
#endif

	break;
case 0x09:			/*  ORA #imm */
	GET_IMM_MEM();
	ORA_INST();

	break;
case 0x0a:			/*  ASL a */
#ifdef ASM
# ifdef ACC8
	ldi	0xff,scratch1
	sh1add	acc,0,scratch3
	INC_KPC_1
	extru	scratch3,24,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,23,1,scratch2
	dep	zero,31,8,acc
	b	dispatch
	dep	scratch2,31,1,psr		/* set carry */
# else
	zdepi	-1,31,16,scratch1
	sh1add	acc,0,scratch3
	INC_KPC_1
	extru	scratch3,16,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,15,1,scratch2
	dep	scratch2,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
	tmp1 = acc + acc;
# ifdef ACC8
	SET_CARRY8(tmp1);
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(acc & 0xff);
# else
	SET_CARRY16(tmp1);
	acc = tmp1 & 0xffff;
	SET_NEG_ZERO16(acc);
# endif
#endif

	break;
case 0x0b:			/*  PHD */
#ifdef ASM
	ldil	l%dispatch,link
	extru	direct,31,16,arg0
	INC_KPC_1
	b	push_16_unsafe
	ldo	r%dispatch(link),link
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	PUSH16_UNSAFE(direct);
	}
#endif

	break;
case 0x0c:			/*  TSB abs */
	GET_ABS_RD();
	TSB_INST(0);

	break;
case 0x0d:			/*  ORA abs */
	GET_ABS_RD();
	ORA_INST();

	break;
case 0x0e:			/*  ASL abs */
	GET_ABS_RD();
	ASL_INST(0);

	break;
case 0x0f:			/*  ORA long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	ORA_INST();
	}

	break;
case 0x10:			/*  BPL disp8 */
#ifdef ASM
	COND_BR1
	comib,<> 0,neg,inst10_2_8
	COND_BR2

inst10_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(neg == 0);
#endif

	break;
case 0x11:			/*  ORA (Dloc),y */
	GET_DLOC_IND_Y_RD();
	ORA_INST();

	break;
case 0x12:			/*  ORA (Dloc) */
	GET_DLOC_IND_RD();
	ORA_INST();

	break;
case 0x13:			/*  ORA (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	ORA_INST();
	}
	break;
case 0x14:			/*  TRB Dloc */
	GET_DLOC_RD();
	TRB_INST(1);

	break;
case 0x15:			/*  ORA Dloc,x */
	GET_DLOC_X_RD();
	ORA_INST();

	break;
case 0x16:			/*  ASL Dloc,X */
	GET_DLOC_X_RD();
	ASL_INST(1);

	break;
case 0x17:			/*  ORA [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	ORA_INST();
	}
	break;
case 0x18:			/*  CLC */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	0,31,1,psr		/* clear carry */
#else
	psr = psr & (~1);
	INC_KPC_1;
#endif

	break;
case 0x19:			/*  ORA abs,y */
	GET_ABS_Y_RD();
	ORA_INST();


	break;
case 0x1a:			/*  INC a */
#ifdef ASM
# ifdef ACC8
	ldi	0xff,scratch2
	addi	1,acc,scratch1
	extru	scratch1,24,1,neg
	INC_KPC_1
	extru	scratch1,31,8,zero
	b	dispatch
	dep	zero,31,8,acc
# else
	zdepi	-1,31,16,scratch2
	addi	1,acc,scratch1
	extru	scratch1,16,1,neg
	INC_KPC_1
	extru	scratch1,31,16,zero
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	acc = (acc & 0xff00) | ((acc + 1) & 0xff);
	SET_NEG_ZERO8(acc & 0xff);
# else
	acc = (acc + 1) & 0xffff;
	SET_NEG_ZERO16(acc);
# endif
#endif

	break;
case 0x1b:			/*  TCS */
#ifdef ASM
	copy	acc,stack
	extru,=	psr,23,1,0		/* in emulation mode, stack page 1 */
	depi	1,23,24,stack
	INC_KPC_1
	b	dispatch
	nop
#else
	IF_OPCODE_SUPPORTED(1)
	{
		stack = acc;
		INC_KPC_1;
		if(psr & 0x100) 
		{
			stack = (stack & 0xff) + 0x100;
		}
	}
#endif

	break;
case 0x1c:			/*  TRB Abs */
	GET_ABS_RD();
	TRB_INST(0);

	break;
case 0x1d:			/*  ORA Abs,X */
	GET_ABS_X_RD();
	ORA_INST();

	break;
case 0x1e:			/*  ASL Abs,X */
	GET_ABS_X_RD_WR();
	ASL_INST(0);

	break;
case 0x1f:			/*  ORA Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_RD();
	ORA_INST();
	}

	break;
case 0x20:			/*  JSR abs */
#ifdef ASM
	addi	2,kpc,arg0
	ldb	1(scratch1),scratch2
	CYCLES_PLUS_2
	ldb	2(scratch1),scratch1
	ldil	l%dispatch,link
	extru	arg0,31,16,arg0
	ldo	r%dispatch(link),link
	dep	scratch2,31,8,kpc
	b	push_16
	dep	scratch1,23,8,kpc
#else
	GET_2BYTE_ARG;
	INC_KPC_2;
	PUSH16(kpc);
	kpc = (kpc & 0xff0000) + arg;
	CYCLES_PLUS_2;
#endif

	break;
case 0x21:			/*  AND (Dloc,X) */
/*  called with arg = val to AND in */
	GET_DLOC_X_IND_RD();
	AND_INST();

	break;
case 0x22:			/*  JSL Long */
#ifdef ASM
	INC_KPC_3
	ldb	3(scratch1),scratch2
	copy	kpc,arg0
	ldb	1(scratch1),kpc
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_3
	dep	scratch2,15,8,kpc
	stw	scratch2,STACK_SAVE_INSTR_TMP1(sp)
	bl	push_24_unsafe,link
	dep	scratch1,23,8,kpc

	b	dispatch
	nop
#else
	IF_OPCODE_SUPPORTED(2)
	{
	GET_3BYTE_ARG;
	tmp1 = arg;
	CYCLES_PLUS_3;
	INC_KPC_3;
	PUSH24_UNSAFE(kpc);
	kpc = tmp1 & 0xffffff;
	}
#endif

	break;
case 0x23:			/*  AND Disp8,S */
/*  called with arg = val to AND in */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_RD();
	AND_INST();
	}
	break;
case 0x24:			/*  BIT Dloc */
	GET_DLOC_RD();
	BIT_INST();

	break;
case 0x25:			/*  AND Dloc */
/*  called with arg = val to AND in */
	GET_DLOC_RD();
	AND_INST();

	break;
case 0x26:			/*  ROL Dloc */
	GET_DLOC_RD();
/*  save1 is now apple addr */
/*  ret0 is data */
	ROL_INST(1);

	break;
case 0x27:			/*  AND [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	AND_INST();
	}
	break;
case 0x28:			/*  PLP */
#ifdef ASM
	bl	pull_8,link
	ldi	0,zero

	extru	psr,27,2,scratch2		/* save old x & m */
	dep	ret0,31,8,psr
	CYCLES_PLUS_1
	INC_KPC_1
	extru,<> ret0,30,1,0
	ldi	1,zero
	copy	scratch2,arg0
	b	update_system_state
	extru	ret0,24,1,neg
#else
	PULL8(tmp1);
	tmp2 = psr;
	CYCLES_PLUS_1;
	INC_KPC_1;
	psr = (psr & ~0xff) | (tmp1 & 0xff);
	zero = !(psr & 2);
	neg = (psr >> 7) & 1;
	UPDATE_PSR(psr, tmp2);
#endif
	

	break;
case 0x29:			/*  AND #imm */
	GET_IMM_MEM();
	AND_INST();

	break;
case 0x2a:			/*  ROL a */
#ifdef ASM
# ifdef ACC8
	extru	psr,31,1,scratch2
	ldi	0xff,scratch1
	sh1add	acc,scratch2,scratch3
	INC_KPC_1
	extru	scratch3,24,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,23,1,scratch2
	dep	zero,31,8,acc
	b	dispatch
	dep	scratch2,31,1,psr		/* set carry */
# else
	extru	psr,31,1,scratch2
	INC_KPC_1
	sh1add	acc,scratch2,scratch3
	zdepi	-1,31,16,scratch1
	extru	scratch3,16,1,neg
	and	scratch3,scratch1,zero
	extru	scratch3,15,1,scratch2
	dep	scratch2,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	tmp1 = ((acc & 0xff) << 1) + (psr & 1);
	SET_CARRY8(tmp1);
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(tmp1 & 0xff);
# else
	tmp1 = (acc << 1) + (psr & 1);
	SET_CARRY16(tmp1);
	acc = (tmp1 & 0xffff);
	SET_NEG_ZERO16(acc);
# endif
#endif

	break;
case 0x2b:			/*  PLD */
#ifdef ASM
	INC_KPC_1
	bl	pull_16_unsafe,link
	CYCLES_PLUS_1
	extru	ret0,31,16,direct
	extru	ret0,16,1,neg
	b	dispatch
	copy	direct,zero
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	PULL16_UNSAFE(direct);
	CYCLES_PLUS_1;
	SET_NEG_ZERO16(direct);
	}
#endif

	break;
case 0x2c:			/*  BIT abs */
	GET_ABS_RD();
	BIT_INST();

	break;
case 0x2d:			/*  AND abs */
	GET_ABS_RD();
	AND_INST();

	break;
case 0x2e:			/*  ROL abs */
	GET_ABS_RD();
	ROL_INST(0);

	break;
case 0x2f:			/*  AND long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	AND_INST();
	}

	break;
case 0x30:			/*  BMI disp8 */
#ifdef ASM
	COND_BR1
	comib,= 0,neg,inst30_2_8
	COND_BR2

inst30_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(neg);
#endif

	break;
case 0x31:			/*  AND (Dloc),y */
	GET_DLOC_IND_Y_RD();
	AND_INST();

	break;
case 0x32:			/*  AND (Dloc) */
	GET_DLOC_IND_RD();
	AND_INST();

	break;
case 0x33:			/*  AND (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	AND_INST();
	}
	break;
case 0x34:			/*  BIT Dloc,x */
	GET_DLOC_X_RD();
	BIT_INST();

	break;
case 0x35:			/*  AND Dloc,x */
	GET_DLOC_X_RD();
	AND_INST();

	break;
case 0x36:			/*  ROL Dloc,X */
	GET_DLOC_X_RD();
	ROL_INST(1);

	break;
case 0x37:			/*  AND [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	AND_INST();
	}
	break;
case 0x38:			/*  SEC */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	1,31,1,psr		/* set carry */
#else
	psr = psr | 1;
	INC_KPC_1;
#endif

	break;
case 0x39:			/*  AND abs,y */
	GET_ABS_Y_RD();
	AND_INST();

	break;
case 0x3a:			/*  DEC a */
#ifdef ASM
# ifdef ACC8
	addi	-1,acc,scratch1
	extru	scratch1,24,1,neg
	INC_KPC_1
	extru	scratch1,31,8,zero
	b	dispatch
	dep	zero,31,8,acc
# else
	addi	-1,acc,scratch1
	extru	scratch1,16,1,neg
	INC_KPC_1
	extru	scratch1,31,16,zero
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	acc = (acc & 0xff00) | ((acc - 1) & 0xff);
	SET_NEG_ZERO8(acc & 0xff);
# else
	acc = (acc - 1) & 0xffff;
	SET_NEG_ZERO16(acc);
# endif
#endif

	break;
case 0x3b:			/*  TSC */
/*  set N,Z according to 16 bit acc */
#ifdef ASM
	copy	stack,acc
	extru	stack,16,1,neg
	INC_KPC_1
	b	dispatch
	extru	acc,31,16,zero
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	acc = stack;
	SET_NEG_ZERO16(acc);
	}
#endif

	break;
case 0x3c:			/*  BIT Abs,x */
	GET_ABS_X_RD();
	BIT_INST();

	break;
case 0x3d:			/*  AND Abs,X */
	GET_ABS_X_RD();
	AND_INST();

	break;
case 0x3e:			/*  ROL Abs,X */
	GET_ABS_X_RD_WR();
	ROL_INST(0);

	break;
case 0x3f:			/*  AND Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_RD();
	AND_INST();
	}

	break;
case 0x40:			/*  RTI */
#ifdef ASM
	bb,>=	psr,23,rti_native_8
	CYCLES_PLUS_1
/*  emulation */
	bl	pull_24,link
	ldi	0,zero

	extru	psr,27,2,scratch2
	extru	ret0,23,16,scratch3
	copy	scratch2,arg0
	extru,<> ret0,30,1,0
	ldi	1,zero
	dep	ret0,31,8,psr

	extru	ret0,24,1,neg
	b	update_system_state
	dep	scratch3,31,16,kpc

rti_native_8
	bl	pull_8,link
	ldi	0,zero

	copy	ret0,scratch1
	extru	ret0,24,1,neg
	dep	ret0,31,8,scratch1
	bl	pull_24,link
	stw	scratch1,STACK_SAVE_INSTR_TMP1(sp)

	extru	psr,27,2,scratch2
	ldw	STACK_SAVE_INSTR_TMP1(sp),psr
	extru	ret0,31,24,kpc
	extru,<> psr,30,1,0
	ldi	1,zero

	b	update_system_state_and_change_kbank
	copy	scratch2,arg0
#else
	CYCLES_PLUS_1
	if(psr & 0x100) {
		PULL24(tmp1);
		kpc = (kpc & 0xff0000) + ((tmp1 >> 8) & 0xffff);
		tmp2 = psr;
		psr = (psr & ~0xff) + (tmp1 & 0xff);
		neg = (psr >> 7) & 1;
		zero = !(psr & 2);
		UPDATE_PSR(psr, tmp2);
	} else {
		PULL8(tmp1);
		tmp2 = psr;
		psr = (tmp1 & 0xff);
		neg = (psr >> 7) & 1;
		zero = !(psr & 2);
		PULL24(kpc);
		UPDATE_PSR(psr, tmp2);
	}
#endif


	break;
case 0x41:			/*  EOR (Dloc,X) */
/*  called with arg = val to EOR in */
	GET_DLOC_X_IND_RD();
	EOR_INST();

	break;
case 0x42:			/*  WDM */
#ifdef ASM
	ldb	1(scratch1),ret0
	CYCLES_PLUS_5
	CYCLES_PLUS_2
	INC_KPC_2
	b	dispatch_done
	depi	RET_WDM,3,4,ret0
#else
	IF_OPCODE_SUPPORTED(2) // 0x42
	{
	GET_1BYTE_ARG;
	INC_KPC_2;
	CYCLES_PLUS_5;
	CYCLES_PLUS_2;
	FINISH(RET_WDM, arg & 0xff);
	}
#endif

	break;
case 0x43:			/*  EOR Disp8,S */
/*  called with arg = val to EOR in */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_RD();
	EOR_INST();
	}
	break;
case 0x44:			/*  MVP */
#ifdef ASM
	ldb	2(scratch1),scratch2		/* src bank */
	bb,<	psr,23,inst44_notnat_8
	ldb	1(scratch1),dbank		/* dest bank */
	bb,<	psr,27,inst44_notnat_8
	stw	scratch2,STACK_SRC_BANK(sp)

inst44_loop_8
	CYCLES_PLUS_1
	ldw	STACK_SRC_BANK(sp),scratch2
	copy	xreg,arg0

	bl	get_mem_long_8,link
	dep	scratch2,15,8,arg0
/*  got byte */
	copy	ret0,arg1
	copy	yreg,arg0
	bl	set_mem_long_8,link
	dep	dbank,15,8,arg0
/*  wrote byte, dec acc */
	CYCLES_PLUS_2
	fldds	0(fcycles_stop_ptr),fcycles_stop
	addi	-1,xreg,xreg
	zdepi	-1,31,16,scratch2
	addi	-1,yreg,yreg
	addi	-1,acc,acc
	fcmp,<,dbl fcycles,fcycles_stop
	and	xreg,scratch2,xreg
	extrs	acc,31,16,scratch1
	and	yreg,scratch2,yreg

	comib,= -1,scratch1,inst44_done_8
	and	acc,scratch2,acc

	ftest
	b	inst44_out_of_time_8

	CYCLES_PLUS_2
	b	inst44_loop_8
	nop

/*  get here if done */
inst44_done_8
	INC_KPC_3
	b	dispatch
	nop

inst44_notnat_8
	copy	dbank,ret0
	dep	scratch2,23,8,ret0
	CYCLES_PLUS_3
	depi	RET_MVP,3,4,ret0
	b	dispatch_done
	CYCLES_PLUS_2

inst44_out_of_time_8
/*  cycle have gone positive, just get out, do not update kpc */
	b,n	dispatch
#else
	IF_OPCODE_SUPPORTED(2) // 0x44
	{
	GET_2BYTE_ARG;
	/* arg & 0xff = dest bank, arg & 0xff00 = src bank */
	if(psr & 0x110) {
		// OG MVP should work even with no native mode - keep the warning though
		printf("MVP but not native m or x!\n");
		/*
		halt_printf("MVP but not native m or x!\n");
		break;
		*/
	}
	CYCLES_MINUS_2
	dbank = arg & 0xff;
	tmp1 = (arg >> 8) & 0xff;
	while(1) {
		CYCLES_PLUS_3;
		GET_MEMORY8((tmp1 << 16) + xreg, arg);
		SET_MEMORY8((dbank << 16) + yreg, arg);
		CYCLES_PLUS_2;
		xreg = (xreg - 1) & 0xffff;
		yreg = (yreg - 1) & 0xffff;
		acc = (acc - 1) & 0xffff;
		if(acc == 0xffff) {
			INC_KPC_3;
			break;
		}
		if(fcycles >= g_sim65816.g_fcycles_stop) {
			break;
		}
	}
	}
#endif


	break;
case 0x45:			/*  EOR Dloc */
/*  called with arg = val to EOR in */
	GET_DLOC_RD();
	EOR_INST();

	break;
case 0x46:			/*  LSR Dloc */
	GET_DLOC_RD();
/*  save1 is now apple addr */
/*  ret0 is data */
	LSR_INST(1);

	break;
case 0x47:			/*  EOR [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	EOR_INST();
	}
	break;
case 0x48:			/*  PHA */
#ifdef ASM
# ifdef ACC8
	INC_KPC_1
	ldil	l%dispatch,link
	extru	acc,31,8,arg0
	b	push_8
	ldo	r%dispatch(link),link
# else
	INC_KPC_1
	ldil	l%dispatch,link
	extru	acc,31,16,arg0
	b	push_16
	ldo	r%dispatch(link),link
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	PUSH8(acc);
# else
	PUSH16(acc);
# endif
#endif

	break;
case 0x49:			/*  EOR #imm */
	GET_IMM_MEM();
	EOR_INST();

	break;
case 0x4a:			/*  LSR a */
#ifdef ASM
# ifdef ACC8
	extru	acc,31,1,scratch2
	INC_KPC_1
	extru	acc,30,7,zero
	ldi	0,neg
	dep	scratch2,31,1,psr		/* set carry */
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	acc,31,1,scratch2
	INC_KPC_1
	extru	acc,30,15,zero
	ldi	0,neg
	dep	scratch2,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	tmp1 = ((acc & 0xff) >> 1);
	SET_CARRY8(acc << 8);
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(tmp1 & 0xff);
# else
	tmp1 = (acc >> 1);
	SET_CARRY8((acc << 8));
	acc = (tmp1 & 0xffff);
	SET_NEG_ZERO16(acc);
# endif
#endif

	break;
case 0x4b:			/*  PHK */
#ifdef ASM
	ldil	l%dispatch,link
	extru	kpc,15,8,arg0
	INC_KPC_1
	b	push_8
	ldo	r%dispatch(link),link
#else
	IF_OPCODE_SUPPORTED(1)
	{
	PUSH8(kpc >> 16);
	INC_KPC_1;
	}
#endif

	break;
case 0x4c:			/*  JMP abs */
#ifdef ASM
	ldb	1(scratch1),scratch2
	CYCLES_PLUS_1
	ldb	2(scratch1),scratch1
	dep	scratch2,31,8,kpc
	b	dispatch
	dep	scratch1,23,8,kpc
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	kpc = (kpc & 0xff0000) + arg;
#endif
	

	break;
case 0x4d:			/*  EOR abs */
	GET_ABS_RD();
	EOR_INST();

	break;
case 0x4e:			/*  LSR abs */
	GET_ABS_RD();
	LSR_INST(0);

	break;
case 0x4f:			/*  EOR long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	EOR_INST();
	}

	break;
case 0x50:			/*  BVC disp8 */
#ifdef ASM
	COND_BR1
	bb,<	psr,25,inst50_2_8
	COND_BR2

inst50_2_8
	COND_BR_UNTAKEN

#else
	BRANCH_DISP8((psr & 0x40) == 0);
#endif

	break;
case 0x51:			/*  EOR (Dloc),y */
	GET_DLOC_IND_Y_RD();
	EOR_INST();

	break;
case 0x52:			/*  EOR (Dloc) */
	GET_DLOC_IND_RD();
	EOR_INST();

	break;
case 0x53:			/*  EOR (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	EOR_INST();
	}
	break;
case 0x54:			/*  MVN  */
#ifdef ASM
	ldb	2(scratch1),scratch2		/* src bank */
	bb,<	psr,23,inst54_notnat_8
	ldb	1(scratch1),dbank		/* dest bank */
	bb,<	psr,27,inst54_notnat_8
	stw	scratch2,STACK_SRC_BANK(sp)

/*  even in 8bit acc mode, use 16-bit accumulator! */

inst54_loop_8
	CYCLES_PLUS_1
	ldw	STACK_SRC_BANK(sp),scratch2
	copy	xreg,arg0

	bl	get_mem_long_8,link
	dep	scratch2,15,8,arg0
/*  got byte */
	copy	ret0,arg1
	copy	yreg,arg0
	bl	set_mem_long_8,link
	dep	dbank,15,8,arg0
/*  wrote byte, dec acc */
	CYCLES_PLUS_2
	fldds	0(fcycles_stop_ptr),fcycles_stop
	addi	1,xreg,xreg
	zdepi	-1,31,16,scratch2
	addi	1,yreg,yreg
	addi	-1,acc,acc
	fcmp,<,dbl fcycles,fcycles_stop
	and	xreg,scratch2,xreg
	extrs	acc,31,16,scratch1
	and	yreg,scratch2,yreg

	comib,=	-1,scratch1,inst54_done_8
	and	acc,scratch2,acc
	ftest
	b,n	inst54_out_of_time_8

	CYCLES_PLUS_2
	b	inst54_loop_8
	nop

/*  get here if done */
inst54_done_8
	INC_KPC_3
	b	dispatch
	nop

inst54_out_of_time_8
/*  cycle have gone positive, just get out, don't update kpc */
	b,n	dispatch

inst54_notnat_8
	copy	dbank,ret0
	dep	scratch2,23,8,ret0
	CYCLES_PLUS_3
	depi	RET_MVN,3,4,ret0
	b	dispatch_done
	CYCLES_PLUS_3
#else
	IF_OPCODE_SUPPORTED(2) // 0x54
	{
		
		GET_2BYTE_ARG;
		/* arg & 0xff = dest bank, arg & 0xff00 = src bank */
		if(psr & 0x110) {
			// OG MVN should work even with no native mode - keep the warning though
			printf("MVN but not native m or x!\n");
		}
		CYCLES_MINUS_2;
		dbank = arg & 0xff;
		tmp1 = (arg >> 8) & 0xff;
		while(1) {
			CYCLES_PLUS_3;
			GET_MEMORY8((tmp1 << 16) + xreg, arg);
			SET_MEMORY8((dbank << 16) + yreg, arg);
			CYCLES_PLUS_2;
			xreg = (xreg + 1) & 0xffff;
			yreg = (yreg + 1) & 0xffff;
			acc = (acc - 1) & 0xffff;
			if(acc == 0xffff) {
				INC_KPC_3;
				break;
			}
			if(fcycles >= g_sim65816.g_fcycles_stop) {
				break;
			}
		}
	}
#endif

	break;
case 0x55:			/*  EOR Dloc,x */
	GET_DLOC_X_RD();
	EOR_INST();

	break;
case 0x56:			/*  LSR Dloc,X */
	GET_DLOC_X_RD();
	LSR_INST(1);

	break;
case 0x57:			/*  EOR [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	EOR_INST();
	}
	break;
case 0x58:			/*  CLI */
#ifdef ASM
	INC_KPC_1
	b	check_irqs_pending	/* check for ints pending! */
	depi	0,29,1,psr		/* clear int disable */
#else
	psr = psr & (~4);
	INC_KPC_1;
	if(((psr & 0x4) == 0) && g_moremem.g_irq_pending) {
		FINISH(RET_IRQ, 0);
	}
#endif

	break;
case 0x59:			/*  EOR abs,y */
	GET_ABS_Y_RD();
	EOR_INST();

	break;
case 0x5a:			/*  PHY */
#ifdef ASM
	INC_KPC_1
	ldil	l%dispatch,link
	bb,>=	psr,27,phy_16_8
	ldo	r%dispatch(link),link

	b	push_8
	copy	yreg,arg0

phy_16_8
	b	push_16
	copy	yreg,arg0
#else
	INC_KPC_1;
	if(psr & 0x10) {
		PUSH8(yreg);
	} else {
		PUSH16(yreg);
	}
#endif

	break;
case 0x5b:			/*  TCD */
#ifdef ASM
	extru	acc,31,16,direct
	INC_KPC_1
	copy	acc,zero
	b	dispatch
	extru	acc,16,1,neg
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	direct = acc;
	SET_NEG_ZERO16(acc);
	}
#endif

	break;
case 0x5c:			/*  JMP Long */
#ifdef ASM
	ldb	1(scratch1),kpc
	ldb	2(scratch1),scratch2
	CYCLES_PLUS_1
	ldb	3(scratch1),arg0		/* new bank */
	dep	scratch2,23,8,kpc
	b	dispatch
	dep	arg0,15,8,kpc
#else
	IF_OPCODE_SUPPORTED(2) // 0x5C
	{
	GET_3BYTE_ARG;
	CYCLES_PLUS_1;
	kpc = arg;
	}
#endif

	break;
case 0x5d:			/*  EOR Abs,X */
	GET_ABS_X_RD();
	EOR_INST();

	break;
case 0x5e:			/*  LSR Abs,X */
	GET_ABS_X_RD_WR();
	LSR_INST(0);

	break;
case 0x5f:			/*  EOR Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_RD();
	EOR_INST();
	}

	break;
case 0x60:			/*  RTS */
#ifdef ASM
	bl	pull_16,link
	CYCLES_PLUS_2
/*  ret0 is new kpc-1 */
	addi	1,ret0,ret0
	b	dispatch
	dep	ret0,31,16,kpc
#else
	CYCLES_PLUS_2
	PULL16(tmp1);
	kpc = (kpc & 0xff0000) + ((tmp1 + 1) & 0xffff);
#endif


	break;
case 0x61:			/*  ADC (Dloc,X) */
/*  called with arg = val to ADC in */
	GET_DLOC_X_IND_RD();
	ADC_INST();

	break;
case 0x62:			/*  PER */
#ifdef ASM
	ldb	1(scratch1),ret0
	INC_KPC_3
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_2
	ldil	l%dispatch,link
	dep	scratch1,23,8,ret0
	ldo	r%dispatch(link),link
	add	kpc,ret0,arg0
	b	push_16_unsafe
	extru	arg0,31,16,arg0
#else
	IF_OPCODE_SUPPORTED(2) // 0x62
	{
	GET_2BYTE_ARG;
	CYCLES_PLUS_2;
	INC_KPC_3;
	PUSH16_UNSAFE(kpc + arg);
	}
#endif

	break;
case 0x63:			/*  ADC Disp8,S */
/*  called with arg = val to ADC in */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_RD();
	ADC_INST();
	}
	break;
case 0x64:			/*  STZ Dloc */
	GET_DLOC_ADDR();
	STZ_INST(1);

	break;
case 0x65:			/*  ADC Dloc */
/*  called with arg = val to ADC in */
	GET_DLOC_RD();
	ADC_INST();

	break;
case 0x66:			/*  ROR Dloc */
	GET_DLOC_RD();
/*  save1 is now apple addr */
/*  ret0 is data */
	ROR_INST(1);

	break;
case 0x67:			/*  ADC [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	ADC_INST();
	}
	break;
case 0x68:			/*  PLA */
#ifdef ASM
# ifdef ACC8
	INC_KPC_1
	bl	pull_8,link
	CYCLES_PLUS_1
	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	dep	ret0,31,8,acc
# else
	INC_KPC_1
	bl	pull_16,link
	CYCLES_PLUS_1

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	extru	ret0,31,16,acc
# endif
#else
	INC_KPC_1;
	CYCLES_PLUS_1;
# ifdef ACC8
	PULL8(tmp1);
	acc = (acc & 0xff00) + tmp1;
	SET_NEG_ZERO8(tmp1);
# else
	PULL16(tmp1);
	acc = tmp1;
	SET_NEG_ZERO16(tmp1);
# endif
#endif


	break;
case 0x69:			/*  ADC #imm */
	GET_IMM_MEM();
	ADC_INST();

	break;
case 0x6a:			/*  ROR a */
#ifdef ASM
# ifdef ACC8
	extru	psr,31,1,neg
	INC_KPC_1
	extru	acc,30,7,zero
	dep	neg,24,1,zero
	dep	acc,31,1,psr			/* set carry */
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	psr,31,1,neg
	INC_KPC_1
	extru	acc,30,15,zero
	dep	neg,16,1,zero
	dep	acc,31,1,psr		/*  set carry */
	b	dispatch
	dep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
# ifdef ACC8
	tmp1 = ((acc & 0xff) >> 1) + ((psr & 1) << 7);
	SET_CARRY8((acc << 8));
	acc = (acc & 0xff00) + (tmp1 & 0xff);
	SET_NEG_ZERO8(tmp1 & 0xff);
# else
	tmp1 = (acc >> 1) + ((psr & 1) << 15);
	SET_CARRY16((acc << 16));
	acc = (tmp1 & 0xffff);
	SET_NEG_ZERO16(acc);
# endif
#endif

	break;
case 0x6b:			/*  RTL */
#ifdef ASM
	bl	pull_24,link
	CYCLES_PLUS_1
/*  ret0 is new kpc-1 */
	copy	ret0,kpc
	addi	1,ret0,scratch1
	b	dispatch
	dep	scratch1,31,16,kpc
	
#else
	IF_OPCODE_SUPPORTED(1)
	{
	CYCLES_PLUS_1;
	PULL24(tmp1);
	kpc = (tmp1 & 0xff0000) + ((tmp1 + 1) & 0xffff);
	}
#endif

	break;
case 0x6c:			/*  JMP (abs) */
#ifdef ASM
	ldb	1(scratch1),arg0
	CYCLES_PLUS_1
	ldb	2(scratch1),scratch1
	bl	get_mem_long_16,link
	dep	scratch1,23,8,arg0
/*  ret0 is addr to jump to */
	b	dispatch
	dep	ret0,31,16,kpc
#else
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	GET_MEMORY16(arg, tmp1, 1);
	kpc = (kpc & 0xff0000) + tmp1;
#endif

	break;
case 0x6d:			/*  ADC abs */
	GET_ABS_RD();
	ADC_INST();

	break;
case 0x6e:			/*  ROR abs */
	GET_ABS_RD();
	ROR_INST(0);

	break;
case 0x6f:			/*  ADC long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	ADC_INST();
	}

	break;
case 0x70:			/*  BVS disp8 */
#ifdef ASM
	COND_BR1
	bb,>=	psr,25,inst70_2_8
	COND_BR2

inst70_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8((psr & 0x40));
#endif

	break;
case 0x71:			/*  ADC (Dloc),y */
	GET_DLOC_IND_Y_RD();
	ADC_INST();

	break;
case 0x72:			/*  ADC (Dloc) */
	GET_DLOC_IND_RD();
	ADC_INST();

	break;
case 0x73:			/*  ADC (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	ADC_INST();
	}
	break;
case 0x74:			/*  STZ Dloc,x */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_X_WR();
	STZ_INST(1);
#else
	GET_1BYTE_ARG;
	GET_DLOC_X_WR();
	STZ_INST(1);
#endif

	break;
case 0x75:			/*  ADC Dloc,x */
	GET_DLOC_X_RD();
	ADC_INST();

	break;
case 0x76:			/*  ROR Dloc,X */
	GET_DLOC_X_RD();
	ROR_INST(1);

	break;
case 0x77:			/*  ADC [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	ADC_INST();
	}
	break;
case 0x78:			/*  SEI */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	1,29,1,psr		/* set int disable */
#else
	psr = psr | 4;
	INC_KPC_1;
#endif

	break;
case 0x79:			/*  ADC abs,y */
	GET_ABS_Y_RD();
	ADC_INST();

	break;
case 0x7a:			/*  PLY */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,inst7a_16bit_8
	nop

	bl	pull_8,link
	CYCLES_PLUS_1

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,yreg

inst7a_16bit_8
	bl	pull_16,link
	CYCLES_PLUS_1

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	copy	zero,yreg

#else
	INC_KPC_1;
	CYCLES_PLUS_1
	if(psr & 0x10) {
		PULL8(yreg);
		SET_NEG_ZERO8(yreg);
	} else {
		PULL16(yreg);
		SET_NEG_ZERO16(yreg);
	}
#endif

	break;
case 0x7b:			/*  TDC */
#ifdef ASM
	extru	direct,31,16,zero
	copy	direct,acc
	INC_KPC_1
	b	dispatch
	extru	direct,16,1,neg
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	acc = direct;
	SET_NEG_ZERO16(direct);
	}
#endif

	break;
case 0x7c:			/*  JMP (Abs,x) */
/*  always access kbank, xreg cannot wrap into next bank */
#ifdef ASM
	ldb	1(scratch1),ret0
	copy	kpc,scratch2
	ldb	2(scratch1),scratch1
	dep	xreg,31,16,scratch2
	CYCLES_PLUS_2
	dep	scratch1,23,8,ret0
	add	ret0,scratch2,arg0
	bl	get_mem_long_16,link
	extru	arg0,31,24,arg0
	b	dispatch
	dep	ret0,31,16,kpc
#else
	GET_2BYTE_ARG;
	arg = (kpc & 0xff0000) + ((xreg + arg) & 0xffff);
	CYCLES_PLUS_2;
	GET_MEMORY16(arg, tmp1, 1);
	kpc = (kpc & 0xff0000) + tmp1;
#endif

	break;
case 0x7d:			/*  ADC Abs,X */
	GET_ABS_X_RD();
	ADC_INST();

	break;
case 0x7e:			/*  ROR Abs,X */
	GET_ABS_X_RD_WR();
	ROR_INST(0);

	break;
case 0x7f:			/*  ADC Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_RD();
	ADC_INST();
	}

	break;
case 0x80:			/*  BRA */
#ifdef ASM
	COND_BR1
	COND_BR2
#else
	BRANCH_DISP8(1);
#endif


	break;
case 0x81:			/*  STA (Dloc,X) */
	GET_DLOC_X_IND_ADDR();
	STA_INST(0);

	break;
case 0x82:			/*  BRL disp16 */
#ifdef ASM
	ldb	1(scratch1),ret0
	CYCLES_PLUS_1
	ldb	2(scratch1),scratch1
	addi	3,kpc,scratch2
	dep	scratch1,23,8,ret0
	add	ret0,scratch2,scratch2
	b	dispatch
	dep	scratch2,31,16,kpc
#else
	IF_OPCODE_SUPPORTED(2)
	{
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	kpc = (kpc & 0xff0000) + ((kpc + 3 + arg) & 0xffff);
	}
#endif

	break;
case 0x83:			/*  STA Disp8,S */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_ADDR();
	STA_INST(1);
	}
	break;
case 0x84:			/*  STY Dloc */
	GET_DLOC_ADDR();
	STY_INST(1);


	break;
case 0x85:			/*  STA Dloc */
	GET_DLOC_ADDR();
	STA_INST(1);

	break;
case 0x86:			/*  STX Dloc */
	GET_DLOC_ADDR();
	STX_INST(1);


	break;
case 0x87:			/*  STA [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_ADDR();
	STA_INST(0);
	}
	break;
case 0x88:			/*  DEY */
#ifdef ASM
	INC_KPC_1
	bb,<	psr,27,inst88_8bit_8
	addi	-1,yreg,yreg
/*  16 bit */
	extru	yreg,31,16,zero
	extru	yreg,16,1,neg
	b	dispatch
	copy	zero,yreg

inst88_8bit_8
	extru	yreg,31,8,zero
	extru	yreg,24,1,neg
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_1;
	SET_INDEX_REG(yreg - 1, yreg);
#endif

	break;
case 0x89:			/*  BIT #imm */
#ifdef ASM
	GET_IMM_MEM();
# ifdef ACC8
/* Immediate BIT does not set condition flags */
	and	acc,ret0,zero
	b	dispatch
	extru	zero,31,8,zero
# else
	and	acc,ret0,zero
	b	dispatch
	extru	zero,31,16,zero
# endif
#else
	GET_IMM_MEM();
# ifdef ACC8
	zero = (acc & arg) & 0xff;
# else
	zero = (acc & arg) & 0xffff;
# endif
#endif

	break;
case 0x8a:			/*  TXA */
#ifdef ASM
# ifdef ACC8
	extru	xreg,31,8,zero
	INC_KPC_1
	extru	xreg,24,1,neg
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	xreg,31,16,zero
	INC_KPC_1
	extru	xreg,16,1,neg
	b	dispatch
	zdep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
	arg = xreg;
	LDA_INST();
#endif

	break;
case 0x8b:			/*  PHB */
#ifdef ASM
	ldil	l%dispatch,link
	extru	dbank,31,8,arg0
	INC_KPC_1
	b	push_8
	ldo	r%dispatch(link),link
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	PUSH8(dbank);
	}
#endif

	break;
case 0x8c:			/*  STY abs */
	GET_ABS_ADDR();
	STY_INST(0);

	break;
case 0x8d:			/*  STA abs */
	GET_ABS_ADDR();
	STA_INST(0);

	break;
case 0x8e:			/*  STX abs */
	GET_ABS_ADDR();
	STX_INST(0);


	break;
case 0x8f:			/*  STA long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_ADDR();
	STA_INST(0);
	}

	break;
case 0x90:			/*  BCC disp8 */
#ifdef ASM
	COND_BR1
	bb,<	psr,31,inst90_2_8
	COND_BR2

inst90_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8((psr & 0x01) == 0);
#endif


	break;
case 0x91:			/*  STA (Dloc),y */
	GET_DLOC_IND_Y_ADDR_FOR_WR();
	STA_INST(0);

	break;
case 0x92:			/*  STA (Dloc) */
	GET_DLOC_IND_ADDR();
	STA_INST(0);

	break;
case 0x93:			/*  STA (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_ADDR();
	STA_INST(0);
	}
	break;
case 0x94:			/*  STY Dloc,x */
	GET_DLOC_X_ADDR();
	STY_INST(1);

	break;
case 0x95:			/*  STA Dloc,x */
	GET_DLOC_X_ADDR();
	STA_INST(1);

	break;
case 0x96:			/*  STX Dloc,Y */
	GET_DLOC_Y_ADDR();
	STX_INST(1);

	break;
case 0x97:			/*  STA [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_ADDR();
	STA_INST(0);
	}
	break;
case 0x98:			/*  TYA */
#ifdef ASM
# ifdef ACC8
	extru	yreg,31,8,zero
	INC_KPC_1
	extru	yreg,24,1,neg
	b	dispatch
	dep	zero,31,8,acc
# else
	extru	yreg,31,16,zero
	INC_KPC_1
	extru	yreg,16,1,neg
	b	dispatch
	zdep	zero,31,16,acc
# endif
#else
	INC_KPC_1;
	arg = yreg;
	LDA_INST();
#endif

	break;
case 0x99:			/*  STA abs,y */
	GET_ABS_INDEX_ADDR_FOR_WR(yreg)
	STA_INST(0);

	break;
case 0x9a:			/*  TXS */
#ifdef ASM
	copy	xreg,stack
	extru,=	psr,23,1,0
	depi	1,23,24,stack
	INC_KPC_1
	b	dispatch
	nop
#else
	stack = xreg;
	if(psr & 0x100) {
		stack = 0x100 | (stack & 0xff);
	}
	INC_KPC_1;
#endif


	break;
case 0x9b:			/*  TXY */
#ifdef ASM
	extru	xreg,24,1,neg
	INC_KPC_1
	extru,<> psr,27,1,0		;skip next if 8bit
	extru	xreg,16,1,neg
	copy	xreg,yreg
	b	dispatch
	copy	xreg,zero
#else
	IF_OPCODE_SUPPORTED(1)
	{
	SET_INDEX_REG(xreg, yreg);
	INC_KPC_1;
	}
#endif


	break;
case 0x9c:			/*  STZ Abs */
	GET_ABS_ADDR();
	STZ_INST(0);

	break;
case 0x9d:			/*  STA Abs,X */
	GET_ABS_INDEX_ADDR_FOR_WR(xreg);
	STA_INST(0);

	break;
case 0x9e:			/*  STZ Abs,X */
	GET_ABS_INDEX_ADDR_FOR_WR(xreg);
	STZ_INST(0);

	break;
case 0x9f:			/*  STA Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_ADDR_FOR_WR();
	STA_INST(0);
	}

	break;
case 0xa0:			/*  LDY #imm */
#ifdef ASM
	INC_KPC_2
	bb,>=	psr,27,insta0_16bit_8
	ldb	1(scratch1),zero

	extru	zero,24,1,neg
	b	dispatch
	copy	zero,yreg
insta0_16bit_8
	ldb	2(scratch1),scratch1
	INC_KPC_1
	CYCLES_PLUS_1
	extru	scratch1,24,1,neg
	dep	scratch1,23,8,zero
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_2;
	if((psr & 0x10) == 0) {
		GET_2BYTE_ARG;
		CYCLES_PLUS_1
		INC_KPC_1;
	} else {
		GET_1BYTE_ARG;
	}
	SET_INDEX_REG(arg, yreg);
#endif


	break;
case 0xa1:			/*  LDA (Dloc,X) */
/*  called with arg = val to LDA in */
	GET_DLOC_X_IND_RD();
	LDA_INST();

	break;
case 0xa2:			/*  LDX #imm */
#ifdef ASM
	ldb	1(scratch1),zero
	bb,>=	psr,27,insta2_16bit_8
	INC_KPC_2;

	extru	zero,24,1,neg
	b	dispatch
	copy	zero,xreg
insta2_16bit_8
	ldb	2(scratch1),scratch1
	INC_KPC_1
	CYCLES_PLUS_1
	extru	scratch1,24,1,neg
	dep	scratch1,23,8,zero
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_2;
	if((psr & 0x10) == 0) {
		GET_2BYTE_ARG;
		CYCLES_PLUS_1
		INC_KPC_1;
	} else {
		GET_1BYTE_ARG;
	}
	SET_INDEX_REG(arg, xreg);
#endif

	break;
case 0xa3:			/*  LDA Disp8,S */
/*  called with arg = val to LDA in */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_RD();
	LDA_INST();
	}
	break;
case 0xa4:			/*  LDY Dloc */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_WR()
	b	get_yreg_from_mem
	nop
#else
	C_LDY_DLOC();
#endif

	break;
case 0xa5:			/*  LDA Dloc */
/*  called with arg = val to LDA in */
	GET_DLOC_RD();
	LDA_INST();

	break;
case 0xa6:			/*  LDX Dloc */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_WR()
	b	get_xreg_from_mem
	nop
#else
	C_LDX_DLOC();
#endif

	break;
case 0xa7:			/*  LDA [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	LDA_INST();
	}
	break;
case 0xa8:			/*  TAY */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,insta8_16bit_8
	extru	acc,31,8,zero

	extru	acc,24,1,neg
	b	dispatch
	copy	zero,yreg

insta8_16bit_8
	extru	acc,31,16,zero
	extru	acc,16,1,neg
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_1;
	SET_INDEX_REG(acc, yreg);
#endif

	break;
case 0xa9:			/*  LDA #imm */
	GET_IMM_MEM();
	LDA_INST();

	break;
case 0xaa:			/*  TAX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instaa_16bit_8
	extru	acc,31,8,zero

	extru	acc,24,1,neg
	b	dispatch
	copy	zero,xreg

instaa_16bit_8
	extru	acc,31,16,zero
	extru	acc,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(acc, xreg);
#endif

	break;
case 0xab:			/*  PLB */
#ifdef ASM
	INC_KPC_1
	bl	pull_8,link
	CYCLES_PLUS_1

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,dbank
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	CYCLES_PLUS_1
	PULL8(dbank);
	SET_NEG_ZERO8(dbank);
	}
#endif

	break;
case 0xac:			/*  LDY abs */
#ifdef ASM
	GET_ABS_ADDR()
	b	get_yreg_from_mem
	nop
#else
	C_LDY_ABS();
#endif


	break;
case 0xad:			/*  LDA abs */
	GET_ABS_RD();
	LDA_INST();

	break;
case 0xae:			/*  LDX abs */
#ifdef ASM
	GET_ABS_ADDR()
	b	get_xreg_from_mem
	nop
#else
	C_LDX_ABS();
#endif

	break;
case 0xaf:			/*  LDA long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	LDA_INST();
	}

	break;
case 0xb0:			/*  BCS disp8 */
#ifdef ASM
	COND_BR1
	bb,>=	psr,31,instb0_2_8
	COND_BR2

instb0_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8((psr & 0x01));
#endif

	break;
case 0xb1:			/*  LDA (Dloc),y */
	GET_DLOC_IND_Y_RD();
	LDA_INST();

	break;
case 0xb2:			/*  LDA (Dloc) */
	GET_DLOC_IND_RD();
	LDA_INST();

	break;
case 0xb3:			/*  LDA (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	LDA_INST();
	}
	break;
case 0xb4:			/*  LDY Dloc,x */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_X_WR();
	b	get_yreg_from_mem
	nop
#else
	C_LDY_DLOC_X();
#endif

	break;
case 0xb5:			/*  LDA Dloc,x */
	GET_DLOC_X_RD();
	LDA_INST();

	break;
case 0xb6:			/*  LDX Dloc,y */
#ifdef ASM
	ldb	1(scratch1),arg0
	GET_DLOC_Y_WR();
	b	get_xreg_from_mem
	nop
#else
	C_LDX_DLOC_Y();
#endif

	break;
case 0xb7:			/*  LDA [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	LDA_INST();
	}
	break;
case 0xb8:			/*  CLV */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	0,25,1,psr		/* clear overflow */
#else
	psr = psr & ~0x40;
	INC_KPC_1;
#endif

	break;
case 0xb9:			/*  LDA abs,y */
	GET_ABS_Y_RD();
	LDA_INST();

	break;
case 0xba:			/*  TSX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instba_16bit_8
	extru	stack,31,8,zero

	extru	stack,24,1,neg
	b	dispatch
	copy	zero,xreg
instba_16bit_8
	copy	stack,zero
	extru	stack,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(stack, xreg);
#endif

	break;
case 0xbb:			/*  TYX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instbb_16bit_8
	copy	yreg,xreg

/*  8 bit */
	extru	yreg,24,1,neg
	b	dispatch
	copy	yreg,zero
instbb_16bit_8
	extru	yreg,16,1,neg
	b	dispatch
	copy	yreg,zero
#else
	IF_OPCODE_SUPPORTED(1)
	{
	INC_KPC_1;
	SET_INDEX_REG(yreg, xreg);
	}
#endif

	break;
case 0xbc:			/*  LDY Abs,X */
#ifdef ASM
	GET_ABS_INDEX_ADDR_FOR_RD(xreg)
	b	get_yreg_from_mem
	nop
#else
	C_LDY_ABS_X();
#endif

	break;
case 0xbd:			/*  LDA Abs,X */
	GET_ABS_X_RD();
	LDA_INST();

	break;
case 0xbe:			/*  LDX Abs,y */
#ifdef ASM
	GET_ABS_INDEX_ADDR_FOR_RD(yreg)
	b	get_xreg_from_mem
	nop
#else
	C_LDX_ABS_Y();
#endif

	break;
case 0xbf:			/*  LDA Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_RD();
	LDA_INST();
	}

	break;
case 0xc0:			/*  CPY #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	bb,>=	psr,27,instc0_16bit_8
	INC_KPC_2;
	CMP_INDEX_REG_MEAT8(yreg)
instc0_16bit_8
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_1
	INC_KPC_1
	dep	scratch1,23,8,ret0
	CMP_INDEX_REG_MEAT16(yreg)
#else
	C_CPY_IMM();
#endif


	break;
case 0xc1:			/*  CMP (Dloc,X) */
/*  called with arg = val to CMP in */
	GET_DLOC_X_IND_RD();
	CMP_INST();

	break;
case 0xc2:			/*  REP #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	extru	psr,27,2,arg0		/* save old x & m */
	INC_KPC_2;
	dep	neg,24,1,psr
	CYCLES_PLUS_1
	depi	0,30,1,psr
	comiclr,<> 0,zero,0
	depi	1,30,1,psr
	andcm	psr,ret0,ret0
	ldi	0,zero
	extru,<> ret0,30,1,0
	ldi	1,zero
	dep	ret0,31,8,psr
	b	update_system_state
	extru	ret0,24,1,neg
#else
	IF_OPCODE_SUPPORTED(2)		// 0xC2
	{
	GET_1BYTE_ARG;
	tmp2 = psr;
	CYCLES_PLUS_1;
	INC_KPC_2;
	psr = (psr & ~0x82) | ((neg & 1) << 7) | ((!zero) << 1);
	psr = psr & ~(arg & 0xff);
	zero = !(psr & 2);
	neg = (psr >> 7) & 1;
	UPDATE_PSR(psr, tmp2);
	}
#endif


	break;
case 0xc3:			/*  CMP Disp8,S */
/*  called with arg = val to CMP in */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_RD();
	CMP_INST();
	}
	break;
case 0xc4:			/*  CPY Dloc */
#ifdef ASM
	GET_DLOC_ADDR()
	CMP_INDEX_REG_LOAD(instc4_16bit_8, yreg)
#else
	C_CPY_DLOC();
#endif


	break;
case 0xc5:			/*  CMP Dloc */
	GET_DLOC_RD();
	CMP_INST();

	break;
case 0xc6:			/*  DEC Dloc */
	GET_DLOC_RD();
	DEC_INST(1);

	break;
case 0xc7:			/*  CMP [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	CMP_INST();
	}
	break;
case 0xc8:			/*  INY */
#ifdef ASM
	INC_KPC_1
	addi	1,yreg,yreg
	bb,>=	psr,27,instc8_16bit_8
	extru	yreg,31,8,zero

	extru	yreg,24,1,neg
	b	dispatch
	copy	zero,yreg

instc8_16bit_8
	extru	yreg,31,16,zero
	extru	yreg,16,1,neg
	b	dispatch
	copy	zero,yreg
#else
	INC_KPC_1;
	SET_INDEX_REG(yreg + 1, yreg);
#endif

	break;
case 0xc9:			/*  CMP #imm */
	GET_IMM_MEM();
	CMP_INST();

	break;
case 0xca:			/*  DEX */
#ifdef ASM
	INC_KPC_1
	addi	-1,xreg,xreg
	bb,>=	psr,27,instca_16bit_8
	extru	xreg,31,8,zero

	extru	xreg,24,1,neg
	b	dispatch
	copy	zero,xreg

instca_16bit_8
	extru	xreg,31,16,zero
	extru	xreg,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(xreg - 1, xreg);
#endif

	break;
case 0xcb:			/*  WAI */
#ifdef ASM
	ldil	l%g_sim65816.g_wait_pending,scratch1
	CYCLES_FINISH
	ldi	1,scratch2
	b	dispatch
	stw	scratch2,r%g_sim65816.g_wait_pending(scratch1)
#else
	IF_OPCODE_SUPPORTED(1)
	{
	g_sim65816.g_wait_pending = 1;
	CYCLES_FINISH
	}
#endif

	break;
case 0xcc:			/*  CPY abs */
#ifdef ASM
	GET_ABS_ADDR()
	CMP_INDEX_REG_LOAD(instcc_16bit_8, yreg)
#else
	C_CPY_ABS();
#endif




	break;
case 0xcd:			/*  CMP abs */
	GET_ABS_RD();
	CMP_INST();

	break;
case 0xce:			/*  DEC abs */
	GET_ABS_RD();
	DEC_INST(0);


	break;
case 0xcf:			/*  CMP long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	CMP_INST();
	}

	break;
case 0xd0:			/*  BNE disp8 */
#ifdef ASM
	COND_BR1
	comib,=	0,zero,instd0_2_8
	COND_BR2

instd0_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(zero != 0);
#endif

	break;
case 0xd1:			/*  CMP (Dloc),y */
	GET_DLOC_IND_Y_RD();
	CMP_INST();

	break;
case 0xd2:			/*  CMP (Dloc) */
	GET_DLOC_IND_RD();
	CMP_INST();

	break;
case 0xd3:			/*  CMP (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	CMP_INST();
	}
	break;
case 0xd4:			/*  PEI Dloc */
#ifdef ASM
	GET_DLOC_ADDR()
	bl	get_mem_long_16,link
	CYCLES_PLUS_1

/*  push ret0 */
	extru	ret0,31,16,arg0
	ldil	l%dispatch,link
	b	push_16_unsafe
	ldo	r%dispatch(link),link
#else
	IF_OPCODE_SUPPORTED(2) // 0xD4
	{
	GET_DLOC_ADDR()
	GET_MEMORY16(arg, arg, 1);
	CYCLES_PLUS_1;
	PUSH16_UNSAFE(arg);
	}
#endif

	break;
case 0xd5:			/*  CMP Dloc,x */
	GET_DLOC_X_RD();
	CMP_INST();

	break;
case 0xd6:			/*  DEC Dloc,x */
	GET_DLOC_X_RD();
	DEC_INST(1);

	break;
case 0xd7:			/*  CMP [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	CMP_INST();
	}
	break;
case 0xd8:			/*  CLD */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	0,28,1,psr		/* clear decimal */
#else
	psr = psr & (~0x8);
	INC_KPC_1;
#endif

	break;
case 0xd9:			/*  CMP abs,y */
	GET_ABS_Y_RD();
	CMP_INST();

	break;
case 0xda:			/*  PHX */
#ifdef ASM
	INC_KPC_1
	bb,>=	psr,27,instda_16bit_8
	ldil	l%dispatch,link

	extru	xreg,31,8,arg0
	b	push_8
	ldo	r%dispatch(link),link

instda_16bit_8
	extru	xreg,31,16,arg0
	b	push_16
	ldo	r%dispatch(link),link
#else
	INC_KPC_1;
	if(psr & 0x10) {
		PUSH8(xreg);
	} else {
		PUSH16(xreg);
	}
#endif

	break;
case 0xdb:			/*  STP */
#ifdef ASM
	ldb	1(scratch1),ret0
	CYCLES_PLUS_1
	b	dispatch_done
	depi	RET_STP,3,4,ret0
#else
	IF_OPCODE_SUPPORTED(1)
	{
	CYCLES_FINISH
	FINISH(RET_STP, 0);
	}
#endif

	break;
case 0xdc:			/*  JML (Abs) */
#ifdef ASM
	ldb	1(scratch1),arg0
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_1
	bl	get_mem_long_24,link
	dep	scratch1,23,8,arg0

	b	dispatch
	copy	ret0,kpc
#else
	IF_OPCODE_SUPPORTED(2) // 0xDC
	{
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	GET_MEMORY24(arg, kpc, 1);
	}
#endif

	break;
case 0xdd:			/*  CMP Abs,X */
	GET_ABS_X_RD();
	CMP_INST();

	break;
case 0xde:			/*  DEC Abs,X */
	GET_ABS_X_RD_WR();
	DEC_INST(0);

	break;
case 0xdf:			/*  CMP Long,X */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_X_RD();
	CMP_INST();
	}

	break;
case 0xe0:			/*  CPX #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	bb,>=	psr,27,inste0_16bit_8
	INC_KPC_2;
	CMP_INDEX_REG_MEAT8(xreg)
inste0_16bit_8
	ldb	2(scratch1),scratch1
	CYCLES_PLUS_1
	INC_KPC_1
	dep	scratch1,23,8,ret0
	CMP_INDEX_REG_MEAT16(xreg)
#else
	C_CPX_IMM();
#endif


	break;
case 0xe1:			/*  SBC (Dloc,X) */
/*  called with arg = val to SBC in */
	GET_DLOC_X_IND_RD();
	SBC_INST();

	break;
case 0xe2:			/*  SEP #imm */
#ifdef ASM
	ldb	1(scratch1),ret0
	extru	psr,27,2,arg0		/* save old x & m */
	INC_KPC_2;
	dep	neg,24,1,psr
	CYCLES_PLUS_1
	depi	0,30,1,psr
	comiclr,<> 0,zero,0
	depi	1,30,1,psr
	or	psr,ret0,ret0
	ldi	0,zero
	extru,<> ret0,30,1,0
	ldi	1,zero
	dep	ret0,31,8,psr
	b	update_system_state
	extru	ret0,24,1,neg
#else
	IF_OPCODE_SUPPORTED(2) // 0xE2
	{
	GET_1BYTE_ARG;
	tmp2 = psr;
	CYCLES_PLUS_1;
	INC_KPC_2;
	psr = (psr & ~0x82) | ((neg & 1) << 7) | ((!zero) << 1);
	psr = psr | (arg & 0xff);
	zero = !(psr & 2);
	neg = (psr >> 7) & 1;
	UPDATE_PSR(psr, tmp2);
	}
#endif


	break;
case 0xe3:			/*  SBC Disp8,S */
/*  called with arg = val to SBC in */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_RD();
	SBC_INST();
	}
	break;
case 0xe4:			/*  CPX Dloc */
#ifdef ASM
	GET_DLOC_ADDR()
	CMP_INDEX_REG_LOAD(inste4_16bit_8, xreg)
#else
	C_CPX_DLOC();
#endif


	break;
case 0xe5:			/*  SBC Dloc */
/*  called with arg = val to SBC in */
	GET_DLOC_RD();
	SBC_INST();

	break;
case 0xe6:			/*  INC Dloc */
	GET_DLOC_RD();
	INC_INST(1);

	break;
case 0xe7:			/*  SBC [Dloc] */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_RD();
	SBC_INST();
	}
	break;
case 0xe8:			/*  INX */
#ifdef ASM
	INC_KPC_1
	addi	1,xreg,xreg
	bb,>=	psr,27,inste8_16bit_8
	extru	xreg,31,8,zero

	extru	xreg,24,1,neg
	b	dispatch
	copy	zero,xreg

inste8_16bit_8
	extru	xreg,31,16,zero
	extru	xreg,16,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	SET_INDEX_REG(xreg + 1, xreg);
#endif

	break;
case 0xe9:			/*  SBC #imm */
	GET_IMM_MEM();
	SBC_INST();

	break;
case 0xea:			/*  NOP */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	nop
#else
	INC_KPC_1;
#endif

	break;
case 0xeb:			/*  XBA */
#ifdef ASM
	extru	acc,16,1,neg		/* Z and N reflect status of low 8 */
	CYCLES_PLUS_1			/*   bits of final acc value! */
	copy	acc,scratch1		/* regardlessof ACC 8 or 16 bit */
	extru	acc,23,8,acc
	INC_KPC_1
	copy	acc,zero
	b	dispatch
	dep	scratch1,23,8,acc
#else
	IF_OPCODE_SUPPORTED(1)
	{
	tmp1 = acc & 0xff;
	CYCLES_PLUS_1
	acc = (tmp1 << 8) + (acc >> 8);
	INC_KPC_1;
	SET_NEG_ZERO8(acc & 0xff);
	}
#endif

	break;
case 0xec:			/*  CPX abs */
#ifdef ASM
	GET_ABS_ADDR()
	CMP_INDEX_REG_LOAD(instec_16bit_8, xreg)
#else
	C_CPX_ABS();
#endif




	break;
case 0xed:			/*  SBC abs */
	GET_ABS_RD();
	SBC_INST();

	break;
case 0xee:			/*  INC abs */
	GET_ABS_RD();
	INC_INST(0);


	break;
case 0xef:			/*  SBC long */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_LONG_RD();
	SBC_INST();
	}

	break;
case 0xf0:			/*  BEQ disp8 */
#ifdef ASM
	COND_BR1
	comib,<> 0,zero,instf0_2_8
	COND_BR2

instf0_2_8
	COND_BR_UNTAKEN
#else
	BRANCH_DISP8(zero == 0);
#endif

	break;
case 0xf1:			/*  SBC (Dloc),y */
	GET_DLOC_IND_Y_RD();
	SBC_INST();

	break;
case 0xf2:			/*  SBC (Dloc) */
	GET_DLOC_IND_RD();
	SBC_INST();

	break;
case 0xf3:			/*  SBC (Disp8,s),y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DISP8_S_IND_Y_RD();
	SBC_INST();
	}
	break;
case 0xf4:			/*  PEA Abs */
#ifdef ASM
	ldb	1(scratch1),arg0
	ldil	l%dispatch,link
	ldb	2(scratch1),scratch1
	INC_KPC_3
	CYCLES_PLUS_1
	ldo	r%dispatch(link),link
	b	push_16_unsafe
	dep	scratch1,23,8,arg0
#else
	IF_OPCODE_SUPPORTED(2) // 0xF4
	{
	GET_2BYTE_ARG;
	CYCLES_PLUS_1;
	INC_KPC_3;
	PUSH16_UNSAFE(arg);
	}
#endif

	break;
case 0xf5:			/*  SBC Dloc,x */
	GET_DLOC_X_RD();
	SBC_INST();

	break;
case 0xf6:			/*  INC Dloc,x */
	GET_DLOC_X_RD();
	INC_INST(1);

	break;
case 0xf7:			/*  SBC [Dloc],Y */
	IF_OPCODE_SUPPORTED(1)
	{
	GET_DLOC_L_IND_Y_RD();
	SBC_INST();
	}
	break;
case 0xf8:			/*  SED */
#ifdef ASM
	INC_KPC_1
	b	dispatch
	depi	1,28,1,psr		/* set decimal */
#else
	INC_KPC_1;
	psr |= 0x8;
#endif

	break;
case 0xf9:			/*  SBC abs,y */
	GET_ABS_Y_RD();
	SBC_INST();

	break;
case 0xfa:			/*  PLX */
#ifdef ASM
	bb,<	psr,27,instfa_8bit_8
	CYCLES_PLUS_1

	INC_KPC_1
	bl	pull_16,link
	nop

	extru	ret0,31,16,zero
	extru	ret0,16,1,neg
	b	dispatch
	copy	zero,xreg

instfa_8bit_8
	INC_KPC_1
	bl	pull_8,link
	nop

	extru	ret0,31,8,zero
	extru	ret0,24,1,neg
	b	dispatch
	copy	zero,xreg
#else
	INC_KPC_1;
	CYCLES_PLUS_1;
	if(psr & 0x10) {
		PULL8(xreg);
		SET_NEG_ZERO8(xreg);
	} else {
		PULL16(xreg);
		SET_NEG_ZERO16(xreg);
	}
#endif

	break;
case 0xfb:			/*  XCE */
#ifdef ASM
	extru	psr,27,2,arg0		/* save old x & m */
	INC_KPC_1
	extru	psr,23,1,scratch1	/* e bit */
	dep	psr,23,1,psr		/* copy carry to e bit */
	b	update_system_state
	dep	scratch1,31,1,psr	/* copy e bit to carry */
#else
	IF_OPCODE_SUPPORTED(1)
	{
	tmp2 = psr;
	INC_KPC_1;
	psr = (tmp2 & 0xfe) | ((tmp2 & 1) << 8) | ((tmp2 >> 8) & 1);
	UPDATE_PSR(psr, tmp2);
	}
#endif

	break;
case 0xfc:			/*  JSR (Abs,X) */
#ifdef ASM
	ldb	1(scratch1),ret0
	extru	kpc,15,8,scratch2
	ldb	2(scratch1),scratch1
	dep	scratch2,15,16,ret0
	INC_KPC_2;
	dep	scratch1,23,8,ret0
	add	xreg,ret0,arg0
	bl	get_mem_long_16,link
	extru	arg0,31,24,arg0

	CYCLES_PLUS_2
	extru	kpc,31,16,arg0
	ldil	l%dispatch,link
	dep	ret0,31,16,kpc
	b	push_16_unsafe
	ldo	r%dispatch(link),link
#else
	IF_OPCODE_SUPPORTED(2) // 0xFC
	{
	GET_2BYTE_ARG;
	INC_KPC_2;
	tmp1 = kpc;
	arg = (kpc & 0xff0000) + ((arg + xreg) & 0xffff);
	GET_MEMORY16(arg, tmp2, 1);
	kpc = (kpc & 0xff0000) + tmp2;
	CYCLES_PLUS_2
	PUSH16_UNSAFE(tmp1);
	}
#endif

	break;
case 0xfd:			/*  SBC Abs,X */
	GET_ABS_X_RD();
	SBC_INST();

	break;
case 0xfe:			/*  INC Abs,X */
	GET_ABS_X_RD_WR();
	INC_INST(0);

	break;
case 0xff:			/*  SBC Long,X */
	

		IF_OPCODE_SUPPORTED(1)
		{
		GET_LONG_X_RD();
		SBC_INST();
		}
		