/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../Src/defc.h"
#include "../Src/video.h"
#include "../Src/sim65816.h"
#import "emulatorView.h"


int x_frame_rate = -1;

@implementation emulatorView


-(void)activateEmulatorRefresh:(int)_frameRate
{
	if (displayLink)
		[self deactivateEmulatorRefresh];
	
	displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(directDraw:)];

    x_frame_rate = _frameRate;
	switch(_frameRate)
	{
		case 30:
			[displayLink setFrameInterval:2]; 
			g_video.g_screen_redraw_skip_amt = 1;
			break;
		case 60:
			[displayLink setFrameInterval:1]; 
			g_video.g_screen_redraw_skip_amt = -1;
			break;
		case 15:
			[displayLink setFrameInterval:4]; 
			g_video.g_screen_redraw_skip_amt = 3;
			break;
		case 20:
		default: // 20FPS
			[displayLink setFrameInterval:3]; 
			g_video.g_screen_redraw_skip_amt = 2;
			break;
	}
	// OG utiliser NSRunLoopCommonModes pour que le rendu ne s'arrete pas durant un uiscrollview
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
}

-(void)deactivateEmulatorRefresh
{
	displayLink=nil;
	
}

int x_video_need_to_be_refreshed = 0;
extern int x_vbl_count ;

void x_invalidrect()
{
	x_video_need_to_be_refreshed = 1;
}

- (void)directDraw:(CADisplayLink *)sender
{
	
	x_vbl_count++;
	addFrameRate(4);

	if (x_video_need_to_be_refreshed)
	{
		x_video_need_to_be_refreshed = 0;
		addFrameRate(3);
		[self setNeedsDisplay];
		
	}
    /*
#ifdef ACTIVEGS
    extern void x_update_framerate();
    x_update_framerate();
#endif
     */
}

-(void) drawRect:(CGRect) rect
{
   
    CGContextRef g = UIGraphicsGetCurrentContext();
	x_async_refresh(g,rect);

}



@end



