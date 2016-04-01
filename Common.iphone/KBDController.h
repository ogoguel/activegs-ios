/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>

enum
{
    LOCKZOOM_AUTO=0,
    LOCKZOOM_ON=1,
    LOCKZOOM_ARCADE=2
};

@interface KBDController : UIViewController <UITextFieldDelegate> 
{

    CGRect keyboardDelta;
	float  keyboardScale;
	BOOL	bMenuBarVisibility;
	BOOL	bLoading;
	BOOL	padEnabled;
	CGPoint posPad;
	float diskAngle ;
    
    
    
	int	inputMode;

    UITextField*    _textField;
	UIButton*       _menuButton;
	UIBarButtonItem*    _optionButton ;
	UIImageView*    _diskView;
    UIImageView*    _diskLoadingView;
    NSMutableArray*        _diskIcons;
	UILabel*        _loaderLabel;
	UILabel*        _debugIndicator;
	UILabel*        _swipeIndicator;
	UILabel*        _zoomIndicator;
	UILabel*        _padIndicator;
	UILabel*        _mouseButtonIndicator;
	UILabel*        _inputIndicator;
	
	UIActivityIndicatorView* _loader;
	NSTimer*        _loaderTimer;
#ifdef GRAPHICAL_PAD
	UIImageView*    _padBackground;
	UIImageView*    _padButton1Down;
	UIImageView*    _padButton2Down;
	UIImageView*    _padStick;
#endif
	UIImageView*    _padCircleOutter;
	UIImageView*    _padCircleInner;
	UIImageView*    _oaButtonView;
	UIImageView*    _caButtonView;
	UITouch*        _padTouch;
	UITouch*        _oaButtonTouch;
	UITouch*        _caButtonTouch;
	UINavigationItem* _emulatorNavItem;
	UINavigationBar* _runtimeView;
	UIView*         _interfaceView;
    NSTimer*        _hardwarekbdDetectionTimer;
    NSTimer*        _animateDiskTimer;
	
    UIImageView*    _runtimeControls;
	UIView*         _runtimeControlsOptions;
    
    UIImageView*    _diskSelection;
	UIView*         _diskSelectionOptions;
	UIScrollView*   _uit;
	UIScrollView*   _uitb;
	UIView*         _accessView;
	UIButton*       _leftAccessView;
	UIButton*       _rightAccessView;
	UINavigationBar* _barView;
	UINavigationBar* _specialView;
	
	
}

@property (nonatomic,strong)    UITextField* textField;

@property (nonatomic,strong)    UIButton *menuButton;
@property (nonatomic,strong)    UIBarButtonItem* optionButton ;
@property (nonatomic,strong)    UIImageView* diskView;
@property (nonatomic,strong)    UIImageView* diskLoadingView;
@property (nonatomic,strong)    NSMutableArray* diskIcons;
@property (nonatomic,strong)    UILabel * loaderLabel;
@property (nonatomic,strong)    UILabel* debugIndicator;
@property (nonatomic,strong)    UILabel* swipeIndicator;
@property (nonatomic,strong)    UILabel* zoomIndicator;
@property (nonatomic,strong)    UILabel* padIndicator;
@property (nonatomic,strong)    UILabel* mouseButtonIndicator;
@property (nonatomic,strong)    UILabel* inputIndicator;
@property (nonatomic,strong)    UIActivityIndicatorView* loader;
@property (nonatomic,strong)    NSTimer* loaderTimer;
#ifdef GRAPHICAL_PAD
@property (nonatomic,retain)    UIImageView* padBackground;
@property (nonatomic,retain)    UIImageView* padButton1Down;
@property (nonatomic,retain)    UIImageView* padButton2Down;
@property (nonatomic,retain)    UIImageView* padStick;
#endif
@property (nonatomic,strong)    UIImageView* padCircleOutter;
@property (nonatomic,strong)    UIImageView* padCircleInner;
@property (nonatomic,strong)    UIImageView* oaButtonView;
@property (nonatomic,strong)    UIImageView* caButtonView;
@property (nonatomic,strong)    UITouch* padTouch;
@property (nonatomic,strong)    UITouch* oaButtonTouch;
@property (nonatomic,strong)    UITouch* caButtonTouch;
@property (nonatomic,strong)    UINavigationItem* emulatorNavItem;
@property (nonatomic,strong)    UINavigationBar* runtimeView;
@property (nonatomic,strong)    UIView* interfaceView;
@property (nonatomic,strong)    NSTimer* animateDiskTimer;
@property (nonatomic,strong)    NSTimer* hardwarekbdDetectionTimer;
@property (nonatomic,strong)    UIImageView* runtimeControls;
@property (nonatomic,strong)    UIView* runtimeControlsOptions;
@property (nonatomic,strong)    UIImageView* diskSelection;
@property (nonatomic,strong)    UIView* diskSelectionOptions;
@property (nonatomic,strong)    UIScrollView* uit;
@property (nonatomic,strong)    UIScrollView* uitb;
@property (nonatomic,strong)    UIView* accessView;
@property (nonatomic,strong)    UIButton* leftAccessView;
@property (nonatomic,strong)    UIButton* rightAccessView;
@property (nonatomic,strong)    UINavigationBar* barView;
@property (nonatomic,strong)    UINavigationBar* specialView;
//@property (assign,getter=inputMode)          int	inputMode;


