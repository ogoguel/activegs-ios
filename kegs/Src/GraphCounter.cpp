/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "video.h"
#include "sim65816.h"
#include "graphcounter.h"
#include "raster.h"

#ifdef ENABLE_GRAPH

s_graph g_graph;

#define GRAPH_X	0
#define GRAPH_Y	100


void s_graph::add_graph(int _counter, float _value, int _vblcount)
{
	int v = _vblcount%NB_GRAPH_VALUES;
	values[_counter][v]=_value;
//	printf("addgraph (%d)(%f) vbl:%d %f\n", _counter,g_sim65816.g_last_vbl_dcycs,_vblcount,_value);
}

s_graph::s_graph()
{

	memset(this,0,sizeof(*this));

	// Counter 0 = frame rate
	min[0] = 5.0f;
	max[0] = 60.0f;
	ref[0] = 10.0f;
	color[0]=0xFF0000;	// RED

	// Counter 1 = frame/sleep precision
	min[1] = 55.0f;
	max[1] = 65.0f;
	ref[1] = 60.0f;
	color[1]=0x00FFFF;

	// Counter 2 = c054/page flipping
	min[2] = 5.0f;
	max[2] = 20.0f;
	ref[2] = 10.0f;
	color[2]=0xFFFFFF;

	// Counter 3 = sleep precision
	min[3] = -.020f;
	max[3] = 0.020f;
	ref[3] = 0.0f;
	color[3]=0x00FF00;


}

 

void s_graph::draw_graph(Kimage* _dest, int _curvbl)
{
	if (!_dest->data_ptr) return ;
	// Clear Rect
	
	int pw = (_dest->mdepth >> 3);
	int wd = _dest->width_act*pw;
	byte* ptr = _dest->data_ptr + GRAPH_X*pw + GRAPH_Y*wd;
	for(int h=0;h<NB_GRAPH_HEIGHT;h++)
	{
		memset(ptr,0,NB_GRAPH_VALUES*pw);
		ptr+=wd;
	}
	
	for(int c=0;c<NB_GRAPH;c++)
	{
		for(int v=0;v<NB_GRAPH_VALUES;v++)
		{
			float m;
			int h;
			float d = (max[c]-min[c]);

			float href = ref[c];
			m = (href - min[c]);

			h = (int)( m*NB_GRAPH_HEIGHT/d );
			if (h>=0 && h<NB_GRAPH_HEIGHT) 
			{
				word32 *ptr2 = (word32*)(_dest->data_ptr+ (h+GRAPH_Y)*wd + (v+GRAPH_X)*pw ) ;	
				*ptr2 = 0xFF;
			}
			int i = (_curvbl -v + NB_GRAPH_VALUES)%NB_GRAPH_VALUES;
			float va = values[c][i];
			m = (va - min[c]);
		
			h = (int)( m*NB_GRAPH_HEIGHT/d );
			if (h>=0 && h<NB_GRAPH_HEIGHT) 
			{
				word32 *ptr = (word32*)(_dest->data_ptr+ (h+GRAPH_Y)*wd + (v+GRAPH_X)*pw ) ;
				*ptr = color[c];
			}

		
			
		}
	}
}

#endif

#ifdef BORDER_SCAN

#define NB_BORDER 3
#define MAX_BORDER 100
int iBorder[NB_BORDER];
double reftime=0.0;
int borders[NB_BORDER][MAX_BORDER];
int colors[NB_BORDER][MAX_BORDER];

extern int screenHeight;

void change_border(int c,int _color)
{
	double t = get_dtime();
	double d = t - reftime;
	int l;
#ifdef WIN32
	if (c==2)
	{
		extern int screenHeight;
		l = getScanLine() * (float)X_A2_WINDOW_HEIGHT/(float)screenHeight;
	}
	else
#endif
		l = d * X_A2_WINDOW_HEIGHT / (1/60.0);

	if (l<0) l=0;
	if (l>X_A2_WINDOW_HEIGHT) l=X_A2_WINDOW_HEIGHT; 


	borders[c][iBorder[c]]=l;
	colors[c][iBorder[c]++]=_color;
//	ASSERT(iBorder<MAX_BORDER);
}


void draw_border()
{
#if 0
	for(int c=0;c<NB_BORDER;c++)
	{
		borders[c][iBorder[c]]=X_A2_WINDOW_HEIGHT;
	//	colors[iBorder]=0;

		extern Kimage g_kimage_offscreen;
		Kimage* image = &g_kimage_offscreen;
		//	int	bitmapBytesPerRow   = (pixelsWide * 4);// 1
		int bitmapBytesPerRow = image->width_act * (image->mdepth >> 3);

		
		// cherche le plus petit scanline
		int min=1000000;
		int imin=-1;
		for(int k=0;k<iBorder[c];k++)
		{
			if (borders[c][k]<min)
			{
				min=borders[c][k];
				imin=k;
			}
		}

		int col = 0;
		int done=0;
		int istart=imin;
		byte* ptrdest = (byte*)image->data_ptr ;
		for(int i=0;i<image->height;i+=1)
		{
			if (i>borders[c][imin] && !done)
			{
				col = colors[c][imin];
				imin++;
				if (imin>=iBorder[c])
					imin=0;
				if (imin==istart)
					done=1;
				
			}
			for(int j=0;j<32;j++)
				((word32*)ptrdest)[00+j+c*32] = col ;
			
		//	memset(ptrdest,col,bitmapBytesPerRow);
			ptrdest += bitmapBytesPerRow;
		//	col += 10;
		}
	}
#endif
	reset_border();
}

void reset_border()
{
	for(int c=0;c<NB_BORDER;c++)
	iBorder[c]=0;
	reftime = get_dtime();
}

#endif
