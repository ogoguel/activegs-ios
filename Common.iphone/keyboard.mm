/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "keyboard.h"
#import "kbdlayout.h"


@implementation keyboard


- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
    }
    return self;
}

- (void)setParam:(const char*) _v
{
	v = _v;
	
}
 
- (void)drawRect:(CGRect)rect {
    // Drawing code
	CGContextRef g = UIGraphicsGetCurrentContext();
	CGContextSetFillColorWithColor(g, [UIColor grayColor].CGColor);
	CGContextFillRect(g, rect);
	
	//draw text in black.
	CGContextSetFillColorWithColor(g, [UIColor blackColor].CGColor);
	int size;
	NSString* str = [NSString stringWithFormat:@"%s", v];
	if ([str length]==1)
		size =12;
	else {
		size = 8;
	}

	[str drawAtPoint:CGPointMake(2.0, 2.0) withFont:[UIFont systemFontOfSize:size]];
	
	
}


- (void)dealloc {
    [super dealloc];
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint pt = [[touches anyObject] locationInView:self];
	startLocation = pt;
	kbdselected* sel =[kbdselected getSelected];
	[[self superview] bringSubviewToFront:sel];
	CGRect frame = [self frame];
	CGRect framesel = [sel frame];
	
	
	framesel.origin.x = frame.origin.x -10,0;
	framesel.origin.y = frame.origin.y - 40.0;
	
	/*
	framesel.origin.x = pt.x -10,0;
	framesel.origin.y = pt.y - 50.0;
	*/
	framesel.size.width = frame.size.width * 2.0;
	framesel.size.height = frame.size.height*2.0;
	[sel setFrame:framesel];
	
	[sel setParam:v];
	[sel setNeedsDisplay];
	sel.hidden=false;
	
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	kbdselected* sel =[kbdselected getSelected];
	sel.hidden=true;
	/*
	CGRect frame = [self frame];
	frame.origin.x += 10.0;
	frame.origin.y += 30.0;
	frame.size.width /=2.0;
	frame.size.height /=2.0;
	[self setFrame:frame];
	 */
}



- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	/*
	CGPoint pt = [[touches anyObject] locationInView:self];
	CGRect frame = [self frame];
	frame.origin.x += pt.x - startLocation.x;
	frame.origin.y += pt.y - startLocation.y;
	[self setFrame:frame];
	*/
	
	
}

@end