-(void)updateSpecialButtonView:(UIButton*)_b state:(int)_state;
-(void)addSpecialKeys;
-(void)addRuntimeControls;
-(void)resetSpecialKeys;
-(void)unloadSpecialKeys;
- (void) animateDisk:(id)_id ;
-(void)refreshControls:(id)sender;
- (void) detectHardwareKeyboard:(id)_id ;
- (void)setLoading:(NSNumber*)_nsvisible;
-(void)setiCadeMode:(BOOL)_mode;
-(void)OnScreenKeyboard:(BOOL)_enabled;
-(void)enableKeyboard:(int)_enable;
-(void)restoreInput;

//#define INPUTMODE_NONE 0
#define INPUTMODE_PAD 1
#define INPUTMODE_KBD 2
#define INPUTMODE_ACCESS 4
#define INPUTMODE_MINIACCESS 8
#define INPUTMODE_ARCADE 16 
#define INPUTMODE_CUSTOMKEYS 32
#define INPUTMODE_KBDFORCED 64

#define INPUTMODE_ALL (INPUTMODE_KBD + INPUTMODE_PAD + INPUTMODE_ACCESS)
#define INPUTMODE_HIDDEN 128


-(int)isInputVisible:(int) _im;
-(void)hideInput;
-(void)cycleInput;
-(void) updateSpecialKey:(CGRect)rv3;
- (void)setInputMode:(int)_mode;
- (void)setMenuBarVisibility:(int)_visible;
//- (void)toggleMenuBarVisibility ;
- (void)setEmulatorTitle:(const char*)_name ;
- (void)swapButton:(id)sender;
- (void)updateView:(CGRect)_rect;
- (void)swipeUp;
- (void)swipeDown;
-(void)setGestureIndicatorVisibility:(int)_mode color:(UIColor*)_color;
-(void)setDebugIndicatorText:(const char*)_text;
-(void)showDebug:(BOOL)_visible;
- (void)hideLoader:(id)_id;
-(void)updatePadView;
-(void)centerPad:(id)center;

-(void)enablePad:(BOOL)_enable;
-(UIImageView*)createCircle:(float)_radius red:(float)_red green:(float)_green blue:(float)_blue alpha:(float)_alpha;
- (BOOL) myTouchesBegan:(NSSet *)touches ;
- (BOOL) myTouchesMoved:(NSSet *)touches ;
- (BOOL) myTouchesEnded:(NSSet *)touches ;
-(CGFloat) getEmulatorScale;
-(CGRect) getEmulatorDelta;
- (void)updateViewForKeyboard:(int)_height;

 @end


