/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef ASM
# ifdef INCLUDE_RCSID_S
	.data
	.export rcsdif_op_routs_h,data
rcsdif_op_routs_h
	.stringz "@(#)$KmKId: op_routs.h,v 1.40 2004-01-10 15:49:46-05 kentd Exp $"
	.code
# endif

	.import	get_mem_b0_16,code
	.import	get_mem_b0_8,code

	.export	op_routs_start,data
op_routs_start	.word	0
#endif /* ASM */

#ifdef ASM
# define CMP_INDEX_REG_MEAT8(index_reg)		\
	extru	ret0,31,8,ret0			! \
	ldi	0xff,scratch3			! \
	subi	0x100,ret0,ret0			! \
	add	index_reg,ret0,ret0		! \
	extru	ret0,23,1,scratch1		! \
	and	ret0,scratch3,zero		! \
	extru	ret0,24,1,neg			! \
	b	dispatch			! \
	dep	scratch1,31,1,psr

# define CMP_INDEX_REG_MEAT16(index_reg)		\
	extru	ret0,31,16,ret0			! \
	ldil	l%0x10000,scratch2		! \
	zdepi	-1,31,16,scratch3		! \
	sub	scratch2,ret0,ret0		! \
	add	index_reg,ret0,ret0		! \
	extru	ret0,15,1,scratch1		! \
	and	ret0,scratch3,zero		! \
	extru	ret0,16,1,neg			! \
	b	dispatch			! \
	dep	scratch1,31,1,psr

# define CMP_INDEX_REG_LOAD(new_label, index_reg)	\
	bb,>=,n	psr,27,new_label		! \
	bl	get_mem_long_8,link		! \
	nop					! \
	CMP_INDEX_REG_MEAT8(index_reg)		! \
	.label	new_label			! \
	bl	get_mem_long_16,link		! \
	nop					! \
	CMP_INDEX_REG_MEAT16(index_reg)
#endif


#ifdef ASM
#define GET_DLOC_X_IND_WR()		\
	CYCLES_PLUS_1			! \
	add	xreg,direct,scratch2	! \
	INC_KPC_2			! \
	add	scratch2,arg0,arg0	! \
	bl	get_mem_b0_direct_page_16,link	! \
	extru	arg0,31,16,arg0		! \
	copy	ret0,arg0		! \
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	dep	dbank,15,8,arg0
#else /* C */
# define GET_DLOC_X_IND_WR()			\
	CYCLES_PLUS_1;				\
	INC_KPC_2;				\
	if(direct & 0xff) {			\
		CYCLES_PLUS_1;			\
	}					\
	arg = arg + xreg + direct;		\
	GET_MEMORY_DIRECT_PAGE16(arg & 0xffff, arg);	\
	arg = (dbank << 16) + arg;
#endif


#ifdef ASM
# define GET_DLOC_X_IND_ADDR()		\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_X_IND_WR()
#else /* C */
# define GET_DLOC_X_IND_ADDR()		\
	GET_1BYTE_ARG;			\
	GET_DLOC_X_IND_WR()
#endif

#ifdef ASM
# define GET_DISP8_S_WR()		\
	CYCLES_PLUS_1			! \
	add	stack,arg0,arg0		! \
	INC_KPC_2			! \
	extru	arg0,31,16,arg0
#else /* C */
#define GET_DISP8_S_WR()		\
	CYCLES_PLUS_1;			\
	arg = (arg + stack) & 0xffff;	\
	INC_KPC_2;
#endif


#ifdef ASM
# define GET_DISP8_S_ADDR()		\
	ldb	1(scratch1),arg0	! \
	GET_DISP8_S_WR()
#else /* C */
# define GET_DISP8_S_ADDR()		\
	GET_1BYTE_ARG;			\
	GET_DISP8_S_WR()
#endif

#ifdef ASM
# define GET_DLOC_WR()			\
	INC_KPC_2			! \
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	add	direct,arg0,arg0	! \
	extru	arg0,31,16,arg0
