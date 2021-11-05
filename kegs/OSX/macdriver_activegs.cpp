/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#if !defined(DRIVER_OSX) && !defined(DRIVER_IOS)
#error "missing DRIVER_IOS or DRIVER_OSX"
#endif

#include "../Src/defc.h"
#include "../Src/protos_macdriver.h"
#include "../Src/sim65816.h"
#include <math.h>
#include "../Src/video.h"
#include "../Src/graphcounter.h"
#include "../Src/async_event.h"
#include "../Src/StdString.h"
#include "../Src/driver.h"

#ifdef DRIVER_IOS
#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGBitmapContext.h>
#else
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVDisplayLink.h>
extern void addConsoleWindow(Kimage*);
#endif

extern int macUsingCoreGraphics;
extern void addConsoleWindow(Kimage*);

#ifdef ENABLEQD
extern WindowRef	g_main_window;
extern PixMapHandle	pixmap_backbuffer;
extern GWorldPtr	backbuffer;
#endif



int x_vbl_count = 0;


void x_check_system_input_events(void)
{
	/*
	while(get_state()==IN_PAUSE  && !should_emulator_terminate() )
		micro_sleep(0.1);
	 */
}


MyString messageLine;
int		 messageLineVBL=0;



void x_async_refresh(CGContextRef myContext,CGRect myBoundingBox)
{
	
#ifdef ENABLEQD
	CEmulatorMac* pEmu = (CEmulatorMac*)CEmulator::theEmulator;
	if (!pEmu) return ;
#endif
    
#ifndef DRIVER_IOS
	x_vbl_count++;
#endif
	
	addFrameRate(0);

	CHANGE_BORDER(1,0xFF);
	
	// OG
	if (macUsingCoreGraphics)
	{
		if(r_sim65816.is_emulator_offscreen_available() && g_kimage_offscreen.dev_handle)
		{
	
            /*
			void addConsoleWindow(Kimage* _dst);
			addConsoleWindow(&g_kimage_offscreen);
	*/
            
			CGContextSaveGState(myContext);
			
#ifndef DRIVER_IOS
		//	CGContextTranslateCTM(myContext,0.0, X_A2_WINDOW_HEIGHT);
        	CGContextTranslateCTM(myContext,0.0, myBoundingBox.size.height);    
			CGContextScaleCTM(myContext,1.0,-1.0);
#endif
			
			
			CGImageRef myImage = CGBitmapContextCreateImage((CGContextRef)g_kimage_offscreen.dev_handle);
            
            // IOS15 Hack, thanks to https://github.com/yoshisuga
            myBoundingBox.size.width = myBoundingBox.size.width+1;
            
			CGContextDrawImage(myContext, myBoundingBox, myImage);// 6
	
#ifndef VIDEO_SINGLEVLINE
			if (r_sim65816.get_video_fx() == VIDEOFX_CRT)
			{
                

				CGContextSetRGBFillColor(myContext,0,0,0,0.5);
				for(int h=0;h<g_kimage_offscreen.height;h+=2)
				{
					CGRect r = CGRectMake(0,h,g_kimage_offscreen.width_act,1);
					CGContextFillRect(myContext,r);
				}
                
			}            
           
#endif
            
			CGImageRelease(myImage);
		
			CGContextRestoreGState(myContext);
#ifndef DRIVER_IOS
			if (!messageLine.IsEmpty())
			{
				CGContextSaveGState(myContext);
				CGContextSetTextMatrix(myContext,CGAffineTransformIdentity);
				CGContextTranslateCTM(myContext,0.0, X_A2_WINDOW_HEIGHT);
				CGContextScaleCTM(myContext,1.0,-1.0);

				CGContextSelectFont(myContext, "Courier", 14.0, kCGEncodingMacRoman);
				CGContextSetTextDrawingMode(myContext, kCGTextFill);
				CGContextSetRGBFillColor (myContext, 1,1, 1, 1);
				CGContextSetShouldAntialias(myContext, true);
#define SHADOW 4.0
                

                CGFloat           myColorValues[] = {0.5, 0.5, 0.5, 1.0};
                
               
                CGColorSpaceRef  myColorSpace = CGColorSpaceCreateDeviceRGB ();// 9
                 CGColorRef  myColor = CGColorCreate (myColorSpace, myColorValues);
				CGContextSetShadowWithColor(myContext, CGSizeMake(SHADOW, -SHADOW), 4,
                                            myColor
                                            //CGColorCreateGenericGray(0.5,1.0)
                    
                                            );
				CGContextShowTextAtPoint(myContext, 20.0, X_A2_WINDOW_HEIGHT-20.0, messageLine.c_str(), messageLine.GetLength());
			
				CGContextRestoreGState(myContext);
				messageLineVBL--;
				if (messageLineVBL<0)
					messageLine.Empty();
				else 
					x_refresh_video();

			}
#endif
			
		}
		else
		{
			CGContextSaveGState(myContext);
#if defined(DRIVER_IOS)
            // efface en noir si l'émulateur n'avait pas encore démarré (le cas sur 3GS)
			CGContextSetRGBFillColor (myContext, 0, 0, 0, 1);
#else
            CGContextSetRGBFillColor (myContext, 0, 0, 1, 1);
#endif
			CGContextFillRect (myContext, CGRectMake (0, 0, X_A2_WINDOW_WIDTH, X_A2_WINDOW_HEIGHT));

			CGContextRestoreGState(myContext);

		}
		
	}
	else
	{
#ifdef ENABLEQD
		CGrafPtr window_port = pEmu->window_port;
		Rect src_rect;
		Rect dest_rect;
		SetRect(&src_rect,0,0,704,462);
		SetRect(&dest_rect,0,0,704,462);
		
		if (pixmap_backbuffer)
			CopyBits( (BitMap *)(*pixmap_backbuffer),
					 GetPortBitMapForCopyBits(window_port), &src_rect, &dest_rect,
					 srcCopy, NULL);
		
#endif
	}
	
	
	CHANGE_BORDER(1,0);
    
    if (r_sim65816.is_emulator_offscreen_available() && g_driver.x_handle_state_on_paint)
        g_driver.x_handle_state_on_paint(myBoundingBox.size.width,myBoundingBox.size.height);

}



