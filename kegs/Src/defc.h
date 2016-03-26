/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#ifdef INCLUDE_RCSID_C
const char rcsid_defc_h[] = "@(#)$KmKId: defc.h,v 1.100 2004-11-09 02:02:07-05 kentd Exp $";
#endif

#if !defined(VIDEO_SINGLEHLINE) && !defined(VIDEO_DOUBLEHLINE)
#	define VIDEO_DOUBLEHLINE
#endif

#if !defined(VIDEO_SINGLEVLINE) && !defined(VIDEO_DOUBLEVLINE)
#	define VIDEO_DOUBLEVLINE
#endif

#ifdef VIDEO_DOUBLEHLINE
#	define VIDEO_HFACTOR 2
#else
#	define VIDEO_HFACTOR 1
#endif


#ifdef VIDEO_DOUBLEVLINE
#	define VIDEO_VFACTOR 2
#else
#	define VIDEO_VFACTOR 1
#endif

#include "defcomm.h"

// OG redirect printf to console
#ifdef ACTIVEGS
#include <stdio.h>
extern "C" int outputInfo(const char* format,...);
extern "C" int fOutputInfo(FILE*,const char* format,...);
#define printf	outputInfo
#define fprintf	fOutputInfo
#ifdef _DEBUG
#define debug_printf(X,...) outputInfo(X,## __VA_ARGS__)
#else
#define debug_printf(X,...) 

#endif
#endif

#define STRUCT(a) typedef struct _ ## a a; struct _ ## a

typedef unsigned char byte;
typedef unsigned short word16;
typedef unsigned int word32;
#if _MSC_VER
typedef unsigned __int64 word64;
#else
typedef unsigned long long word64;
#endif

void U_STACK_TRACE();

/* 28MHz crystal, plus every 65th 1MHz cycle is stretched 140ns */
#define CYCS_28_MHZ		(28636360)
#define DCYCS_28_MHZ		(1.0*CYCS_28_MHZ)
#define CYCS_3_5_MHZ		(CYCS_28_MHZ/8)
#define DCYCS_1_MHZ		((DCYCS_28_MHZ/28.0)*(65.0*7/(65.0*7+1.0)))
#define CYCS_1_MHZ		((int)DCYCS_1_MHZ)

/* #define DCYCS_IN_16MS_RAW	(DCYCS_1_MHZ / 60.0) */
#define DCYCS_IN_16MS_RAW	(262.0 * 65.0)
/* Use precisely 17030 instead of forcing 60 Hz since this is the number of */
/*  1MHz cycles per screen */
#define DCYCS_IN_16MS		((double)((int)DCYCS_IN_16MS_RAW))
#define DRECIP_DCYCS_IN_16MS	(1.0 / (DCYCS_IN_16MS))

#ifdef KEGS_LITTLE_ENDIAN
# define BIGEND(a)    ((((a) >> 24) & 0xff) +			\
			(((a) >> 8) & 0xff00) + 		\
			((((unsigned)a) << 8) & 0xff0000) + 		\
			((((unsigned)a) << 24) & 0xff000000))
# define GET_BE_WORD16(a)	((((a) >> 8) & 0xff) + (((a) << 8) & 0xff00))
# define GET_BE_WORD32(a)	(BIGEND(a))
#else
# define BIGEND(a)	(a)
# define GET_BE_WORD16(a)	(a)
# define GET_BE_WORD32(a)	(a)
#endif

#define MAXNUM_HEX_PER_LINE     32

#ifdef __NeXT__
# include <libc.h>
#endif

#if !defined( _WIN32) && !defined(UNDER_CE)	// OG
# include <unistd.h>
# include <sys/ioctl.h>
# include <sys/wait.h>
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef UNDER_CE	// OG CE SPecific
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
// OG Adding support for open
#ifdef WIN32
#include <io.h>
#endif