#else /* C */
# define GET_DLOC_WR()			\
	arg = (arg + direct) & 0xffff;	\
	if(direct & 0xff) {		\
		CYCLES_PLUS_1;		\
	}				\
	INC_KPC_2;
#endif

#ifdef ASM
# define GET_DLOC_ADDR()		\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_WR()
#else /* C */
# define GET_DLOC_ADDR()		\
	GET_1BYTE_ARG;			\
	GET_DLOC_WR()
#endif

#ifdef ASM
# define GET_DLOC_L_IND_WR()		\
	INC_KPC_2			! \
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	add	direct,arg0,arg0	! \
	bl	get_mem_b0_24,link	! \
	extru	arg0,31,16,arg0		! \
	copy	ret0,arg0
#else /* C */
# define GET_DLOC_L_IND_WR()		\
	arg = (arg + direct) & 0xffff;	\
	if(direct & 0xff) {		\
		CYCLES_PLUS_1;		\
	}				\
	INC_KPC_2;			\
	GET_MEMORY24(arg, arg, 1);
#endif


#ifdef ASM
# define GET_DLOC_L_IND_ADDR()		\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_L_IND_WR()
#else /* C */
# define GET_DLOC_L_IND_ADDR()		\
	GET_1BYTE_ARG;			\
	GET_DLOC_L_IND_WR()
#endif

#ifdef ASM
# define GET_DLOC_IND_Y_ADDR_FOR_WR()	\
	ldb	1(scratch1),arg0	! \
	CYCLES_PLUS_1			! \
	GET_DLOC_IND_Y_WR_SPECIAL()
#else /* C */
# define GET_DLOC_IND_Y_ADDR_FOR_WR()					\
	GET_1BYTE_ARG;							\
	if(direct & 0xff) {						\
		CYCLES_PLUS_1;						\
	}								\
	GET_MEMORY_DIRECT_PAGE16((direct + arg) & 0xffff, tmp1);	\
	tmp1 += (dbank << 16);						\
	arg = tmp1 + yreg;						\
	CYCLES_PLUS_1;							\
	INC_KPC_2;
#endif


#ifdef ASM
# define GET_DLOC_IND_WR()		\
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	INC_KPC_2			! \
	add	direct,arg0,arg0	! \
	bl	get_mem_b0_direct_page_16,link	! \
	extru	arg0,31,16,arg0		! \
	copy	ret0,arg0		! \
	dep	dbank,15,16,arg0
#else /* C */
# define GET_DLOC_IND_WR()		\
	INC_KPC_2;			\
	if(direct & 0xff) {		\
		CYCLES_PLUS_1;		\
	}				\
	GET_MEMORY_DIRECT_PAGE16((direct + arg) & 0xffff, arg);	\
	arg = (dbank << 16) + arg;
#endif


#ifdef ASM
# define GET_DLOC_IND_ADDR()		\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_IND_WR()
#else
# define GET_DLOC_IND_ADDR()		\
	GET_1BYTE_ARG;			\
	GET_DLOC_IND_WR();
#endif

#ifdef ASM
#define GET_DLOC_INDEX_WR(index_reg)	\
	GET_DLOC_INDEX_WR_A(index_reg) ! GET_DLOC_INDEX_WR_B(index_reg)

#define GET_DLOC_INDEX_WR_A(index_reg)	\
	CYCLES_PLUS_1			! \
	add	index_reg,direct,scratch2	! \
	extru	direct,23,8,scratch1	! \
	INC_KPC_2			! \
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	bb,>=	psr,23,.+16		! \
/* 4*/	add	scratch2,arg0,arg0	! \
/* 8*/	extru,<> direct,31,8,0		! \
/*12*/	dep	scratch1,23,8,arg0

/* GET_DLOC_INDeX_WR_B must be exactly one instruction! */
#define GET_DLOC_INDEX_WR_B(index_reg)	\
/*16*/	extru	arg0,31,16,arg0

