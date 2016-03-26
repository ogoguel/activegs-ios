/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "kbdlayout.h"
#import "keyboard.h"
#import "activegsAppDelegate.h"
#include "../src91/protos_macdriver.h"
extern void add_event_key(int,int); 

// http://farm3.static.flickr.com/2448/3825169053_859571c493_o.jpg


#define KBDWIDTH 22
#define KBDWIDTHTAB 25
#define KBDWIDTHCTRL 30
#define KBDWIDTHRET 30
#define KBDWIDTHSHIFT 42
#define KBDWIDTHAPPLE 30
#define KBDWIDTHSPACE 100
struct kbdDefinition kbdDefinitions[]=
{
{ KBDWIDTH,"esc",0x35,0,0 },
{ KBDWIDTH,"1",0x12,"!",0 },
{ KBDWIDTH,"2",0x13,"@",0 },
{ KBDWIDTH,"3",0x14,"#",0 },
{ KBDWIDTH,"4",0x15,"$",0 },
{ KBDWIDTH,"5",0x17,"%",0 },
{ KBDWIDTH,"6",0x16,"^",0 },
{ KBDWIDTH,"7",0x1A,"&",0 },
{ KBDWIDTH,"8",0x1C,"*",0 },
{ KBDWIDTH,"9",0x19,"(",0 },
{ KBDWIDTH,"0",0x1D,")",0 },
{ KBDWIDTH,"-",0x1B,"_",0 },
{ KBDWIDTH,"=",0x18,"+",0 },
{ KBDWIDTHTAB,"delete",0x33,0,0 },
{ -1,0,0,0,0 },
{ KBDWIDTHTAB,"tab",0x30,0,0 },	
{ KBDWIDTH,"q",0x0C,"Q",0 },
{ KBDWIDTH,"w",0x0D,"W",0 },
{ KBDWIDTH,"e",0x0E,"E",0 },
{ KBDWIDTH,"r",0x0F,"R",0 },
{ KBDWIDTH,"t",0x11,"T",0 },
{ KBDWIDTH,"y",0x10,"Y",0 },
{ KBDWIDTH,"u",0x20,"U",0 },
{ KBDWIDTH,"i",0x22,"I",0 },
{ KBDWIDTH,"o",0x1F,"O",0 },
{ KBDWIDTH,"p",0x23,"P",0 },
{ KBDWIDTH,"[",0x21,"{",0 },
{ KBDWIDTH,"]",0x1E,"}",0 },
	{ KBDWIDTHTAB,"menu",-1,0,0 },
{ -1,0,0,0,0 },
{ KBDWIDTHCTRL,"control",0x36,0,0 },	
{ KBDWIDTH,"a",0x00,"A",0 },
{ KBDWIDTH,"s",0x01,"S",0 },
{ KBDWIDTH,"d",0x02,"D",0 },
{ KBDWIDTH,"f",0x03,"F",0 },
{ KBDWIDTH,"g",0x05,"G",0 },
{ KBDWIDTH,"h",0x04,"H",0 },
{ KBDWIDTH,"j",0x26,"J",0 },
{ KBDWIDTH,"k",0x28,"K",0 },
{ KBDWIDTH,"l",0x25,"L",0 },
{ KBDWIDTH,";",0x29,":",0 },
{ KBDWIDTH,"'",0x27,"\"",0 },
{ KBDWIDTHRET,"return",0x24,0,0 },
{ -1,0,0,0,0 },
	{ KBDWIDTHSHIFT,"shift",0x38,0,0 },	
	{ KBDWIDTH,"z",0x06,"Z",0 },
	{ KBDWIDTH,"x",0x07,"X",0 },
	{ KBDWIDTH,"c",0x08,"C",0 },
	{ KBDWIDTH,"v",0x09,"V",0 },
	{ KBDWIDTH,"b",0x0B,"B",0 },
	{ KBDWIDTH,"n",0x2D,"N",0 },
	{ KBDWIDTH,"m",0x2E,"M",0 },
	{ KBDWIDTH,",",0x2B,"<",0 },
	{ KBDWIDTH,".",0x2F,">",0 },
	{ KBDWIDTH,"/",0x2C,"?",0 },
	{ KBDWIDTHSHIFT,"shift",0x38,0,0 },
	{ -1,0,0,0,0 },
	{ KBDWIDTH,"caps",0x39,0,0 },	
	{ KBDWIDTH,"option",0x37,0,0 },	
	{ KBDWIDTHAPPLE,"",0x3A,0,0 },
	{ KBDWIDTH,"`",0x12,0,0 },
	{ KBDWIDTHSPACE," ",0x31,0,0 },
	{ KBDWIDTH,"x",0x13,0,0 },
	{ KBDWIDTH,"->",0x3C,0,0 },
	{ KBDWIDTH,"<-",0x3B,0,0 },
	{ KBDWIDTH,"^",0x5B,0,0 },
	{ KBDWIDTH,"v",0x13,0,0 },
	
	{ 0,0,0,0,0 }
	
};

static kbdselected* sharedKBDSelected = nil;

