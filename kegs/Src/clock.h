/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#define CLK_IDLE		1
#define CLK_TIME		2
#define CLK_INTERNAL		3
#define CLK_BRAM1		4
#define CLK_BRAM2		5

#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

class s_clock : public serialized
{
public:
	int		g_clk_mode;
	int		g_clk_read;
	int		g_clk_reg1 ;

	byte	g_bram[2][256];
	byte	*g_bram_ptr ;

	word32	g_clk_cur_time ;
	int		g_clk_next_vbl_update ;
	
	s_clock()
	{
		INIT_SERIALIZED();

		g_clk_mode = CLK_IDLE;	
		g_clk_cur_time = 0xa0000000;
		g_bram_ptr = &(g_bram[0][0]);
	}

#define SERIALIZE_BRAM  g_bram_ptr = (byte*)( (g_bram_ptr  == &g_bram[0][0])?1:3 );
#define UNSERIALIZE_BRAM  g_bram_ptr = ((intptr_t)g_bram_ptr  ==1) ? &g_bram[0][0] : &g_bram[1][0]

	
	void in(serialize* _s)
	{
		byte* ptr = (byte*)_s->data;
		int ssize = *(int*)ptr;
		if (ssize!= sizeof(*this))
			ALIGNMENT_ERROR("clock",ssize,sizeof(*this));

		ptr += sizeof(ssize);

		memcpy(this,ptr,sizeof(*this));
		UNSERIALIZE_BRAM ;
	}

	int out(serialize* _s, int _fastalloc)
	{
		int ssize = sizeof(*this);
		int size = ssize + sizeof(ssize);
		_s->size = size;
		_s->fastalloc = _fastalloc;
		_s->data = (void*)x_malloc(size,_fastalloc);
		byte* ptr = (byte*) _s->data;
		memcpy(ptr,&ssize,sizeof(ssize));
		ptr+=sizeof(ssize);
		SERIALIZE_BRAM;
		memcpy(ptr,this,sizeof(*this));
		UNSERIALIZE_BRAM;
		return size;
	}

} XCODE_PRAGMA_PACK;
#ifdef _WIN32
#pragma pack(pop)
#endif
extern  s_clock g_clock;