#define GET_DLOC_Y_WR()			\
	GET_DLOC_INDEX_WR(yreg)

#define GET_DLOC_X_WR()			\
	GET_DLOC_INDEX_WR(xreg)

#define GET_DLOC_Y_ADDR()			\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_Y_WR()

# define GET_DLOC_X_ADDR()			\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_X_WR()

#else
# define GET_DLOC_INDEX_WR(index_reg)	\
	CYCLES_PLUS_1;			\
	arg = (arg & 0xff) + index_reg;	\
	INC_KPC_2;			\
	if(direct & 0xff) {		\
		CYCLES_PLUS_1;		\
	}				\
	if((psr & 0x100) && ((direct & 0xff) == 0)) {	\
		arg = (arg & 0xff);	\
	}				\
	arg = (arg + direct) & 0xffff;

# define GET_DLOC_X_WR()	\
	GET_DLOC_INDEX_WR(xreg)
# define GET_DLOC_Y_WR()	\
	GET_DLOC_INDEX_WR(yreg)

# define GET_DLOC_X_ADDR()	\
	GET_1BYTE_ARG;		\
	GET_DLOC_INDEX_WR(xreg)

# define GET_DLOC_Y_ADDR()	\
	GET_1BYTE_ARG;		\
	GET_DLOC_INDEX_WR(yreg)
#endif


#ifdef ASM
# define GET_DISP8_S_IND_Y_WR()		\
	add	stack,arg0,arg0		! \
	bl	get_mem_b0_16,link	! \
	extru	arg0,31,16,arg0		! \
	dep	dbank,15,16,ret0	! \
	CYCLES_PLUS_2			! \
	add	ret0,yreg,arg0		! \
	INC_KPC_2			! \
	extru	arg0,31,24,arg0

# define GET_DISP8_S_IND_Y_ADDR()	\
	ldb	1(scratch1),arg0	! \
	GET_DISP8_S_IND_Y_WR()
#else /* C */

# define GET_DISP8_S_IND_Y_WR()		\
	arg = (stack + arg) & 0xffff;	\
	GET_MEMORY16(arg,arg,1);	\
	CYCLES_PLUS_2;			\
	arg += (dbank << 16);		\
	INC_KPC_2;			\
	arg = (arg + yreg) & 0xffffff;

# define GET_DISP8_S_IND_Y_ADDR()	\
	GET_1BYTE_ARG;			\
	GET_DISP8_S_IND_Y_WR()
#endif


#ifdef ASM
# define GET_DLOC_L_IND_Y_WR()		\
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	INC_KPC_2			! \
	add	direct,arg0,arg0	! \
	bl	get_mem_b0_24,link	! \
	extru	arg0,31,16,arg0		! \
	add	ret0,yreg,arg0		! \
	extru	arg0,31,24,arg0

# define GET_DLOC_L_IND_Y_ADDR()	\
	ldb	1(scratch1),arg0	! \
	GET_DLOC_L_IND_Y_WR()
#else /* C */

# define GET_DLOC_L_IND_Y_WR()		\
	arg = (direct + arg) & 0xffff;	\
	if(direct & 0xff) {		\
		CYCLES_PLUS_1;		\
	}				\
	GET_MEMORY24(arg,arg,1);	\
	INC_KPC_2;			\
	arg = (arg + yreg) & 0xffffff;

# define GET_DLOC_L_IND_Y_ADDR()	\
	GET_1BYTE_ARG;			\
	GET_DLOC_L_IND_Y_WR()
#endif


#ifdef ASM
# define GET_ABS_ADDR()			\
	ldb	1(scratch1),arg0	! \
	ldb	2(scratch1),scratch1	! \
	CYCLES_PLUS_1			! \
	dep	dbank,15,8,arg0		! \
	INC_KPC_3			! \
	dep	scratch1,23,8,arg0

