/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "mouseView.h"
#include "emulatorView.h"

@implementation mouseView


- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
    }
    return self;
}


- (void)drawRect:(CGRect)rect {
    // Drawing code
}

/*
- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	pt = [[touches anyObject] locationInView:self];
	
	
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	
	}



- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchesBegan:touches withEvent:event];
	
	 CGPoint ptdef = [[touches anyObject] locationInView:self];
	//CGPoint delta = ptdef - pt;
	mac_add_event_mouse(ptdef.x-pt.x,ptdef.y-pt.y,0,1);
	
}
*/

- (void)dealloc {
    [super dealloc];
}


@end
