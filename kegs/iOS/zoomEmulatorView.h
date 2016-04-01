/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#import "emulatorView.h"

#if defined(VIDEO_DOUBLEVLINE)
#   define HEIGHT_MULTIPLY 1
#else
#   define HEIGHT_MULTIPLY 2
#endif

#if defined(VIDEO_DOUBLEHLINE)
#   define WIDTH_MULTIPLY 1
#else
#   define WIDTH_MULTIPLY 2
#endif

 struct orientationVar
{
	float scale;
	CGPoint p;
} ;

typedef struct orientationVar orientationVar;

@interface zoomEmulatorView : UIScrollView<UIScrollViewDelegate>
{

    float dpiRatio;
    CGSize viewSize;
	

    
	int mouseDown;
	int zoomDisabled;
	orientationVar orientationVars[4];

	float kbdScale;

	UIPanGestureRecognizer* panGR;
	UIPinchGestureRecognizer* pinchGR;
	CGPoint lastMousePos;
	BOOL allowInput;
	BOOL startNewPhase;
@public
	UIImageView* _crt ;
    emulatorView * _ew ;
	UITouch* _useTouch;
	UITouch* _secondTouch;
    	int curOrientation;
}
@property (strong,nonatomic) emulatorView* ew ;
@property (strong,nonatomic) UIImageView* crt ;
@property (strong,nonatomic) UITouch* useTouch ;
@property (strong,nonatomic) UITouch* secondTouch ;

-(void)updateFrame:(CGRect)frame kbdRect:(CGRect)kbdRect;
-(void)updateScreen:(CGRect)frame ratio:(float)ratio;
- (void)switchOrientation:(int)orientation;
-(void)disableZoom;
-(void)enableZoom;
-(void)enableInput;
-(void)disableInput;
-(void)mouseUp;
-(void)mouseDown;
-(void)setRatioToOne;
-(CGPoint)rotateTouch:(UITouch*) _touch;
- (id)initWithFrame:(CGRect)frame  ratio:(float)ratio;
@end