# define GET_LONG_ADDR()		\
	ldb	1(scratch1),arg0	! \
	ldb	2(scratch1),scratch2	! \
	CYCLES_PLUS_2			! \
	ldb	3(scratch1),scratch1	! \
	INC_KPC_4			! \
	dep	scratch2,23,8,arg0	! \
	dep	scratch1,15,8,arg0
#else /* C */

# define GET_ABS_ADDR()			\
	GET_2BYTE_ARG;			\
	CYCLES_PLUS_1;			\
	arg = arg + (dbank << 16);	\
	INC_KPC_3;

# define GET_LONG_ADDR()		\
	GET_3BYTE_ARG;			\
	CYCLES_PLUS_2;			\
	INC_KPC_4;
#endif

#ifdef ASM
#define GET_ABS_INDEX_ADDR_FOR_WR(index_reg)	\
	ldb	1(scratch1),arg0	! \
	copy	index_reg,scratch3	! \
	ldb	2(scratch1),scratch2	! \
	dep	dbank,15,8,scratch3	! \
	INC_KPC_3			! \
	dep	scratch2,23,8,arg0	! \
	CYCLES_PLUS_2			! \
	add	arg0,scratch3,arg0	! \
	extru	arg0,31,24,arg0

#define GET_LONG_X_ADDR_FOR_WR()	\
	ldb	3(scratch1),scratch2	! \
	copy	xreg,scratch3		! \
	ldb	1(scratch1),arg0	! \
	ldb	2(scratch1),scratch1	! \
	CYCLES_PLUS_2			! \
	dep	scratch2,15,8,scratch3	! \
	INC_KPC_4			! \
	dep	scratch1,23,8,arg0	! \
	add	arg0,scratch3,arg0	! \
	extru	arg0,31,24,arg0
#else /* C */

#define GET_ABS_INDEX_ADDR_FOR_WR(index_reg)	\
	GET_2BYTE_ARG;			\
	arg = arg + (dbank << 16);	\
	INC_KPC_3;			\
	CYCLES_PLUS_2;			\
	arg = (arg + index_reg) & 0xffffff;

#define GET_LONG_X_ADDR_FOR_WR()		\
	GET_3BYTE_ARG;			\
	INC_KPC_4;			\
	arg = (arg + xreg) & 0xffffff;	\
	CYCLES_PLUS_2;

#endif /* ASM */


#ifdef ASM
	.export	op_routs_end,data
op_routs_end	.word	0


#define GET_DLOC_IND_Y_WR_SPECIAL()	\
	add	direct,arg0,arg0	! \
	extru,=	direct,31,8,0		! \
	CYCLES_PLUS_1			! \
	bl	get_mem_b0_direct_page_16,link	! \
	extru	arg0,31,16,arg0		! \
	dep	dbank,15,8,ret0		! \
	INC_KPC_2			! \
	add	yreg,ret0,arg0			/* don't change this instr */
						/*  or add any after */
						/*  to preserve ret0 & arg0 */


/* cycle calc:  if yreg is 16bit or carry into 2nd byte, inc cycle */
/* So, if y==16bit, add 1.  If x==8bit, add 1 if carry */
get_dloc_ind_y_rd_8
	stw	link,STACK_SAVE_OP_LINK(sp)
	GET_DLOC_IND_Y_WR_SPECIAL()
	xor	arg0,ret0,scratch1
	extru,=	psr,27,1,0
	extru,=	scratch1,23,8,0
	CYCLES_PLUS_1
	b	get_mem_long_8
	ldw	STACK_SAVE_OP_LINK(sp),link

get_dloc_ind_y_rd_16
	stw	link,STACK_SAVE_OP_LINK(sp)
	GET_DLOC_IND_Y_WR_SPECIAL()
	xor	arg0,ret0,scratch1
	extru,=	psr,27,1,0
	extru,=	scratch1,23,8,0
	CYCLES_PLUS_1
	b	get_mem_long_16
	ldw	STACK_SAVE_OP_LINK(sp),link



#endif /* ASM */