void
x_redraw_status_lines()
{
}

void
x_push_done()
{
}
	
	
int macmain(int argc, char* argv[])
{	
	update_window();
	return kegsmain(0,NULL);
}
	
void
x_full_screen(int do_full)
{
}

void x_invalidrect();


int
x_show_alert(int is_fatal, const char *str)
{
	return 0;
}

void x_refresh_video()
{
		
	if (!r_sim65816.is_emulator_offscreen_available())
	{
		return ;
	}
	
	x_vbl_count = 0;
	
#ifdef ENABLE_GRAPH
	g_graph.draw_graph(&g_kimage_offscreen, g_sim65816.g_vbl_count);
#endif

	x_invalidrect();
}

#ifdef USE_RASTER


int x_init_raster(void* _data)
{
	return 1;
}


int x_close_raster()
{
	return 1;
}

// NANO SLEEP XP  http://www.dotnet247.com/247reference/msgs/57/289291.aspx

int x_wait_for_vbl()
{
	return 1;	
}

int x_wait_for_last_refresh(double _w)
{
	
	if (_w<0.0) 
	{
		// on est trop lent : ca a pris plus d'une 1/60ieme de seconde pour construire la frame
		return 0 ; 
	}
		
#ifdef DRIVER_IOS
	extern int x_frame_rate;
	if (x_frame_rate==60 && g_video_offscreen_has_been_modified && !x_vbl_count)
#else
	if (g_video_offscreen_has_been_modified && !x_vbl_count)
#endif
	{
		while(!x_vbl_count && !r_sim65816.should_emulator_terminate()) micro_sleep(0.01);
		x_vbl_count--;
	}
	else
	{
		if(_w>0.0) 
		{
			double s = get_dtime();
			if (_w>0.01)
				micro_sleep(_w);
			// sur windows le micro_sleep est lÈgËrement en retard!
			while( get_dtime() < s+_w);
		}
	}

	return 0;
}

#endif
