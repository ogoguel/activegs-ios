/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>

struct kbdDefinition
{
	int w;
	const char* k1;
	int code1;
	const char* k2;
	int code2;
	int posx,posy;
};

extern struct kbdDefinition kbdDefinitions[];

#define KBDHEIGHT 20

@interface keyboard : UIView {
	CGPoint startLocation;
	const char* v;
}
- (void)setParam:(const char*) _v;

@end
