/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once


 #ifdef USE_RASTER

//#define TEST_RASTER
int getScanLine();
int x_init_raster(void*);
void x_close_raster();
 int x_wait_for_vbl();
 int x_wait_for_last_refresh(double);
int x_test_raster();
#endif