int findCode(const char* _s)
{
	int i =0;
	while(kbdDefinitions[i].w)
	{
		if (kbdDefinitions[i].k1 && !strcmp(kbdDefinitions[i].k1,_s))
			return kbdDefinitions[i].code1;
		if (kbdDefinitions[i].k2 && !strcmp(kbdDefinitions[i].k2,_s))
			return (kbdDefinitions[i].code1 + shiftKey);
		i++;
	}
	return -1;

}

@implementation kbdselected


- (void)drawRect:(CGRect)rect {
    // Drawing code
	CGContextRef g = UIGraphicsGetCurrentContext();
	CGContextSetFillColorWithColor(g, [UIColor whiteColor].CGColor);
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

- (void) setParam: (const char*)_v
{
	v = _v;
}

+ (kbdselected*)getSelected {
	return sharedKBDSelected;
}
@end


@implementation kbdlayout


- (id)initWithFrame:(CGRect)frame {

    if (self = [super initWithFrame:frame]) {
        // Initialization code
	
		int y = 1;
		int x = 0;
		int i = 0;
		int w;
		while( w = kbdDefinitions[i].w )
		{
			if (w==-1)
			{
				x=0;
				y+=KBDHEIGHT;
			}
			kbdDefinitions[i].posx = x;
			kbdDefinitions[i].posy = y;
			
			CGRect r = CGRectMake(x,y,w,KBDHEIGHT);
			x += w/*+1*/;
			keyboard * kbd = [[keyboard alloc] initWithFrame:r];
			[kbd setUserInteractionEnabled:FALSE];
			[kbd setParam:kbdDefinitions[i].k1];
			[self addSubview:kbd];
			[kbd release];
			i++;
		}
		
		CGRect r = CGRectMake(100,100,32.0,30.0);
		kbdselected * sel = [[kbdselected alloc] initWithFrame:r];
		[sel setUserInteractionEnabled:FALSE];
		sharedKBDSelected = sel;
		sel.hidden = true;
		iKbd = 0;
		[self addSubview:sel];
		[sel release];
		
		
    }
    return self;
}


- (void)drawRect:(CGRect)rect {
    // Drawing code
	CGContextRef g = UIGraphicsGetCurrentContext();
	CGContextSetFillColorWithColor(g, [UIColor whiteColor].CGColor);
	CGContextFillRect(g, rect);
}


- (void)dealloc {
    [super dealloc];
}

int findKey(int _x,int _y)
{
	int found=0;
	int posx,posy;
	// cherche le clavier le plus proche
	int i = 0;
	int w;
	while( w = kbdDefinitions[i].w )
	{
		if (w==-1)
			continue;
		posx = kbdDefinitions[i].posx ;
		posy = kbdDefinitions[i].posy ;
		if (_x>=posx && _x<(posx+w) && _y>=posy && _y<(posy+KBDHEIGHT))
		{
			found = i;
			break;
		}
		i++;
	}
	return found;
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint pt = [[touches anyObject] locationInView:self];
	
	int i = findKey(pt.x,pt.y);
	if (iKbd && i != iKbd)
		[self touchesEnded:touches withEvent:event];
	
	if (!i) 
	{
		iKbd = 0;
		return ;
	}
	
	if (i==iKbd)
		return ;
	iKbd= i;
	
	printf("Down : %s (%X)\n",kbdDefinitions[i].k1,kbdDefinitions[i].code1);
	if (kbdDefinitions[i].code1==-1)
	{
		
	//	activegsList* vl = nil;
	//	UIView* vl = [[[pManager getBrowserView]viewList]view];
		[pManager doStateChange:[pManager getBrowserView] sens:kCATransitionFromRight];
		return ;
	}
	add_event_key(kbdDefinitions[i].code1,0);
	
	int posx = kbdDefinitions[i].posx ;
	int posy = kbdDefinitions[i].posy ;
	
//	startLocation = pt;
	kbdselected* sel =[kbdselected getSelected];
	[[self superview] bringSubviewToFront:sel];
	CGRect frame = [self frame];
	CGRect framesel = [sel frame];

	framesel.origin.x =/* frame.origin.x + */posx,0;
	framesel.origin.y = /*frame.origin.y +*/ posy - 30.0;
	
	/*
	 framesel.origin.x = pt.x -10,0;
	 framesel.origin.y = pt.y - 50.0;
	 */
	framesel.size.width = kbdDefinitions[i].w;
//	framesel.size.height = frame.size.height*2.0;
	[sel setFrame:framesel];
	
	[sel setParam:kbdDefinitions[i].k1];
	[sel setNeedsDisplay];
	sel.hidden=false;
	
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{

	kbdselected* sel =[kbdselected getSelected];
	sel.hidden=true;
	if (iKbd)
	{
			add_event_key(kbdDefinitions[iKbd].code1,1);
		printf("Up : %s (%x)\n",kbdDefinitions[iKbd].k1,kbdDefinitions[iKbd].code1);
	}
	iKbd= 0;
}



- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchesBegan:touches withEvent:event];
	
	
	
}


@end
