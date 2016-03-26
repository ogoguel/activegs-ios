/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#if defined(_DEBUG) && defined(_WIN32)
//#define ENABLE_GRAPH
#endif

#if defined(USE_RASTER) && defined(_DEBUG)
#define BORDER_SCAN
#endif

#ifdef BORDER_SCAN
void change_border(int,int);
void draw_border();
void reset_border();
#define RESET_BORDER() reset_border()
#define CHANGE_BORDER(ID,X) change_border(ID,X)
#define DRAW_BORDER() draw_border()
#else
#define CHANGE_BORDER(ID,X)
#define DRAW_BORDER()
#define RESET_BORDER()

#endif


#ifdef ENABLE_GRAPH

#define NB_GRAPH	4
#define NB_GRAPH_VALUES	300
#define NB_GRAPH_HEIGHT	100

class s_graph
{
	float min[NB_GRAPH];
	float max[NB_GRAPH];
	float ref[NB_GRAPH];
	int color[NB_GRAPH];
	float values[NB_GRAPH][NB_GRAPH_VALUES];
	
public:
	s_graph();
	void add_graph(int _counter, float _value, int _vblcount);
	void draw_graph(Kimage* _dest, int _curvbl);
};

extern s_graph g_graph;
#endif // ENABLE_GRAPH