#else
extern int errno;
extern int open(const char* name,int,...);
extern int read(int,char*,int);
extern int close(int);
extern int write(  int fd,  const void *buffer,  unsigned int count );
extern	int lseek(int,int,int);
struct stat { int st_size; };
extern int stat(const char* name, struct stat*);
extern int fstat(int, struct stat*);
#define	O_RDWR		1
#define O_BINARY	2
#define	O_RDONLY	4
#define	O_WRONLY	8
#define	O_CREAT		16
#define	O_TRUNC		32
#define EAGAIN		11
#define EINTR		4

#endif


#ifdef HPUX
# include <machine/inline.h>		/* for GET_ITIMER */
#endif

#ifdef SOLARIS
# include <sys/filio.h>
#endif


#ifndef O_BINARY
/* work around some Windows junk */
# define O_BINARY	0
#endif

STRUCT(Pc_log) {
	double	dcycs;
	word32	dbank_kpc;
	word32	instr;
	word32	psr_acc;
	word32	xreg_yreg;
	word32	stack_direct;
	word32	pad;
};

STRUCT(Data_log) {
	double	dcycs;
	word32	addr;
	word32	val;
	word32	size;
};

#if defined(_MSC_VER)
#define _ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define _ALIGNED(x) __attribute__ ((aligned(x)))
#endif
#endif

#define PRAGMA_PACK_SIZE	4
#ifdef WIN32
#	define XCODE_PRAGMA_PACK	
#else
#	define XCODE_PRAGMA_PACK __attribute__((aligned (PRAGMA_PACK_SIZE)))
#endif
#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

STRUCT(Event) {
	double	dcycs;
	int	type;
	Event	*next;
} XCODE_PRAGMA_PACK;

STRUCT(Fplus) {
	double	plus_1;
	double	plus_2;
	double	plus_3;
	double	plus_x_minus_1;
} XCODE_PRAGMA_PACK;

STRUCT(Engine_reg) {
	double	fcycles;
	word32	kpc;
	word32	acc;

	word32	xreg;
	word32	yreg;

	word32	stack;
	word32	dbank;

	word32	direct;
	word32	psr;
	Fplus	*fplus_ptr;
} XCODE_PRAGMA_PACK;

STRUCT(Kimage) {
	void	*dev_handle;
	void	*dev_handle2;
	byte	*data_ptr;
	int		data_size;
	int	width_req;
	int	width_act;
	int	height;
	int	depth;
	int	mdepth;
	int	aux_info;
} XCODE_PRAGMA_PACK;

typedef byte *Pg_info;
STRUCT(Page_info) {
	Pg_info rd_wr;
} XCODE_PRAGMA_PACK;

STRUCT(Cfg_menu) {
	const char *str;
	void	*ptr;
	const char *name_str;
	void	*defptr;
	int	cfgtype;
} XCODE_PRAGMA_PACK;

STRUCT(Cfg_dirent) {
	char	*name;
	int	is_dir;
	int	size;
	int	image_start;
	int	part_num;
} XCODE_PRAGMA_PACK;

STRUCT(Cfg_listhdr) {
	Cfg_dirent	*direntptr;
	int	max;
	int	last;
	int	invalid;

	int	curent;
	int	topent;

	int	num_to_show;
} XCODE_PRAGMA_PACK;

STRUCT(Emustate_intlist) {
	const char *str;
	int	*iptr;
} XCODE_PRAGMA_PACK;

STRUCT(Emustate_dbllist) {
	const char *str;
	double	*dptr;
} XCODE_PRAGMA_PACK;

STRUCT(Emustate_word32list) {
	const char *str;
	word32	*wptr;
} XCODE_PRAGMA_PACK;

#ifdef __LP64__
# define PTR2WORD(a)	((unsigned long)(a))
#else
# define PTR2WORD(a)	((unsigned int)(a))
#endif



#define C041_EN_25SEC_INTS	0x10
#define C041_EN_VBL_INTS	0x08
#define C041_EN_SWITCH_INTS	0x04
#define C041_EN_MOVE_INTS	0x02
#define C041_EN_MOUSE		0x01

