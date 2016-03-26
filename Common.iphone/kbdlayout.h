/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>


@interface kbdlayout : UIView 
{
	int iKbd;
}

@end


@interface kbdselected : UIView 
{
	const char* v;
	int time;
//	int iKbd;
}
//@property(nonatomic,getter=getikbd) int              iKbd;     

- (void) setParam: (const char*)_v;
+ (kbdselected*) getSelected;
@end
