/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>



enum enum_key_type
{
    KEYTYPE_KEYBOARD=66,
    KEYTYPE_BUTTON
    
};

enum custom_align
{
    LEFT_ALIGN = 0x100,
    RIGHT_ALIGN =0x200,
    SAME_LINE = 0x400,
    SAME_COL = 0x800,
    TOP_ALIGN = 0x1000,
    BOTTOM_ALIGN = 0x2000,
    BOTTOM_BORDER=0x4000,
    LEFT_BORDER=0x8000,
    RIGHT_BORDER =0x10000,
    VERTICAL_CENTER = 0x20000,
    LEFTCENTER_ALIGN = 0x40000,
    MIDDLE_BOTTOM = 0x80000
    
    
};

struct key_type
{
    enum enum_key_type type;
    NSString* str;
    int code;
   
    int status;
    int halign;
    int valign;
    int mode;
    int  vfactor;
    int hfactor;
    UILabel* view;
    UIColor* bgcolor_off;
    UIColor* bgcolor_on;
    UIColor* bgcolor_over;
    key_type()
    {
        mode = 0;
        bgcolor_on = [UIColor colorWithRed:0.5f green:0.5f blue:0.5f alpha:0.5f];
        bgcolor_off = [UIColor colorWithRed:0.5f green:0.5f blue:0.5f alpha:0.0f];
        bgcolor_over = [UIColor colorWithRed:0.8f green:0.8f blue:0.8f alpha:0.5f];
        vfactor=1;
        hfactor=1;
    };
}  ;

#define MAX_CUSTOM_KEYS 32
extern key_type custom_keys[MAX_CUSTOM_KEYS];
extern void show_custom_keys(CGRect _rv, int _mode,int _centery);
//extern void hide_custom_keys();
extern void fade_custom_keys(float _alpha);
extern void init_custom_keys();

extern void touch_begin_custom_key(CGPoint p,int nbtap);
extern void touch_end_custom_key();
extern void toggle_custom_keys(int _mode, int _hide);

extern float CUSTOM_KEY_WIDTH ;
extern float CUSTOM_KEY_HEIGHT ;

@interface UICustomKey : UIView

@end
