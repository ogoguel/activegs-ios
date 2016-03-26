/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#define IOR(val) ( (val) ? 0x80 : 0x00 )

#define ALTZP	(g_moremem.g_c068_statereg & 0x80)
/* #define PAGE2 (g_c068_statereg & 0x40) */
#define RAMRD	(g_moremem.g_c068_statereg & 0x20)
#define RAMWRT	(g_moremem.g_c068_statereg & 0x10)
#define RDROM	(g_moremem.g_c068_statereg & 0x08)
#define LCBANK2	(g_moremem.g_c068_statereg & 0x04)
#define ROMB	(g_moremem.g_c068_statereg & 0x02)
#define INTCX	(g_moremem.g_c068_statereg & 0x01)


#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

class s_moremem : public serialized
{
public:
	int	g_num_shadow_all_banks ;
	int	g_em_emubyte_cnt ;
	int	g_paddle_buttons ;
	int	g_irq_pending ;

	int	g_c023_val;

	int	g_c029_val_some ;
	int	g_c02b_val ;
	int	g_c02d_int_crom ;
	int	g_c031_disk35 ;
	int	g_c033_data ;
	int	g_c034_val ;
	int	g_c035_shadow_reg ;
	int	g_c036_val_speed ;
	int	g_c03ef_doc_ptr ;
	
	int	g_c041_val ;		/* C041_EN_25SEC_INTS, C041_EN_MOVE_INTS */
	int	g_c046_val ;
	int	g_c05x_annuncs ;
	int	g_c068_statereg ;
	int	g_c08x_wrdefram ;
	int	g_zipgs_unlock ;
	int	g_zipgs_reg_c059 ;
	// 7=LC cache dis, 6==5ms paddle del en, 5==5ms ext del en,
	// 4==5ms c02e enab, 3==CPS follow enab, 2-0: 111
	int	g_zipgs_reg_c05a ;
	// 7:4 = current ZIP speed, 0=100%, 1=93.75%, F=6.25%
	// 3:0: always 1111
	int	g_zipgs_reg_c05b;
	// 7==1ms clock, 6==cshupd: tag data at c05f updated
	// 5==LC cache disable, 4==bd is disabled, 3==delay in effect,
	// 2==rombank, 1-0==ram size (00:8K, 01=16K, 10=32K, 11=64K)
	int	g_zipgs_reg_c05c ;
	// 7:1==slot delay enable (for 52-54ms), 0==speaker 5ms delay
	int transwarp_low_val ;

	// from engine_c.cpp
	int	g_ret1;
	int	g_ret2;


	s_moremem()
	{
	
		INIT_SERIALIZED();

		g_c029_val_some = 0x41;
		g_c02b_val = 0x08;
		g_c035_shadow_reg = 0x08;
		g_c036_val_speed = 0x80;
		g_zipgs_reg_c059 = 0x5f;
		g_zipgs_reg_c05a = 0x0f;
		g_zipgs_reg_c05b = 0x40;
	}
	
	DEFAULT_SERIALIZE_IN_OUT ;

} XCODE_PRAGMA_PACK;

#ifdef _WIN32
#pragma pack(pop)
#endif

extern s_moremem g_moremem;

