/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#import <UIKit/UIKit.h>
#include <QuartzCore/CADisplayLink.h>

extern void x_async_refresh(CGContextRef myContext,CGRect r, CGImageRef *imageRef);
extern void add_event_key(int,int); 
extern void add_event_mouse(int _x,int _y, int _state, int _button);
extern void add_event_modifier(unsigned int state)	;

extern int	g_warp_pointer;
extern int g_pause;


@interface emulatorView : UIView 
{
	CADisplayLink* displayLink;

}


-(void)activateEmulatorRefresh:(int)_frameRate;
-(void)deactivateEmulatorRefresh;


@end