/* WARNING: SCC1 and SCC0 interrupts must be in this order for scc.c */
/*  This order matches the SCC hardware */
#define IRQ_PENDING_SCC1_ZEROCNT	0x00001
#define IRQ_PENDING_SCC1_TX		0x00002
#define IRQ_PENDING_SCC1_RX		0x00004
#define IRQ_PENDING_SCC0_ZEROCNT	0x00008
#define IRQ_PENDING_SCC0_TX		0x00010
#define IRQ_PENDING_SCC0_RX		0x00020
#define IRQ_PENDING_C023_SCAN		0x00100
#define IRQ_PENDING_C023_1SEC		0x00200
#define IRQ_PENDING_C046_25SEC		0x00400
#define IRQ_PENDING_C046_VBL		0x00800
#define IRQ_PENDING_ADB_KBD_SRQ		0x01000
#define IRQ_PENDING_ADB_DATA		0x02000
#define IRQ_PENDING_ADB_MOUSE		0x04000
#define IRQ_PENDING_DOC			0x08000


#define EXTRU(val, pos, len) 				\
	( ( (len) >= (pos) + 1) ? ((val) >> (31-(pos))) : \
	  (((val) >> (31-(pos)) ) & ( (1<<(len) ) - 1) ) )

#define DEP1(val, pos, old_val)				\
	(((old_val) & ~(1 << (31 - (pos))) ) |		\
	 ( ((val) & 1) << (31 - (pos))) )

#define set_halt(val) \
	if(val) { set_halt_act(val); }

#define clear_halt() \
	clr_halt_act()

#define GET_PAGE_INFO_RD(page) \
	(r_sim65816.page_info_rd_wr[page].rd_wr)

#define GET_PAGE_INFO_WR(page) \
	(r_sim65816.page_info_rd_wr[0x10000 + PAGE_INFO_PAD_SIZE + (page)].rd_wr)

#define SET_PAGE_INFO_RD(page,val) \
	r_sim65816.page_info_rd_wr[page].rd_wr = (Pg_info)val;

#define SET_PAGE_INFO_WR(page,val) \
	r_sim65816.page_info_rd_wr[0x10000 + PAGE_INFO_PAD_SIZE + (page)].rd_wr = (Pg_info)val;

#define VERBOSE_DISK	0x001
#define VERBOSE_IRQ	0x002
#define VERBOSE_CLK	0x004
#define VERBOSE_SHADOW	0x008
#define VERBOSE_IWM	0x010
#define VERBOSE_DOC	0x020
#define VERBOSE_ADB	0x040
#define VERBOSE_SCC	0x080
#define VERBOSE_TEST	0x100
#define VERBOSE_VIDEO	0x200
#define VERBOSE_MAC	0x400

#if !defined(_DEBUG) || defined(NO_VERB)
# define DO_VERBOSE	0
#else
# define DO_VERBOSE	1
#endif

#define disk_printf	if(DO_VERBOSE && (Verbose & VERBOSE_DISK)) printf
#define irq_printf	if(DO_VERBOSE && (Verbose & VERBOSE_IRQ)) printf
#define clk_printf	if(DO_VERBOSE && (Verbose & VERBOSE_CLK)) printf
#define shadow_printf	if(DO_VERBOSE && (Verbose & VERBOSE_SHADOW)) printf
#define iwm_printf	if(DO_VERBOSE && (Verbose & VERBOSE_IWM)) printf
#define doc_printf	if(DO_VERBOSE && (Verbose & VERBOSE_DOC)) printf
#define adb_printf	if(DO_VERBOSE && (Verbose & VERBOSE_ADB)) printf
#define scc_printf	if(DO_VERBOSE && (Verbose & VERBOSE_SCC)) printf
#define test_printf	if(DO_VERBOSE && (Verbose & VERBOSE_TEST)) printf
#define vid_printf	if(DO_VERBOSE && (Verbose & VERBOSE_VIDEO)) printf
#define mac_printf	if(DO_VERBOSE && (Verbose & VERBOSE_MAC)) printf


