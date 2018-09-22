/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../Common/CEMulatorCtrl.h"
#include "../kegs/Src/sim65816.h"

extern word32 g_font40_even_bits[0x100][8][16/4];
extern word32 g_font40_odd_bits[0x100][8][16/4];

word32 lastdrawtime = 0;
//void x_need2refresh();
#define MAXCONSOLE 1
int curConsole=0;
MyString Console[MAXCONSOLE];

extern void refresh_video(int);

void addConsole(const char* _str)
{
	MyString temp  = _str;
#ifndef UNDER_CE
	temp.ToLower();
#endif
	Console[curConsole++]=temp.c_str();
	if (curConsole==MAXCONSOLE) curConsole=0;
//	printf(temp.c_str());
	lastdrawtime = 0;
//	refresh_video(1);
}
void drawText(Kimage* _dst, const char* text,int _x,int _y);
void drawConsole(Kimage* _dst)
{	
	if ( !Console[0].GetLength() ) return ;
	if (!_dst) return ;

	/*
	word32 curtime = g_sim65816.g_vbl_count;
	if (lastdrawtime)
	{
		if (curtime>lastdrawtime+2*60)
		{		
			Console[0].Empty();
			return ;
		}
	}
	else
		lastdrawtime=curtime;
	 */

	int pixsize =  _dst->mdepth / 8 ;
	int stride  =_dst->width_act* pixsize;
	int x = 16, y = 16;
	int w=400, h=20;
	byte* ptr = _dst->data_ptr + y*stride + x*pixsize ;
	while(h--)
	{
		memset(ptr,0x77777777,w*pixsize);
		ptr+=stride;
	}

	for(int i=0;i<MAXCONSOLE;i++)
		drawText(_dst,Console[i].c_str(),x+2,i*14+2+y);

}

extern byte g_font_array[256][8] ;

void drawText(Kimage* _dst, const char* text,int _x,int _y)
{
	char c;

	int pixsize =  _dst->mdepth / 8 ;
	int stride  =_dst->width_act * pixsize ;
	byte* ptrline = _dst->data_ptr + _y*stride + _x*pixsize;
	
	while( (c=*text++) != 0 )
	{
		/*
		if (c==13)
		{
			curline++;
			curpos=0;
			break;
		}
		 */
		byte* ptr = ptrline;
		for(int j=0;j<8;j++)
		{
			word32* ptrw = (word32*)ptr;
		//	byte* ptr = (byte*)ptrline;
			for(int i=0;i<7;i++)
			{
				int mask = 1 << (7-i);
				int v = g_font_array[c][j] ;
				int on =  v & mask;
			//	int color = 0;
				if (!on)
				{
					const int color = -1;
					if (pixsize==1)
					{
						ptr[i]=color;
						ptr[i+1]=color;
					}
					else
					{
						ptrw[i] = color; 
						ptrw[i+1] = color;
					}
				}
			}
			ptr += stride;
		}
		ptrline += 8*pixsize;		
	}
}
