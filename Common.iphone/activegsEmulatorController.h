/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#import "../kegs/iOS/zoomEmulatorView.h"
#import "../kegs/iOS/emulatorView.h"
#import "KBDController.h"

enum gestureModes
{
	
	MODE_PAD = 1,
	MODE_ZOOM = 2,
	MODE_SWIPE= 4,
	MODE_ALL = MODE_PAD+MODE_ZOOM+MODE_SWIPE,
	MODE_MOUSEBUTTON=8,
	MODE_EMULATOR=16,
};

@interface customView : UIView
{
	double possibleSwipeLeftTime ;
	double possibleSwipeRightTime ;
	double possibleSwipeUpTime ;
	double possibleSwipeDownTime ;
	
	UISwipeGestureRecognizer* _grswipeup;
	UISwipeGestureRecognizer* _grswipedown;
	UISwipeGestureRecognizer* _grswipeleft;
	UISwipeGestureRecognizer* _grswiperight;
	NSTimer* enableTimer;
	NSTimer* disableTimer;
}

@property (strong,nonatomic) UISwipeGestureRecognizer* grswipeup;
@property (strong,nonatomic) UISwipeGestureRecognizer* grswipedown;
@property (strong,nonatomic) UISwipeGestureRecognizer* grswipeleft;
@property (strong,nonatomic) UISwipeGestureRecognizer* grswiperight;

-(void)disableGestures:(int)_mode  ;
-(void)enableGestures:(int)_mode  ;
-(void)disableAllGestures;
-(void)reenableAllGestures ;
-(void)processGesturesOnTouchesEnded ;
-(void)scheduleDisableAllGestures;
-(void)scheduleEnableAllGestures;
-(void)invalidateTimers;

@end

enum attachMode
{
    ATTACH_NONE,
    ATTACH_PRIMARY,
    ATTACH_SECONDARY
};

@interface activegsEmulatorController : UIVIEWCONTROLLERROOT
{
@public
    int    attachedTo;
}

#ifdef USE_CAPPTAIN
-(NSString*)getCapptainActivityName;
#endif

-(void) updateView;

@property	(nonatomic, strong) customView* contentView;
@property	(nonatomic, strong) zoomEmulatorView* zv;
@property	(nonatomic, strong) KBDController*		kbdc ;
@property	(nonatomic, strong) NSString*		trackerName ;


@end