#define HALT_ON_SCAN_INT	0x001
#define HALT_ON_IRQ		0x002
#define HALT_ON_SHADOW_REG	0x004
#define HALT_ON_C70D_WRITES	0x008

#define HALT_ON(a, msg)			\
	if(g_sim65816.Halt_on & a) {		\
		halt_printf(msg);	\
	}


#ifndef MIN
# define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b)	(((a) < (b)) ? (b) : (a))
#endif

#define GET_ITIMER(dest)	dest = get_itimer();

#include "protos.h"

// OG Added interfaces to communicate with the emulator
void x_clk_setup_bram_version();
void x_notify_disk_insertion(int _size, int _start, int _isprodos,int _slot, int _drive);
void x_notify_motor_status(int _motorOn,int _slot,int _drive, int _curtrack);
void x_notify_disk_ejection(int _slot,int _drive);
void x_refresh_video();

void x_exit(int _err);
void x_alert(const char* format,...);
void x_fatal_exit(const char* _str);

enum  refreshpanelenum 
{
	PANEL_DISK = 1,
	PANEL_RUNTIME = 2,
	PANEL_PARAMS = 4,
	PANEL_BOOT = 8,
    PANEL_LOADED = 16,
    PANEL_SPEED = 32,
	PANEL_ALL = -1
};

extern void x_refresh_panel(int _panel);


#include "compression.h"

#ifdef DRIVER_ANDROID
#define ALIGNMENT_ERROR(_MODULE,_SIZE,_THISSIZE) \
{	\
	char str[512];	\
	sprintf(str,"Alignment error in %s : got %d, this : %d",_MODULE,_SIZE,_THISSIZE);	\
	x_fatal_exit(str);	\
}
#else
#define ALIGNMENT_ERROR(_MODULE,_SIZE,_THISSIZE) \
{	\
	char str[512];	\
	sprintf(str,"Alignment error in %s : got %d, this : %ld",_MODULE,_SIZE,_THISSIZE);	\
	x_fatal_exit(str);	\
}
#endif
class serialize
{
public:
	word32 size;
	void* data;
	int fastalloc;
	int rlesize;
	void* rledata;
	int	rletype;
	serialize()
	{
		memset(this,0,sizeof(*this));

	}
	
	void release();
	int compress();
	int	expand();
	int	releasecompressed();
	int	releaseuncompressed();
	void save(const char* name,void*);
	int load(const char*_name,void* _fileptr);
} XCODE_PRAGMA_PACK;

class serialized
{
public:
	int	_size;
	serialized* _this;
	void in(serialize* _s);
	int out(serialize* _s, int _fastalloc);
	
} XCODE_PRAGMA_PACK;

#ifdef _WIN32
#pragma pack(pop)
#endif

#define INIT_SERIALIZED() \
	memset(this,0,sizeof(*this)); \
	_size = sizeof(*this); \
	_this = this;

#define DEFAULT_SERIALIZE_IN_OUT		\
	void in(serialize* _s)				\
	{									\
		byte* ptr = (byte*)_s->data;	\
		int ssize = *(int*)ptr;			\
		if (ssize!= sizeof(*this))		\
			ALIGNMENT_ERROR("n/a",ssize,sizeof(*this));	\
		ptr += sizeof(ssize);					\
		memcpy(this,ptr,sizeof(*this));		\
	}										\
	int out(serialize* _s, int _fastalloc)	\
	{										\
		int ssize = sizeof(*this);			\
		int size = ssize + sizeof(ssize);	\
		_s->size = size;					\
		_s->fastalloc = _fastalloc;			\
		_s->data = (void*)x_malloc(size,_fastalloc); \
		byte* ptr = (byte*) _s->data;	\
		memcpy(ptr,&ssize,sizeof(ssize));	\
		ptr+=sizeof(ssize);	\
		memcpy(ptr,this,sizeof(*this));	\
		return size;	\
	}
