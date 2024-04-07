/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "KBDController.h"
#include "../kegs/Src/defc.h"
#include "../kegs/Src/protos_macdriver.h"
#include "../kegs/Src/sim65816.h"
#include "../kegs/Src/moremem.h"
#include "../kegs/Src/adb.h"
#include "../kegs/Src/sound.h"
#include "../kegs/Src/async_event.h"
#include "../Common/svnversion.h"
#import "activegsAppDelegate.h"
#import "activegsViewController.h"
#import <UIKit/UINavigationBar.h>
#import <ExternalAccessory/ExternalAccessory.h>
#include "../kegs/Src/paddles.h"
#import "../kegs/iOS/emulatorView.h"
#include "../kegs/Src/SaveState.h"
#include "../Common/ActiveDownload.h"
#import "MfiGameControllerHandler.h"

#ifdef ACTIVEGS_CUSTOMKEYS
    #include "UICustomKey.h"
#endif

#ifndef ACTIVEGS_NOHARDWAREKEYBOARDETECTION
extern "C" void GSEventSetHardwareKeyboardAttached(Boolean);
extern "C" void UIKeyboardOrderInAutomatic();
extern "C" void UIKeyboardOrderOutAutomatic();
extern "C" BOOL UIKeyboardAutomaticIsOnScreen();
#endif

#undef debug_printf
#define debug_printf printf

float joyX=0.0f,joyY=0.0f;
int oaButton=FALSE,caButton=FALSE;
int padEnabled=FALSE;
int externalKeyboard=FALSE; 
int iCadeDetected=FALSE;
int bForceOnScreenKeyboard = FALSE;

int lastArrowKey = 0;


enum runtimeKeysAction
{
//	RT_PAUSE,
	RT_COLOR,
	RT_FX,
	RT_SPEED,
	RT_CAPTURE,
	RT_RESTORE,
	RT_ZOOM
};
	

typedef struct {
	const char* label;
	enum runtimeKeysAction   action;
	UIButton* hButton;
} rkey;

rkey runtimeKeysList[]=
{
//	{ "Pause", RT_PAUSE, nil },
	{ "Color", RT_COLOR, nil },
	{ "F/X", RT_FX, nil },	
	{ "Speed", RT_SPEED, nil },
	{ "Save", RT_CAPTURE, nil },
	{ "Restore", RT_RESTORE, nil },
//	{ "Zoom", RT_ZOOM, nil }
};
	

#define NBBUTTONLOOP 2

typedef struct {
	const char* label;
	int code;
	int modifier;
	int state;
	int shiftxxx;
	UIButton* hButton[NBBUTTONLOOP];
} skey;


#define UITB_BUTTON_WIDTH 20
#define UITB_BUTTON_PADDING 4
#define ACCESS_HEIGHT 36

skey buttonList[] = { 
	
	{ "SHIFT",0x38,0,0,0,nil},
	{ "ESC", 0x35 , 0, -1, 0,nil},
	{ "TAB",0x30,0,-1,0,nil }, 
	{ "CTRL",0x36,0,0,0,nil },
	{ "",0x37,0,0,0,nil},
	{ "OPT",0x3A,0,0,0,nil},
	{ "RESET",0x7F,0,-1,0,nil},
	
	{ "←", 0x3B,0, -1,0,nil},
	{ "→",0x3C,0,-1,0,nil},
	{ "↑",0x3E,0,-1,0,nil},
	{ "↓",0x3D,0,-1,0,nil},
	

	{ "J",0x26+shiftKey,0,-1,1,nil},
	{ "K",0x28+shiftKey,0,-1,1,nil},	
	{ "SPC",0x31,0,-1,0,nil},
	{ "RET",0x24,0,-1,0,nil},
    { "1",0x12,0,-1,0,nil},
	{ "2",0x13,0,-1,0,nil},
    { "3",0x14,0,-1,0,nil},
	{ "4",0x15,0,-1,0,nil},
	{ NULL,0,0,0 } }; 

#define KBDWIDTH 22
#define KBDWIDTHTAB 25
#define KBDWIDTHCTRL 30
#define KBDWIDTHRET 30
#define KBDWIDTHSHIFT 42
#define KBDWIDTHAPPLE 30
#define KBDWIDTHSPACE 100


struct kbdDefinition
{
	int w;
	const char* k1;
	int code1;
	const char* k2;
	int code2;
	int posx,posy;
};

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


#define INNER_SIZE 64
#define OUTTER_SIZE 128
#define BUTTON_SIZE 80
#define BUTTON_PADDING 32

static float innerSize,outterSize,buttonSize;
static const char* disk_names[] = { "disk_35_1", "disk_35_2", "disk_525_1","disk_525_2" , "disk" };


UIImageView* loadImage(NSString* _name)
{
	
	NSString *imgSource = [[NSBundle mainBundle] pathForResource:_name ofType:@"png"];
	UIImage* img = [UIImage imageWithContentsOfFile: imgSource];
		UIImageView* view = [[UIImageView alloc] initWithImage:img];
	return view;
	
}


int isHardwareKeyboard()
{
	

    if (bForceOnScreenKeyboard)
        return 1;

#ifdef ACTIVEGS_NOHARDWAREKEYBOARDDETECTION
	// ppour la soumission appstore!
	return option.getIntValue(OPTION_EXTERNALKBD);
#else
	int isHardware=0;
	@try
	{
		Class UIKeyboardImpl = NSClassFromString(@"UIKeyboardImpl");
		id activeInstance = [UIKeyboardImpl performSelector:@selector(sharedInstance)];
		id v = [activeInstance valueForKey:@"isInHardwareKeyboardMode"];
		isHardware  = [(NSNumber*)v intValue];
	}
	@catch (NSException *exception)
	{
		NSLog(@"Exception %@", exception);
	}
	
	 return isHardware;
#endif

}

@interface KBDController() {
    UISegmentedControl *saveStateSegmentedControl;
}
@property (nonatomic,strong) MfiGameControllerHandler *mfiControllerHandler;
@end

@implementation KBDController

@synthesize textField = _textField;
//@synthesize inputMode = _inputMode;
@synthesize menuButton = _menuButton;
@synthesize optionButton = _optionButton;
@synthesize diskView  = _diskView;
@synthesize diskLoadingView = _diskLoadingView;
@synthesize diskIcons = _diskIcons;
@synthesize loaderLabel = _loaderLabel;
@synthesize debugIndicator = _debugIndicator;
@synthesize swipeIndicator = _swipeIndicator;
@synthesize zoomIndicator = _zoomIndicator;
@synthesize padIndicator = _padIndicator;
@synthesize mouseButtonIndicator = _mouseButtonIndicator;
@synthesize inputIndicator = _inputIndicator;
@synthesize loader = _loader;
@synthesize loaderTimer = _loaderTimer;
#ifdef GRAPHICAL_PAD
@synthesize  padBackground = _padBackground;
@synthesize  padButton1Down = _padButton1Down;
@synthesize  padButton2Down = _padButton2Down;
@synthesize  padStick = _padStick;
#endif
@synthesize padCircleOutter = _padCircleOutter;
@synthesize padCircleInner = _padCircleInner;
@synthesize oaButtonView = _oaButtonView;
@synthesize caButtonView = _caButtonView;
@synthesize padTouch = _padTouch;
@synthesize oaButtonTouch = _oaButtonTouch;
@synthesize caButtonTouch = _caButtonTouch;
@synthesize emulatorNavItem = _emulatorNavItem;
@synthesize runtimeView = _runtimeView;
@synthesize interfaceView = _interfaceView;
@synthesize hardwarekbdDetectionTimer = _hardwarekbdDetectionTimer;
@synthesize animateDiskTimer = _animateDiskTimer;
@synthesize runtimeControls = _runtimeControls;
@synthesize runtimeControlsOptions = _runtimeControlsOptions;
@synthesize diskSelection = _diskSelection;
@synthesize diskSelectionOptions = _diskSelectionOptions;
@synthesize uit = _uit;
@synthesize uitb = uitb;
@synthesize accessView = _accessView;
@synthesize leftAccessView = _leftAccessView;
@synthesize rightAccessView = _rightAccessView;
@synthesize barView = _barView;
@synthesize specialView = _specialView;

extern int findCode(const char* _s);


- (void)loadView
{
    
     NSLog(@"---kbd loadView");
	
	CGRect r = [UIScreen mainScreen].applicationFrame;
	self.interfaceView = [ [UIView alloc] initWithFrame:r] ;
 
	[self.interfaceView setUserInteractionEnabled:TRUE];
	self.interfaceView.multipleTouchEnabled = 1;

	float s = [pManager resolutionRatio];
	
	self.barView = [[UINavigationBar alloc] initWithFrame:CGRectMake(0, 0, r.size.width, BARVIEW_HEIGHT)];	
	self.barView.barStyle = UIBarStyleBlack;
	self.barView.translucent = TRUE;
#ifdef ACTIVEGS_NOAPPLEBRANDING
	self.emulatorNavItem = [[UINavigationItem alloc] initWithTitle:@"ActiveGS"];
#else
	self.emulatorNavItem = [[UINavigationItem alloc] initWithTitle:@"Apple IIGS™"];
#endif

	[self.barView pushNavigationItem:self.emulatorNavItem animated:FALSE];
	
	// left Arrow
	UIButton* backButton = [UIButton buttonWithType:(UIButtonType)101]; // left-pointing shape!
	[backButton addTarget:self action:@selector(menuButton:) forControlEvents:UIControlEventTouchUpInside];
	[backButton setTitle:@"Back" forState:UIControlStateNormal];
	
	// create button item -- possible because UIButton subclasses UIView!
	UIBarButtonItem* browseItem = [[UIBarButtonItem alloc] initWithCustomView:backButton];
	
	self.emulatorNavItem.leftBarButtonItem = browseItem;

	self.optionButton = [[UIBarButtonItem alloc] initWithTitle:@"Options" style:UIBarButtonItemStylePlain
															   target:self
									action:@selector(optionsButton:)];
	
#if !defined(BESTOFFTA)
	self.emulatorNavItem.rightBarButtonItem = self.optionButton;
#endif
	
	[self.interfaceView addSubview:self.barView];
	
	
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(keyboardWillShow:) name: UIKeyboardWillShowNotification object:nil];
	[nc addObserver:self selector:@selector(keyboardWillHide:) name: UIKeyboardWillHideNotification object:nil];
	[nc addObserver:self selector:@selector(keyboardDidShow:) name: UIKeyboardDidShowNotification object:nil];
	[nc addObserver:self selector:@selector(keyboardDidHide:) name: UIKeyboardDidHideNotification object:nil];
	
	
    // charge toutes les icones disks
	
    self.diskIcons = [[NSMutableArray alloc] init];
        
    for(int i=0;i<5;i++)
    {
	
        NSString *imgSource = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:disk_names[i]] ofType:@"png"];
        NSLog(@"loading %@",imgSource);
        [self.diskIcons addObject:[UIImage imageWithContentsOfFile: imgSource]];
    }

         CGSize ds = [[self.diskIcons objectAtIndex:4] size];
	ds.width *=s;
	ds.height*=s;
	self.diskView = [[UIImageView alloc] initWithFrame:CGRectMake(0,BARVIEW_HEIGHT+4*s,ds.width,ds.height)];
	self.diskView.image = [self.diskIcons objectAtIndex:4];
	self.diskView.hidden = TRUE;
	self.diskView.alpha=0.8;

	
    self.diskLoadingView = [[UIImageView alloc] initWithFrame:CGRectMake(0,BARVIEW_HEIGHT+4*s,ds.width,ds.height)];
	self.diskLoadingView.image = [self.diskIcons objectAtIndex:0];
	self.diskLoadingView.hidden = FALSE;
	self.diskLoadingView.alpha=0.0; // hidden by default

    
	[self.interfaceView addSubview:self.diskView];
	[self.interfaceView addSubview:self.diskLoadingView];
	
#define WHEELSIZE 50
	self.loader = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
	CGRect lr = self.loader.frame;
	lr.origin.x = 8;
	lr.origin.y = 32;
	[self.loader setFrame:lr];
	[self.interfaceView addSubview:self.loader];
	[self.loader stopAnimating];
	self.loader.hidden = TRUE;


	self.swipeIndicator = [[UILabel alloc] initWithFrame:CGRectMake(8,64,16,16)];
	self.swipeIndicator.backgroundColor = [UIColor redColor];
	self.swipeIndicator.text = @"S";
	[self.interfaceView addSubview:self.swipeIndicator];

	self.zoomIndicator = [[UILabel alloc] initWithFrame:CGRectMake(8+20,64,16,16)];
	self.zoomIndicator.backgroundColor = [UIColor redColor];
	self.zoomIndicator.text = @"Z";
	[self.interfaceView addSubview:self.zoomIndicator];
		
	self.padIndicator = [[UILabel alloc] initWithFrame:CGRectMake(8+40,64,16,16)];
	self.padIndicator.backgroundColor = [UIColor redColor];
	self.padIndicator.text = @"P";
	[self.interfaceView addSubview:self.padIndicator];
	
	self.mouseButtonIndicator = [[UILabel alloc] initWithFrame:CGRectMake(8+60,64,16,16)];
	self.mouseButtonIndicator.backgroundColor = [UIColor greenColor];
	self.mouseButtonIndicator.text = @"B";
	[self.interfaceView addSubview:self.mouseButtonIndicator];
	

	self.inputIndicator = [[UILabel alloc] initWithFrame:CGRectMake(8+80,64,16,16)];
	self.inputIndicator.backgroundColor = [UIColor greenColor];
	self.inputIndicator.text = @"I";
	[self.interfaceView addSubview:self.inputIndicator];
	
	self.debugIndicator = [[UILabel alloc] initWithFrame:CGRectMake(8+120,64,150,16)];
	self.debugIndicator.hidden = TRUE;
	self.debugIndicator.backgroundColor = [UIColor lightGrayColor];
	self.debugIndicator.font = [UIFont systemFontOfSize:(CGFloat)12.0];
	self.debugIndicator.lineBreakMode=UILineBreakModeClip;
	[self.interfaceView addSubview:self.debugIndicator];
	
	[self  showDebug:FALSE];
	
	CGRect lrl = CGRectMake(lr.origin.x,lr.origin.y+lr.size.height,50,10);
	
	self.loaderLabel = [[UILabel alloc] initWithFrame:lrl];
	self.loaderLabel.text=@"100pc";
	[self.interfaceView addSubview:self.loaderLabel];
	self.loaderLabel.hidden = TRUE;
	

	self.textField = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
	self.textField.delegate = self;
	self.textField.autocapitalizationType= UITextAutocorrectionTypeNo;
	self.textField.text= @"*";	// Put a default value to capture del key
	[self.interfaceView addSubview:self.textField];
	
	self.view = self.interfaceView;
	
	keyboardScale = 1.0;
	 
	
	float rr = [pManager resolutionRatio];
	innerSize = INNER_SIZE * rr;
	outterSize = OUTTER_SIZE * rr;
	buttonSize = BUTTON_SIZE * rr;
	
	self.padCircleOutter = [self createCircle:outterSize red:1.0 green:1.0 blue:1.0 alpha:0.3];
	self.padCircleOutter.frame = CGRectMake(0,r.size.height-outterSize,outterSize,outterSize);
	[self.interfaceView addSubview:self.padCircleOutter];	
	
	self.padCircleInner = [self createCircle:innerSize red:0.8 green:0.8 blue:0.8 alpha:1.0];
	
	[self.padCircleOutter addSubview:self.padCircleInner];	
	
	self.oaButtonView = [self createCircle:buttonSize red:1.0 green:1.0 blue:1.0 alpha:1.0];
	self.oaButtonView.frame = CGRectMake(r.size.width-buttonSize,r.size.height-buttonSize*2,buttonSize,buttonSize);
	[self.interfaceView addSubview:self.oaButtonView];	
	
	self.caButtonView = [self createCircle:buttonSize red:1.0 green:1.0 blue:1.0 alpha:1.0];
	self.caButtonView.frame = CGRectMake(r.size.width-buttonSize,r.size.height-buttonSize,buttonSize,buttonSize);
	[self.interfaceView addSubview:self.caButtonView];
	
#ifdef GRAPHICAL_PAD
	self.padBackground = loadImage(@"pad_up");
	
	// resize la view
	CGSize rgpad = self.padBackground.image.size;
	float gratio = 320.0 / rgpad.width ; 
	float hpad = gratio * rgpad.height ; 
	self.padBackground.frame = CGRectMake(0,480-hpad,320,hpad);
	
	self.padButton1Down = loadImage(@"pad_button1_down"); 
	
	// 42,28
	
	// resize la view
	rgpad = self.padButton1Down.image.size;
	self.padButton1Down.frame = CGRectMake(42/2,480-hpad+28/2,gratio * rgpad.width,gratio * rgpad.height);
	
	self.padButton2Down = loadImage(@"pad_button2_down"); 

	// 42,226
	
	// resize la view
	rgpad = self.padButton2Down.image.size;
	self.padButton2Down.frame = CGRectMake(42/2,480-hpad+226/2,gratio * rgpad.width,gratio * rgpad.height);
	
	self.padStick = loadImage(@"pad_stick"); 
	
	// 42,226
	
	// resize la view
	rgpad = self.padStick.image.size;
	self.padStick.frame = CGRectMake(200,480-hpad+200/2,gratio * rgpad.width,gratio * rgpad.height);
	
	
	[self.interfaceView addSubview:self.padBackground];
	[self.interfaceView addSubview:self.padButton1Down];
	[self.interfaceView addSubview:self.padButton2Down];
	[self.interfaceView addSubview:self.padStick];	
	

#endif
	
	[self addRuntimeControls];
    [self addDiskSelection];
	
	[self addSpecialKeys];

#if 0 
	[self addRuntimeKeys];
#endif

#ifdef ACTIVEGS_CUSTOMKEYS
    init_custom_keys();
#endif
    
	iCadeDetected = FALSE;
    externalKeyboard = FALSE;

    [self detectHardwareKeyboard:nil];

    // If iCade explicitly selected in options, force enable it
    if ( option.getIntValue(OPTION_JOYSTICKMODE) == JOYSTICK_TYPE_ICADE ) {
        [self setiCadeMode:YES];
    }

    [self setInputMode:INPUTMODE_ACCESS+INPUTMODE_HIDDEN];
	[self setMenuBarVisibility:TRUE]; // So First time users are not lost!
    
#if !defined(ACTIVEGS_NOMFISUPPORT)
    self.mfiControllerHandler = [[MfiGameControllerHandler alloc] init];
    
    [self.mfiControllerHandler discoverController:^(GCController *gameController) {
        [self setupMfiController:gameController];
        [self setInputMode:self->inputMode&INPUTMODE_PAD];
        [pManager setNotificationText:@"mFi Controller Connected"];
    } disconnectedCallback:^{
        [pManager setNotificationText:@"mFi Controller Disconnected"];
    }];
#endif
    
   
    }
- (NSArray*)keyCommands
{
    if (lastArrowKey != 0)
    {
        add_event_key(lastArrowKey, 1);
        lastArrowKey = 0;
    }
    UIKeyCommand *upArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputUpArrow
                                                modifierFlags:0
                                                       action:@selector(upArrow:)];
    UIKeyCommand *downArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputDownArrow
                                                modifierFlags:0
                                                       action:@selector(downArrow:)];
    UIKeyCommand *leftArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputLeftArrow
                                                modifierFlags:0
                                                       action:@selector(leftArrow:)];
    UIKeyCommand *rightArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputRightArrow
                                                modifierFlags:0
                                                       action:@selector(rightArrow:)];
    return @[downArrow, upArrow, leftArrow, rightArrow];
}

- (void) upArrow: (UIKeyCommand *) keyCommand
{
    lastArrowKey =0x3e;
    add_event_key(0x3e, 0);
}

- (void) downArrow: (UIKeyCommand *) keyCommand
{
    lastArrowKey =0x3d;
    add_event_key(0x3d, 0);
}

- (void) leftArrow: (UIKeyCommand *) keyCommand
{
    lastArrowKey =0x3b;
    add_event_key(0x3b, 0);
}

- (void) rightArrow: (UIKeyCommand *) keyCommand
{
    lastArrowKey =0x3c;
    add_event_key(0x3c, 0);
}
            
-(void) setupMfiController:(GCController*)controller {
    void (^appleJoyButton0Handler)(GCControllerButtonInput *, float, BOOL) = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
        if ( pressed ) {
            add_event_key(0x37, 0);
        } else {
            add_event_key(0x37, 1);
        }
    };
    void (^appleJoyButton1Handler)(GCControllerButtonInput *, float, BOOL) = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
        if ( pressed ) {
            add_event_key(0x3a, 0);
        } else {
            add_event_key(0x3a, 1);
        }
    };
    void (^appleJoystickhHandler)(GCControllerDirectionPad *, float, float) = ^(GCControllerDirectionPad *, float xvalue, float yvalue) {
        joyX = xvalue;
        joyY = yvalue * -1.0;
    };
    GCControllerButtonInput *buttonX = controller.extendedGamepad ? controller.extendedGamepad.buttonX : controller.gamepad.buttonX;
    GCControllerButtonInput *buttonA = controller.extendedGamepad ? controller.extendedGamepad.buttonA : controller.gamepad.buttonA;
    GCControllerDirectionPad *dpad = controller.extendedGamepad ? controller.extendedGamepad.dpad : controller.gamepad.dpad;
    
    buttonX.valueChangedHandler = appleJoyButton0Handler;
    buttonA.valueChangedHandler = appleJoyButton1Handler;
    dpad.valueChangedHandler = appleJoystickhHandler;
    
    if ( controller.extendedGamepad ) {
        controller.extendedGamepad.leftThumbstick.valueChangedHandler = appleJoystickhHandler;
    }
}

int hardwarekeyboard= 0;

-(void)detectHardwareKeyboard:(id)_obj
{
  
    if (bForceOnScreenKeyboard)
        return ;
    
    int d = isHardwareKeyboard();
    if (d == hardwarekeyboard) return ;
    hardwarekeyboard = d;
    if ( d )
    {
        [pManager setNotificationText:@"External keyboard enabled"];
        [self enableKeyboard:TRUE];
    }
    else
    {
        [pManager setNotificationText:@"External keyboard disabled"];

        [self setiCadeMode:FALSE];
    }
    
	
}


-(void)setiCadeMode:(BOOL)_mode
{
    if (iCadeDetected == _mode)
        return ;
    
    iCadeDetected = _mode;
    NSLog(@"setting iCadeMode to %d",_mode);
    
    if (_mode==TRUE)
    {
        option.setIntValue(OPTION_JOYSTICKMODE,JOYSTICK_TYPE_ICADE);
        [self setInputMode:inputMode&INPUTMODE_PAD];
        [pManager setNotificationText:@"iCade activated"];
        g_joystick_type = JOYSTICK_TYPE_ICADE;
    }
    else
    {
        option.setIntValue(OPTION_JOYSTICKMODE,JOYSTICK_TYPE_NATIVE_1);
        [pManager setNotificationText:@"iCade de-activated"];
        g_joystick_type = JOYSTICK_TYPE_NATIVE_1;
    }   
    
    [self refreshControls:nil];
        
}


typedef struct {
	int cid;
	const char* name;
	const char* options[5];
	int			optionValues[5];
	UISegmentedControl* hControl;
} scontrol;

 
enum runtimeControlEnum
{
	RC_LOCKZOOM=100,
    RC_JOYSTICK,
	RC_DISPLAY,
	RC_TYPE,
	RC_SPEED,
	RC_FPS,
	RC_KBD,
	RC_MOUSE,
    RC_DISKSOUND,
	RC_AUDIO,
	RC_MEMORY,
	RC_END=-1

};



scontrol  runtimeControlDefs[]={ 
	
#if !defined(BESTOFFTA)
    { 
		RC_SPEED,
		"Speed",
		{ "1Mhz", "2GS", "ZIP",NULL } ,
		{ (int)SPEED_1MHZ, (int)SPEED_GS, (int)SPEED_ZIP },
		nil
	},
    { 
		RC_FPS,
		"Frames per second",
		{ "15", "20","30", "60",NULL } ,
		{ 15, 20,30, 60 },
		nil
	},
	    
	{ 
		RC_AUDIO,
		"Audio",
		{ "None", "22KHz", "44KHz",NULL } ,
		{ 0, 22050, 44100 },
		nil
	},	
	{ 
		RC_MEMORY,
		"Memory Size",
		{ "1MB", "2MB", "4MB",NULL } ,
		{ 1, 2, 4 },
		nil
	},
#endif // BESTOFFTA
    {	
		RC_DISPLAY,
		"Display" ,
		{ "CRT", "LCD", NULL },
		{ (int)VIDEOFX_CRT, (int)VIDEOFX_LCD },
		nil 
	},
	{
		RC_TYPE,
		"Screen",
		{ "B&W", "Green" ,"Colors", NULL },
		{  (int)COLORMODE_BW, (int)COLORMODE_GREEN,(int)COLORMODE_AUTO, },
		nil 
	},

#if !defined(BESTOFFTA)
    {	
		RC_LOCKZOOM,
        "Zoom Control" ,
		{ "Auto", "Lock", "Arcade",NULL },
		{ LOCKZOOM_AUTO,LOCKZOOM_ON, LOCKZOOM_ARCADE},
		nil
	},
    {
        RC_JOYSTICK,
        "Pad Input" ,
        { "TouchPad", "iCade",NULL },
        { JOYSTICK_TYPE_NATIVE_1,JOYSTICK_TYPE_ICADE},
        nil
    },
#ifdef ACTIVEGS_NOHARDWAREKEYBOARDDETECTION
	{ 
		RC_KBD,
		"External Keyboard",
		{ "Disable", "Enable",NULL } ,
		{ 0, 1 },
		nil
	},
#endif
	{ 
		RC_MOUSE,
		"Mouse Control",
		{ "Warp", "Touch",NULL } ,
		{ (int)WARP_POINTER, WARP_TOUCHSCREEN,0 },
		nil
	},
#endif // BESTFTA
    { 
		RC_DISKSOUND,
		"Disk Drive Sounds",
		{ "Enable", "Disable",NULL } ,
		{ (int)1, 0,0 },
		nil
	},
	{
		RC_END,
		NULL,
		NULL,
		NULL,
		nil
	}
		
};


scontrol* findControlByID(int _controlId)
{
	int i=0;
	while(runtimeControlDefs[i].cid!=RC_END)
	{
		if (runtimeControlDefs[i].cid==_controlId)
			return &runtimeControlDefs[i];
		i++;
	}
	return NULL;
}

void setControlValue(int _control,int _v)
{
	scontrol* s = findControlByID(_control);
	if (!s) return ;
	
	int i =0;
	while(s->options[i])
	{
		if (s->optionValues[i]==_v)
		{
			if (s->hControl)
				s->hControl.selectedSegmentIndex = i;
			return;
		}
		i++;
	}
	printf("value %d not found for controlerid:%d\n",_v,_control);
}

extern int x_lock_zoom ;
extern int x_frame_rate ;

-(void)refreshControls:(id)sender
{

	setControlValue(RC_LOCKZOOM, x_lock_zoom);
	setControlValue(RC_DISPLAY, r_sim65816.get_video_fx());
	setControlValue(RC_TYPE, r_sim65816.get_color_mode());
	setControlValue(RC_SPEED, g_sim65816.get_limit_speed());
	setControlValue(RC_FPS, x_frame_rate);
	setControlValue(RC_MOUSE, g_adb.g_warp_pointer);
    setControlValue(RC_DISKSOUND, option.getIntValue(OPTION_DISKSOUND));
	setControlValue(RC_AUDIO, option.getIntValue(OPTION_AUDIORATE)); 
	setControlValue(RC_MEMORY, option.getIntValue(OPTION_MEMORYSIZE)); 
	setControlValue(RC_KBD, option.getIntValue(OPTION_EXTERNALKBD)); 
    setControlValue(RC_JOYSTICK, option.getIntValue(OPTION_JOYSTICKMODE));
     
}

- (void)ctrlChanged:(id)sender
{

	int i = 0;
	while (runtimeControlDefs[i].cid!=RC_END)
	{
		if (sender == runtimeControlDefs[i].hControl )
		{
			int sel = runtimeControlDefs[i].hControl.selectedSegmentIndex;
			int v = runtimeControlDefs[i].optionValues[sel];
			//CEmulator* p = CEmulator::theEmulator;
			switch(runtimeControlDefs[i].cid)
			{
				case RC_LOCKZOOM:
					option.setIntValue(OPTION_LOCKZOOM,v); 
					x_lock_zoom = v;
					if ( (v==LOCKZOOM_ON) || (v==LOCKZOOM_ARCADE) )
						[[pManager getEmulatorView].contentView enableGestures:MODE_ZOOM];
					else
						[[pManager getEmulatorView].contentView disableGestures:MODE_ZOOM];
					break;
                case RC_JOYSTICK:
                    option.setIntValue(OPTION_JOYSTICKMODE,v);
                    g_joystick_type = v;
                    if ( v == JOYSTICK_TYPE_ICADE ) {
                        [self setiCadeMode:YES];
                    } else {
                        [self setiCadeMode:NO];
                    }
					break;
				case RC_DISPLAY:
					// monitor
                {
                    extern void x_set_video_fx(videofxenum _vfx);
					option.setIntValue(OPTION_VIDEOFX,v);
                    x_set_video_fx((videofxenum)v);
                }
				//	[pManager getEmulatorView].zv.crt.hidden = (v==VIDEOFX_CRT?0:1);
					break;
				case RC_TYPE:
					// color
					option.setIntValue(OPTION_COLORMODE,v);
					break;	
				case RC_SPEED:
					// speed
					option.setIntValue(OPTION_SPEED,v);
				//	g_sim65816.set_limit_speed((speedenum)v);
					break;	
				case RC_FPS:
					// frame rate
					option.setIntValue(OPTION_FRAMERATE,v); 
					x_frame_rate = v;
					[[pManager getEmulatorView].zv.ew activateEmulatorRefresh:v];
					break;
				case RC_MOUSE:
					// warp
					g_adb.g_warp_pointer = (warpmode)v;
					break;
				case RC_AUDIO:
					// audiorate
					option.setIntValue(OPTION_AUDIORATE,v); 
					break;
                case RC_DISKSOUND:
					// audiorate
					option.setIntValue(OPTION_DISKSOUND,v); 
					break;
                    
				case RC_MEMORY:
					// memorysize
					option.setIntValue(OPTION_MEMORYSIZE,v); 
					break;
				case RC_KBD:
					// external kbd
					option.setIntValue(OPTION_EXTERNALKBD,v); 
					[self setInputMode:inputMode]; // pour forcer un refresh!
					break;
				default:
					break;
			}
			option.saveOptions(1);
			r_sim65816.request_update_emulator_runtime_config();
			return ;
					
		}
		i++;
	}
	printf("control not found\n");
	
}

-(void) saveStateButtonPressed:(id)sender {
    MyString dir = CDownload::getPersistentPath();
    dir = dir + ACTIVEGS_DIRECTORY_SEPARATOR + "savestates" + ACTIVEGS_DIRECTORY_SEPARATOR + config.name;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error;
    [fileManager createDirectoryAtPath:[NSString stringWithCString:dir.c_str() encoding:NSUTF8StringEncoding] withIntermediateDirectories:YES attributes:nil error:&error];
    dir += ACTIVEGS_DIRECTORY_SEPARATOR;
    NSString *segIndex = [saveStateSegmentedControl titleForSegmentAtIndex:[saveStateSegmentedControl selectedSegmentIndex]];
    dir += "save";
    dir += [segIndex UTF8String];
    g_savestate.saveState(dir.c_str());
    [self optionsButton:nil];
    [pManager setNotificationText:[NSString stringWithFormat:@"Saved State #%@",segIndex]];
}

-(void) loadStateButtonPressed:(id)sender {
    NSString *segIndex = [saveStateSegmentedControl titleForSegmentAtIndex:[saveStateSegmentedControl selectedSegmentIndex]];
    MyString dir = CDownload::getPersistentPath();
    dir += ACTIVEGS_DIRECTORY_SEPARATOR;
    dir += "savestates";
    dir += ACTIVEGS_DIRECTORY_SEPARATOR;
    dir += config.name;
    dir += ACTIVEGS_DIRECTORY_SEPARATOR;
    dir += "save";
    dir += [segIndex UTF8String];
    FILE *fHandle = fopen(dir.c_str(),"r");
    if ( fHandle == NULL ) {
        [pManager setNotificationText:[NSString stringWithFormat:@"State #%@ Does Not Exist",segIndex]];
        return;
    }
    g_savestate.restoreState(dir.c_str());
    [self optionsButton:nil];
    [pManager setNotificationText:[NSString stringWithFormat:@"Loaded State #%@",segIndex]];
}

//
-(void)addRuntimeControls
{
		
	
//	CGSize s = self.view.frame.size;
	

	float	res =  [pManager resolutionRatio];
	float	OPTIONWIDTH = 200* res;
	float	OPTIONMARGIN = 20* res; 
	float	LINEHEIGHT = 20*res;
    
	
	self.runtimeControlsOptions = [[UIView alloc] initWithFrame:CGRectZero];
	self.runtimeControlsOptions.userInteractionEnabled=TRUE; // NOT SET BY DEFAULT ON UIIMAGEVIEW
	
	int l = 0;
	int i=0;
	int nbs=0;
	while (runtimeControlDefs[i].cid!=RC_END)
	{

		
		UILabel* label = [[UILabel alloc] initWithFrame:CGRectMake(OPTIONMARGIN,l,OPTIONWIDTH,LINEHEIGHT)];
		label.text = [NSString stringWithUTF8String:runtimeControlDefs[i].name];
		label.textAlignment = NSTextAlignmentCenter;
		label.font = [UIFont systemFontOfSize:12*res];
		label.backgroundColor = [UIColor clearColor];
		[self.runtimeControlsOptions addSubview:label];
		l+=LINEHEIGHT;
		
		CGRect segrect = CGRectMake(OPTIONMARGIN,l,OPTIONWIDTH,LINEHEIGHT);
		UISegmentedControl* seg = [[UISegmentedControl alloc ] initWithFrame:segrect];
	//	[seg setSegmentedControlStyle:UISegmentedControlStyleBar];
		const char** nameptr = runtimeControlDefs[i].options;
	
		int j=0;
		while(*nameptr)
		{
			[seg insertSegmentWithTitle:[NSString stringWithUTF8String:*nameptr]  atIndex:j++ animated:FALSE];
			nameptr++;
		}									

		[seg addTarget:self action:@selector(ctrlChanged:) forControlEvents:UIControlEventValueChanged];

		runtimeControlDefs[i].hControl = seg;
		[self.runtimeControlsOptions addSubview:seg];
		

		l+=LINEHEIGHT;
		i++;
		nbs++;
				
	}
    
#if !defined(BESTOFFTA)
    // hack in some save state controls
    l += 2.0;
    UILabel* saveStatelabel = [[UILabel alloc] initWithFrame:CGRectMake(OPTIONMARGIN,l,OPTIONWIDTH,LINEHEIGHT)];
    saveStatelabel.text = @"Save States";
    saveStatelabel.textAlignment = NSTextAlignmentCenter;
    saveStatelabel.font = [UIFont systemFontOfSize:12*res];
    saveStatelabel.backgroundColor = [UIColor clearColor];
    [self.runtimeControlsOptions addSubview:saveStatelabel];
    l += LINEHEIGHT;
    
    saveStateSegmentedControl = [[UISegmentedControl alloc] initWithFrame:CGRectMake(OPTIONMARGIN, l, OPTIONWIDTH, LINEHEIGHT)];
    for (int segIndex = 0; segIndex < 6; segIndex++) {
        [saveStateSegmentedControl insertSegmentWithTitle:[NSString stringWithFormat:@"%d",segIndex] atIndex:segIndex animated:NO];
    }
    [saveStateSegmentedControl setSelectedSegmentIndex:0];
    [self.runtimeControlsOptions addSubview:saveStateSegmentedControl];

    l += LINEHEIGHT;
    l += 2.0;
    
    UIButton *saveStateButton = [UIButton buttonWithType:UIButtonTypeCustom];
    saveStateButton.frame = CGRectMake(OPTIONMARGIN,l,OPTIONWIDTH * 0.4,LINEHEIGHT);
    [saveStateButton setTitle:@"Save" forState:UIControlStateNormal];
    saveStateButton.titleLabel.font = [UIFont systemFontOfSize:12*res];
    [saveStateButton setTitleColor:self.view.tintColor forState:UIControlStateNormal];
    saveStateButton.backgroundColor = [UIColor clearColor];
    saveStateButton.layer.borderWidth = 1.0f;
    saveStateButton.layer.borderColor = [self.view.tintColor CGColor];
    [saveStateButton addTarget:self action:@selector(saveStateButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.runtimeControlsOptions addSubview:saveStateButton];
    
    UIButton *loadStateButton = [UIButton buttonWithType:UIButtonTypeCustom];
    loadStateButton.frame = CGRectMake((OPTIONWIDTH + OPTIONMARGIN) - (OPTIONWIDTH * 0.4),l,OPTIONWIDTH * 0.4,LINEHEIGHT);
    [loadStateButton setTitle:@"Load" forState:UIControlStateNormal];
    loadStateButton.titleLabel.font = [UIFont systemFontOfSize:12*res];
    loadStateButton.backgroundColor = [UIColor clearColor];
    [loadStateButton setTitleColor:self.view.tintColor forState:UIControlStateNormal];
    loadStateButton.layer.borderWidth = 1.0f;
    loadStateButton.layer.borderColor = [self.view.tintColor CGColor];
    [loadStateButton addTarget:self action:@selector(loadStateButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.runtimeControlsOptions addSubview:loadStateButton];    
    
#endif // BESTOFTA
    l+=LINEHEIGHT;
    nbs++;
	
	float w = OPTIONWIDTH+OPTIONMARGIN*2;

	// resize la vue correctement
	[self.runtimeControlsOptions setFrame:CGRectMake(0,0,w,l)];

	float ho = nbs*(LINEHEIGHT+OPTIONMARGIN) + (LINEHEIGHT+OPTIONMARGIN) + OPTIONMARGIN*2 ;
	if (ho>280*res)
		ho = 280*res;
	
	CGRect r = CGRectMake(0,0,w,ho);
	self.runtimeControls = [[UIImageView alloc] initWithFrame:r];
    self.runtimeControls.userInteractionEnabled=TRUE; // NOT SET BY DEFAULT ON UIIMAGEVIEW
	self.runtimeControls.alpha = 0.0;
		
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB(); 
	CGContextRef context = CGBitmapContextCreate(nil, (int)r.size.width,(int)r.size.height, 8, (int)r.size.width*4,colorSpace,kCGImageAlphaPremultipliedLast);
	
	float ovalWidth=14,ovalHeight=14;
    CGContextSaveGState(context);
    CGContextTranslateCTM (context, CGRectGetMinX(r), CGRectGetMinY(r));
    CGContextScaleCTM (context, ovalWidth, ovalHeight);
    float fw = CGRectGetWidth (r) / ovalWidth;
    float fh = CGRectGetHeight (r) / ovalHeight;
    CGContextMoveToPoint(context, fw, fh/2);
    CGContextAddArcToPoint(context, fw, fh, fw/2, fh, 1);
    CGContextAddArcToPoint(context, 0, fh, 0, fh/2, 1);
    CGContextAddArcToPoint(context, 0, 0, fw/2, 0, 1);
    CGContextAddArcToPoint(context, fw, 0, fw, fh/2, 1);
    CGContextClosePath(context);
	CGContextRestoreGState(context);
	CGContextClip(context);

	const CGFloat* col = CGColorGetComponents( [[UIColor colorWithWhite:1.0 alpha:0.8] CGColor]);
	CGContextSetFillColor(context,col);
	CGContextFillRect(context,r);
	
	// TODO RAJOUTER UNE OMBRE
	
	CGImageRef imageRef = CGBitmapContextCreateImage(context);
	UIImage* newImage = [UIImage imageWithCGImage:imageRef];
	self.runtimeControls.image = newImage;
    
    // cleanup
	CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);

	const float lb = ho - LINEHEIGHT - OPTIONMARGIN;
	const float WB = 100 * res;
	
	 UIButton* button = [UIButton buttonWithType:(UIButtonType)100]; 
	 [button addTarget:self action:@selector(optionsButton:) forControlEvents:UIControlEventTouchUpInside];
	 [button setTitle:@"Done" forState:UIControlStateNormal];
	 
	 

	 button.frame = CGRectMake((w-WB-OPTIONMARGIN),lb,WB,LINEHEIGHT);
	 
	 [self.runtimeControls addSubview:button];
	 
	 UIButton* buttonrestore = [UIButton buttonWithType:(UIButtonType)100]; 
	 [buttonrestore addTarget:self action:@selector(restoreButton:) forControlEvents:UIControlEventTouchUpInside];
	 [buttonrestore setTitle:@"Defaults" forState:UIControlStateNormal];
	 
	 buttonrestore.frame = CGRectMake(OPTIONMARGIN,lb,WB,LINEHEIGHT);
	 
	 [self.runtimeControls addSubview:buttonrestore];
	 
	

	// ajoute le scrollView
	
    self.uit = [[UIScrollView alloc]initWithFrame:CGRectMake(0,OPTIONMARGIN,w,-OPTIONMARGIN+ho-OPTIONMARGIN-LINEHEIGHT*2)];
	self.uit.userInteractionEnabled=TRUE; // NOT SET BY DEFAULT ON UIIMAGEVIEW
	self.uit.showsHorizontalScrollIndicator = NO;
	self.uit.showsVerticalScrollIndicator = YES;
	self.uit.maximumZoomScale = 1.0;
	self.uit.minimumZoomScale = 1.0;
	self.uit.contentSize = CGSizeMake(self.runtimeControlsOptions.frame.size.width,self.runtimeControlsOptions.frame.size.height); 
	[self.uit addSubview:self.runtimeControlsOptions];
	[self.runtimeControls addSubview:self.uit];
		
	/*
	[[runtimeControls layer] setCornerRadius:8];
	[[runtimeControls layer] setMasksToBounds:NO];
	[[runtimeControls layer] setShadowColor:[UIColor blackColor].CGColor];
	[[runtimeControls layer] setShadowOpacity:1.0f];
	[[runtimeControls layer] setShadowRadius:6.0f];
	[[runtimeControls layer] setShadowOffset:CGSizeMake(0, 3)];
	*/
	
	[self.view addSubview:self.runtimeControls];
	
									
}



-(void)addDiskSelection
{
    
    self.diskSelectionOptions = nil;
    self.diskSelection =  nil;
    
    if (!pMac)
        return ;
    int swap = pMac->getSmartSwap();
    if (!swap)
        return ;
    
    int slot = swap/10;
    int disk = swap%10;
    
    
  //  CActiveImage& active = pMac->config->getActiveImage(slot,disk);
	
    //	CGSize s = self.view.frame.size;
	
    
	float	res =  [pManager resolutionRatio];
	float	SELECTIONWIDTH = 200* res;
	float	OPTIONMARGIN = 10* res;
	float	LINEHEIGHT = 20*res;
    float   MOUNTWIDTH = 64*res;
	
	self.diskSelectionOptions = [[UIView alloc] initWithFrame:CGRectZero];
	self.diskSelectionOptions.userInteractionEnabled=TRUE; // NOT SET BY DEFAULT ON UIIMAGEVIEW
	
	int l = 0;
//	int i=0;
	int nbs=0;
//	while (runtimeControlDefs[i].cid!=RC_END)
    for(int i=0;i<ACTIVEGSMAXIMAGE;i++)
	{
        

        
		CSlotInfo& info = pMac->config->localImages[slot-5][disk-1][i];
        if (info.status==UNDEFINED)
            continue ;
		UILabel* label = [[UILabel alloc] initWithFrame:CGRectMake(OPTIONMARGIN,l,SELECTIONWIDTH-MOUNTWIDTH,LINEHEIGHT)];
		MyString l8;
        l8.Format("#%d %s",i,info.shortname);
        label.text = [NSString stringWithUTF8String:l8.c_str()];
        NSLog(@"adding %@",label.text);
		label.textAlignment = UITextAlignmentLeft;
		label.font = [UIFont systemFontOfSize:12*res];
		label.backgroundColor = [UIColor clearColor];
         [self.diskSelectionOptions addSubview:label];
        
        if( info.status!=MOUNTED)
        {
            UIButton* button = [UIButton buttonWithType:(UIButtonType)100];
            button.tag = slot+disk*10+i*100;
            [button addTarget:self action:@selector(mountButton:) forControlEvents:UIControlEventTouchUpInside];
            [button setTitle:@"Mount" forState:UIControlStateNormal];
            [button setFrame:CGRectMake(OPTIONMARGIN+SELECTIONWIDTH,l,MOUNTWIDTH,LINEHEIGHT)];
             [self.diskSelectionOptions addSubview:button];
        }
        
        
      
       
        
        nbs++;
		l+=LINEHEIGHT;
	//	i++;
    }
   	
	float w = MOUNTWIDTH+SELECTIONWIDTH+OPTIONMARGIN*2;
    
	// resize la vue correctement
	[self.diskSelectionOptions setFrame:CGRectMake(0,0,w,l)];
    
	float ho = nbs*(LINEHEIGHT+OPTIONMARGIN) + (LINEHEIGHT+OPTIONMARGIN) + OPTIONMARGIN*2 ;
	if (ho>280*res)
		ho = 280*res;
	
	CGRect r = CGRectMake(0,0,w,ho);
	self.diskSelection= [[UIImageView alloc] initWithFrame:r];
    self.diskSelection.userInteractionEnabled=TRUE; // NOT SET BY DEFAULT ON UIIMAGEVIEW
	self.diskSelection.alpha = 0.0;
    
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(nil, (int)r.size.width,(int)r.size.height, 8, (int)r.size.width*4,colorSpace,kCGImageAlphaPremultipliedLast);
	
	float ovalWidth=14,ovalHeight=14;
    CGContextSaveGState(context);
    CGContextTranslateCTM (context, CGRectGetMinX(r), CGRectGetMinY(r));
    CGContextScaleCTM (context, ovalWidth, ovalHeight);
    float fw = CGRectGetWidth (r) / ovalWidth;
    float fh = CGRectGetHeight (r) / ovalHeight;
    CGContextMoveToPoint(context, fw, fh/2);
    CGContextAddArcToPoint(context, fw, fh, fw/2, fh, 1);
    CGContextAddArcToPoint(context, 0, fh, 0, fh/2, 1);
    CGContextAddArcToPoint(context, 0, 0, fw/2, 0, 1);
    CGContextAddArcToPoint(context, fw, 0, fw, fh/2, 1);
    CGContextClosePath(context);
	CGContextRestoreGState(context);
	CGContextClip(context);
    
	const CGFloat* col = CGColorGetComponents( [[UIColor colorWithWhite:1.0 alpha:0.8] CGColor]);
	CGContextSetFillColor(context,col);
	CGContextFillRect(context,r);
	
	// TODO RAJOUTER UNE OMBRE
	
	CGImageRef imageRef = CGBitmapContextCreateImage(context);
	UIImage* newImage = [UIImage imageWithCGImage:imageRef];
	self.diskSelection.image = newImage;
    
    // cleanup
	CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
	const float lb = ho - LINEHEIGHT - OPTIONMARGIN;
	const float WB = 50 * res;
	
    UIButton* button = [UIButton buttonWithType:(UIButtonType)100];
    [button addTarget:self action:@selector(diskSelectionButton:) forControlEvents:UIControlEventTouchUpInside];
    [button setTitle:@"Close" forState:UIControlStateNormal];
    
    
    
    button.frame = CGRectMake((w-WB-OPTIONMARGIN),lb,WB,LINEHEIGHT);
    
    [self.diskSelection addSubview:button];
    /*
    UIButton* buttonrestore = [UIButton buttonWithType:(UIButtonType)100];
    [buttonrestore addTarget:self action:@selector(restoreButton:) forControlEvents:UIControlEventTouchUpInside];
    [buttonrestore setTitle:@"Default" forState:UIControlStateNormal];
    
    buttonrestore.frame = CGRectMake(OPTIONMARGIN,lb,WB,LINEHEIGHT);
    
    [self.runtimeControls addSubview:buttonrestore];
    */
	
    
	// ajoute le scrollView
	
   UIScrollView* sw = [[UIScrollView alloc]initWithFrame:CGRectMake(0,OPTIONMARGIN,w,-OPTIONMARGIN+ho-OPTIONMARGIN-LINEHEIGHT*2)];
	sw.userInteractionEnabled=TRUE; // NOT SET BY DEFAULT ON UIIMAGEVIEW
	sw.showsHorizontalScrollIndicator = NO;
	sw.showsVerticalScrollIndicator = YES;
	sw.maximumZoomScale = 1.0;
	sw.minimumZoomScale = 1.0;
	sw.contentSize = CGSizeMake(self.diskSelectionOptions.frame.size.width,self.diskSelectionOptions.frame.size.height);
	[sw addSubview:self.diskSelectionOptions];
	[self.diskSelection addSubview:sw];
    
	/*
     [[runtimeControls layer] setCornerRadius:8];
     [[runtimeControls layer] setMasksToBounds:NO];
     [[runtimeControls layer] setShadowColor:[UIColor blackColor].CGColor];
     [[runtimeControls layer] setShadowOpacity:1.0f];
     [[runtimeControls layer] setShadowRadius:6.0f];
     [[runtimeControls layer] setShadowOffset:CGSizeMake(0, 3)];
     */
	
	[self.view addSubview:self.diskSelection];
	
    
}




// Ajoute les modifiers au clavier

-(void)addRuntimeKeys
{
	
	if (self.runtimeView)
	{
		printf("removing RuntimeView");
		[self.runtimeView removeFromSuperview];
		self.runtimeView;
	}
	
	// position par défault
	CGRect rv3 = [UIScreen mainScreen].applicationFrame;
	
	self.runtimeView = [[UINavigationBar alloc] initWithFrame:CGRectMake(0, 40, rv3.size.width, 36.0)];	
	self.runtimeView.barStyle = UIBarStyleBlack;
	self.runtimeView.translucent = TRUE;
	
	
	int pos = 2;
	int nbkey = sizeof(runtimeKeysList)/sizeof(rkey);
					   
	for(int i=0;i<nbkey;i++)
	{
		
		UIButton* button = [UIButton buttonWithType:(UIButtonType)100]; // left-pointing shape!
		[button addTarget:self action:@selector(runtimeButton:) forControlEvents:UIControlEventTouchUpInside];
		[button setTitle:[NSString  stringWithUTF8String:runtimeKeysList[i].label] forState:UIControlStateNormal];
		button.tag = i; 
		
		button.frame = CGRectMake(pos,4,48,16);
		button.titleLabel.font = [UIFont systemFontOfSize:9];
		button.alpha = 0.8;
		
		[self.runtimeView addSubview:button];
		
		runtimeKeysList[i].hButton = button; 
	
		pos += 64*[pManager resolutionRatio];
	}

	[self.barView addSubview:self.runtimeView];
	
	
}



-(void)updateSpecialButtonView:(UIButton*)_b state:(int)_state
{
	
	//NSLog(@"Button %@ state:%d",_b,_state);
	if (_state!=1)
	{
		
		//UIImage* b = [_b backgroundImageForState:UIControlStateNormal];
		//[_b setBackgroundImage:b /*imgDefault*/ forState:UIControlStateNormal];
		_b.backgroundColor=[UIColor clearColor];
		
	}
	else
	{
		
		//UIImage* b = [_b backgroundImageForState:UIControlStateHighlighted];
		//[_b setBackgroundImage:b /*imgSelected*/ forState:UIControlStateNormal];
		_b.backgroundColor=[UIColor blueColor];
	}
}

- (void)leftAccessView:(id)sender
{
	[self hideInput];
}


- (void)rightAccessView:(id)sender
{
	[self cycleInput];
}

- (void)specialButton:(id)sender
{
	UIButton* s = (UIButton*)sender;
	
	int t = s.tag; 
	int code = buttonList[t].code;
	int state = buttonList[t].state;
	if (state<0)
	{

		if (code & shiftKey)
			add_event_modifier(shiftKey);
		add_event_key(code & 0xFF,0);
		
		NSNumber* ui = [[NSNumber alloc ]initWithInt:code];
		[NSTimer scheduledTimerWithTimeInterval:1.0/20.0 target:self selector:@selector(keyup:) userInfo:ui repeats:NO];
	}
	else
	{
		add_event_key(code,state);	
		
		if (state)
			state=0;
		else
			state = 1;

		for(int j=0;j<NBBUTTONLOOP;j++)
			[self updateSpecialButtonView:buttonList[t].hButton[j] state:state];
		
		buttonList[t].state = state;
	}
}




- (void)viewDidLoad
{
    
    NSLog(@"---kbd viewDidLoad");
}

- (void)viewDidUnload 
{
	
    NSLog(@"---kbd viewDidUnload");

	[self unloadSpecialKeys];
	[self hideLoader:nil];
		
	[super viewDidUnload];

}

-(void)viewWillAppear:(BOOL)animated
{
      NSLog(@"---kbd viewWillAppear");

	self.padCircleInner.alpha = 0.5;
	
	joyX= 0;
	joyY= 0;
			
	oaButton = FALSE;
	caButton = FALSE;
	
	self.padTouch = nil;
	self.oaButtonTouch = nil;
	self.caButtonTouch = nil;
	
	self.oaButtonView.alpha = 0.5;
	self.caButtonView.alpha = 0.5;
	
    
	// pour repositionner le disk "swap" dans le bon sense
	diskAngle = 0;
    self.diskView.layer.sublayers= nil;
  //  self.diskView.layer = nil;
	[self animateDisk:NULL];
    
    
    // efface licone dechargement
    self.diskLoadingView.alpha= 0.0;
   
    self.animateDiskTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/10.0 target:self selector:@selector(animateDisk:) userInfo:nil repeats:YES];

    
#ifndef ACTIVEGS_NOHARDWAREKEYBOARDDETECTION
   self.hardwarekbdDetectionTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(detectHardwareKeyboard:) userInfo:nil repeats:YES];
#endif

}

- (void)viewWillDisappear:(BOOL)animated;     // Called when the view has been fully transitioned onto the screen. Default does nothing
{
    NSLog(@"---kbd viewWillDisappear");

    [self.animateDiskTimer invalidate];
    self.animateDiskTimer = nil;
    
#ifndef ACTIVEGS_NOHARDWAREKEYBOARDDETECTION
    [self.hardwarekbdDetectionTimer invalidate];
    self.hardwarekbdDetectionTimer = nil;
#endif

}

#ifdef ACTIVEGS_CUSTOMKEYS
-(void)updateCustomKeyView:(CGRect)_rv
{
    if (inputMode & INPUTMODE_CUSTOMKEYS)
        show_custom_keys(_rv);
    else
        hide_custom_keys();
}
#endif

-(void)updatePadView
{
	CGRect ofr = self.padCircleInner.frame ;
	float posx = (outterSize-innerSize)/2*(joyX+1.0);
	float posy = (outterSize-innerSize)/2*(joyY+1.0);
	ofr.origin.x = posx;
	ofr.origin.y = posy;
	self.padCircleInner.frame = ofr;
	
	int padDisplayed = [self isInputVisible:INPUTMODE_PAD ];
	
	self.padCircleOutter.hidden = !padDisplayed;
	self.oaButtonView.hidden = !padDisplayed;
	self.caButtonView.hidden = !padDisplayed;
	
	
#ifdef GRAPHICAL_PAD
	self.padBackground.hidden =  !padDisplayed; 
	self.padStick.hidden =  !padDisplayed; 
	self.padButton1Down.hidden = true; //!padDisplayed; 
	self.padButton2Down.hidden = true; 
	
#endif	
}
	

-(CGFloat)getEmulatorScale
{
	if ([self isInputVisible:INPUTMODE_PAD])
		return 1.0f;
	else 
		return keyboardScale;
}

-(CGRect)getEmulatorDelta
{
	if ([self isInputVisible:INPUTMODE_PAD])
	{
        return CGRectZero;
        /*
		BOOL isPortrait = abs([pManager getAngle])==90 ;
		CGFloat h;

		if (isPortrait)
		{
#ifdef GRAPHICAL_PAD
			h = padBackground.frame.size.height;
#else
			if ([self isInputVisible:INPUTMODE_MINIACCESS])
				h=buttonSize+BUTTON_PADDING*2;
			else
				h = (buttonSize+BUTTON_PADDING)*2;  // keyboard?
#endif		
		}
		else 
			h=0;

		return CGRectMake(0,0,0,h);
         */
	}
	else 
		return keyboardDelta;
}


-(UIImageView*)createCircle:(float)_radius red:(float)_red green:(float)_green blue:(float)_blue alpha:(float)_alpha
{
	CGRect icircle = CGRectMake(0,0,_radius,_radius);
	CGColorSpaceRef innerColorSpace = CGColorSpaceCreateDeviceRGB(); 

	CGContextRef icontext = CGBitmapContextCreate(nil, (int)icircle.size.width,(int)icircle.size.height, 8, (int)icircle.size.width*4,innerColorSpace,kCGImageAlphaPremultipliedLast);
	
	CGContextSetRGBFillColor(icontext,_red,_green,_blue,_alpha);
	CGContextAddEllipseInRect(icontext, icircle);
	CGContextFillPath(icontext);
    CGImageRef innerImageRef = CGBitmapContextCreateImage(icontext);
    
    
    CGColorSpaceRelease(innerColorSpace);
    CGContextRelease(icontext);
    
	UIImage* img = [UIImage imageWithCGImage:innerImageRef];
	return [[UIImageView alloc]initWithImage:img];
}

-(void) updateView:(CGRect)rv3;
{

	debug_printf("kbdc updateView\n");
	
	float s = [pManager resolutionRatio];
	
	// repositionne les options
	
	CGRect f = self.runtimeControls.frame;
	f.origin.x = (rv3.size.width - f.size.width)/2;
	f.origin.y = (rv3.size.height - f.size.height)/2;
	self.runtimeControls.frame = f;
    
    // repositionne diskSelection
	
	/*CGRect*/ f = self.diskSelection.frame;
	f.origin.x = (rv3.size.width - f.size.width)/2;
	f.origin.y = (rv3.size.height - f.size.height)/2;
	self.diskSelection.frame = f;
	
	// Repositionne l'icone disk

	 self.diskLoadingView.layer.transform = CATransform3DIdentity;    // vire le layer avant de faire des calculs sur la frame
    CGSize sil = [self.diskLoadingView.image size];
	CGRect oldiml = [self.diskLoadingView frame];
	oldiml.origin.x = rv3.size.width - (sil.width+4)*s;
	[self.diskLoadingView setFrame:oldiml];
    
 
    self.diskView.layer.transform = CATransform3DIdentity;    // vire le layer avant de faire des calculs sur la frame
	CGSize si = [self.diskView.image size];
	CGRect oldim = [self.diskView frame];
	oldim.origin.x = rv3.size.width - (si.width+4)*s - (sil.width+4)*s;
	//oldim.origin.y = 48;
	[self.diskView setFrame:oldim];
	

	
    // Etire la barre derriere les boutons
	CGRect oldbar = [self.barView frame];
	oldbar.size.width = rv3.size.width;
	[self.barView setFrame:oldbar];
	
	
	float padDeltaY=0;
	if ([self isInputVisible:INPUTMODE_ACCESS])
		padDeltaY = -50;
		
	// repositionne le pad
	CGRect po = self.padCircleOutter.frame;
	po.origin.x = BUTTON_PADDING;
	po.origin.y = rv3.size.height - po.size.height-BUTTON_PADDING + padDeltaY;
	self.padCircleOutter.frame = po;
	
	CGRect oa = self.oaButtonView.frame;
	oa.origin.y = rv3.size.height - oa.size.height*2-BUTTON_PADDING  + padDeltaY ;
	oa.origin.x = rv3.size.width - oa.size.width-BUTTON_PADDING;
	self.oaButtonView.frame = oa;


	CGRect ca = self.caButtonView.frame;
	ca.origin.y = rv3.size.height - ca.size.height-BUTTON_PADDING  + padDeltaY ;
	ca.origin.x = rv3.size.width - ca.size.width-BUTTON_PADDING;
	self.caButtonView.frame = ca;
	
	[self updatePadView];
	[self updateSpecialKey:rv3];
    
#ifdef ACTIVEGS_CUSTOMKEYS
    [self updateCustomKeyView:rv3];
#endif
}

#define SPECIALKEYWIDTH 64
#define SPECIALKEYPAD 8

-(void) updateSpecialKey:(CGRect)rv3
{
    
	if (![self isInputVisible:(INPUTMODE_ACCESS+INPUTMODE_MINIACCESS)])
	{
		debug_printf("accessView.hidden=TRUE");
		self.accessView.hidden=TRUE;
		return ; 
	}
		
	self.accessView.hidden = FALSE;
	debug_printf("accessView.hidden=FALSE");
	if (![self isInputVisible:(INPUTMODE_MINIACCESS)])
	{
		uitb.hidden=FALSE;
	}
	else
	{
		uitb.hidden=TRUE;
	}


	
	int ypos = 0;
	float s = [pManager resolutionRatio];
		
	if (![self isInputVisible:INPUTMODE_KBD] && !isHardwareKeyboard()/*(inputMode & INPUTMODE_HARDWAREKBD)*/)
	{
		ypos  = rv3.size.height - ACCESS_HEIGHT*s;
	}
		
		
	// repositionne les special keys (ne fonctionne pas lors de la création???)
		
	float pos = SPECIALKEYPAD *s;

	for(int j=0;j<NBBUTTONLOOP;j++)
	{
	int i = 0;
	while(buttonList[i].label)
	{
		[buttonList[i].hButton[j] setFrame:CGRectMake(pos,2,SPECIALKEYWIDTH *s,(ACCESS_HEIGHT-4)*s)];
		pos += (SPECIALKEYPAD+SPECIALKEYWIDTH)*s;
		i++;
	}	
	}
	
	// resize la special view
	[self.specialView setFrame:CGRectMake(0,0,pos+SPECIALKEYPAD*s,ACCESS_HEIGHT*s)];
	
	self.uitb.contentSize = CGSizeMake(self.specialView.frame.size.width,self.specialView.frame.size.height); 
	
	[self.uitb setFrame:CGRectMake(0,0,rv3.size.width,ACCESS_HEIGHT*s)];
	
	[self.accessView setFrame:CGRectMake(0,ypos,rv3.size.width,ACCESS_HEIGHT*s)];
	
	[self.rightAccessView setFrame:CGRectMake(rv3.size.width+(-UITB_BUTTON_WIDTH+UITB_BUTTON_PADDING)*s,0,UITB_BUTTON_WIDTH*s,ACCESS_HEIGHT*s)];
	
	[self.leftAccessView setFrame:CGRectMake(-UITB_BUTTON_PADDING*s,0,UITB_BUTTON_WIDTH*s,ACCESS_HEIGHT*s)];
	
	
	
}


// Reinitialise les modifiers au lancement de l'emulateur

-(void)resetSpecialKeys
{
	int i = 0;
	while(buttonList[i].label)
	{
		if (buttonList[i].state != -1)
			buttonList[i].state = 0;
		i++;
	}	
}

// Elimine les modifiers si une touche normal est appuyé

-(void)clearSpecialKeys
{
	for(int j=0;j<NBBUTTONLOOP;j++)
	{
	int i = 0;
	while(buttonList[i].label)
	{
		if (buttonList[i].state == 1)
		{
			add_event_key(buttonList[i].code,1);
			buttonList[i].state = 0;
			for(int j=0;j<NBBUTTONLOOP;j++)
				[self updateSpecialButtonView:buttonList[i].hButton[j] state:0];
			
		}
		i++;
	}
	}

}

-(void)unloadSpecialKeys
{
	
	for(int j=0;j<NBBUTTONLOOP;j++)
	{
	int i = 0;
	while(buttonList[i].label)
	{
		
		UIButton* b = buttonList[i].hButton[j];
		if (b)
		{
			[b removeFromSuperview];
			buttonList[i].hButton[j] = nil;
		}
		i++;
	}
	}
	
	if (self.specialView)
	{
		[self.specialView removeFromSuperview];
		self.specialView = nil;
	}
}

// Ajoute les modifiers au clavier

-(void)addSpecialKeys
{
	
	self.specialView = [[UINavigationBar alloc] initWithFrame:CGRectZero];		
	self.specialView.barStyle = UIBarStyleBlack;
	self.specialView.translucent = TRUE;	
	
	for(int j=0;j<NBBUTTONLOOP;j++)
	{
	int i = 0;
	while(buttonList[i].label)
	{

		
		UIButton* button = [UIButton buttonWithType:(UIButtonType)100]; // left-pointing shape!
		[button addTarget:self action:@selector(specialButton:) forControlEvents:UIControlEventTouchUpInside];
		[button setTitle:[NSString  stringWithUTF8String:buttonList[i].label] forState:UIControlStateNormal];
		button.tag = i; 
	
		button.titleLabel.font = [UIFont systemFontOfSize:12*[pManager resolutionRatio]];
		button.alpha = 0.8;
		
		[self.specialView addSubview:button];
		buttonList[i].hButton[j] = button; 
		i++;
	}
	}

	// Crée un scrollView
	self.uitb = [[UIScrollView alloc]initWithFrame:CGRectZero]; 
	self.uitb.showsHorizontalScrollIndicator = NO;
	self.uitb.showsVerticalScrollIndicator = NO;
	self.uitb.maximumZoomScale = 1.0;
	self.uitb.minimumZoomScale = 1.0;
//	uitb.bounces=NO; (cf. ci-dessous)
//	uitb.contentSize = CGSizeMake(specialView.frame.size.width,specialView.frame.size.height); 
	self.uitb.contentOffset = CGPointZero; //CGPointMake(specialView.frame.size.width/2,0);
	[self.uitb addSubview:self.specialView];
	
	self.accessView = [[UIView alloc]initWithFrame:CGRectZero];
	
	self.leftAccessView = [UIButton buttonWithType:(UIButtonType)100]; // left-pointing shape!
	[self.leftAccessView addTarget:self action:@selector(leftAccessView:) forControlEvents:UIControlEventTouchUpInside];
	[self.leftAccessView setTitle:@"†" forState:UIControlStateNormal];
	
	self.rightAccessView = [UIButton buttonWithType:(UIButtonType)100]; // left-pointing shape!
	[self.rightAccessView addTarget:self action:@selector(rightAccessView:) forControlEvents:UIControlEventTouchUpInside];
	[self.rightAccessView setTitle:@"◊" forState:UIControlStateNormal];
	
	[self.accessView addSubview:self.uitb];
	// après pour qu'elles apparaissent devant!
	[self.accessView addSubview:self.leftAccessView];
	[self.accessView addSubview:self.rightAccessView];
	
	// ajoute un observer pour gérer le scroll infini
	[self.uitb addObserver:self forKeyPath:@"contentOffset" options:0 context:nil];
		
	
}

// Infinite scroll
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object  change:(NSDictionary *)change context:(void *)context
{
	CGPoint p = uitb.contentOffset;
	if (p.y != 0.0)
		p.y = 0.0;
	if (p.x>self.specialView.frame.size.width/2)
	{
		p.x -= self.specialView.frame.size.width/2; 
	}	
	/*
	// A UTILISER AVEC BOUNCES=NO : NE MARCHE PAS!
	if (p.x<uitb.frame.size.width)
	{
		p.x += specialView.frame.size.width/2; 
		uitb.contentOffset = p;
		printf("*new :%f\n",p.x);
	}
	 */
	if (p.x<0)
		p.x += self.specialView.frame.size.width/2; 
		

	if (p.x != self.uitb.contentOffset.x  || p.y != self.uitb.contentOffset.y )
		self.uitb.contentOffset = p;


}
	

-(void)setGestureIndicatorVisibility:(int)_mode color:(UIColor*)_color
{
	if (_mode & MODE_ZOOM)
		self.zoomIndicator.backgroundColor = _color;
	if (_mode & MODE_PAD)
		self.padIndicator.backgroundColor = _color;
	if (_mode & MODE_SWIPE)
		self.swipeIndicator.backgroundColor = _color;
	if (_mode & MODE_MOUSEBUTTON)
		self.mouseButtonIndicator.backgroundColor = _color;
	if (_mode & MODE_EMULATOR)
		self.inputIndicator.backgroundColor = _color;
}


-(void) animateDiskEjection:(id)_id
{
    
#ifdef _DEBUG
    CALayer *layer = self.diskLoadingView.layer;
	CATransform3D rotationAndPerspectiveTransform = CATransform3DIdentity;
    rotationAndPerspectiveTransform.m34 = 1.0 / -200;
    rotationAndPerspectiveTransform = CATransform3DRotate(rotationAndPerspectiveTransform, M_PI / 180.0f /3, 0.0f, 1.0f, 0.0f);
    layer.transform = rotationAndPerspectiveTransform;
#endif
    
}

- (void) animateDisk:(id)_id 
{
   
	CATransform3D rotationAndPerspectiveTransform = CATransform3DIdentity;
	if (!bLoading && pMac)
	{
		int sw = pMac->getSmartSwap();
		if (bMenuBarVisibility && sw)
		{
			rotationAndPerspectiveTransform.m34 = 1.0 / -200;
			rotationAndPerspectiveTransform = CATransform3DRotate(rotationAndPerspectiveTransform, diskAngle * M_PI / 180.0f, 0.0f, 1.0f, 0.0f);
			self.diskView.hidden = false;	
		}
		else
			self.diskView.hidden = true;
	}
    else
        self.diskView.hidden = true;
    
	self.diskView.layer.transform = rotationAndPerspectiveTransform;
	diskAngle += 10;
		
}

- (void) keyup:(NSTimer*)theTimer
{
	NSNumber* ui = [theTimer userInfo];
	int c = [ui intValue];
	if (c & shiftKey)
		add_event_modifier(0);
	add_event_key(c&0xFF,1);
	
	
	if (c!=0x36 && c!=0x37 && c!=0x3a)
		[self clearSpecialKeys];
}



const char* iCadeDetectString="wewexzxz";
int iCadeDetectPos=0;

int keypad_x = 0;
int keypad_y = 0;


void paddle_trigger_icade(double dcycs)
{
	g_paddles.g_paddle_val[0] = keypad_x;
	g_paddles.g_paddle_val[1] = keypad_y;
	g_paddles.g_paddle_val[2] = 32767;
	g_paddles.g_paddle_val[3] = 32767;
	g_moremem.g_paddle_buttons |= 0xc;
	paddle_update_trigger_dcycs(dcycs);
}

int x_adb_get_keypad_x()
{
    return keypad_x;
}

int x_adb_get_keypad_y()
{
    return keypad_y;
}


    

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string 
{
	const char* s = [string UTF8String];
  //  printf("input:%s\n",s);
    if (!iCadeDetected || bForceOnScreenKeyboard)
    {
        if (*s == iCadeDetectString[iCadeDetectPos])
            iCadeDetectPos++;
        else
            iCadeDetectPos=0;
    
        if (!iCadeDetectString[iCadeDetectPos])
        {
            NSLog(@"icade activated");
            iCadeDetectPos=0;
            if (bForceOnScreenKeyboard)
            {
                [self OnScreenKeyboard:FALSE];
            }
               
            else
                [self setiCadeMode:TRUE];
            return NO;

            option.setIntValue(OPTION_JOYSTICKMODE,JOYSTICK_TYPE_ICADE);
            [self setInputMode:inputMode&INPUTMODE_PAD];
            [self refreshControls:nil];
        }
    }
    else
    if (!bForceOnScreenKeyboard)
    {
            
        char c;
        int i=0;
        while( (c = s[i++]) != 0) 
        {
            switch(c)
            {
                case 'w': // up
                    keypad_y = -32767;
                    break;
                case 'e': // !verti
                case 'z':
                    keypad_y = 0;
                    break;
                case 'x': // down
                    keypad_y = 32767;
                    break;
                case 'a': // left
                    keypad_x = -32767;
                    break;
                case 'q':   // !hori
                case 'c':
                    keypad_x = 0;
                    break;
                case 'd': // right
                    keypad_x = 32767;
                    break;
                case 'y': // button 1 pressed
                    add_event_key(0x37, 0);
                    break;
                case 't': // button 1 depressed
                    add_event_key(0x37, 1);
                    break;
                case 'h': // button 2 pressed
                    add_event_key(0x3a, 0);
                    break;
                case 'r': // button 2 depressed
                    add_event_key(0x3a, 1);
                    break;
                case  'g': //à faire sur un keydepressed
                    // display keyboard      
                    printf("*** forcing on-screeen keyboard");
                    [self OnScreenKeyboard:TRUE];        
                     break;
                case  'v': //à faire sur un keydepressed
                    // toggle la menu bar
                    [self setMenuBarVisibility:!bMenuBarVisibility];
                    break;
                default:
                    break;
            }
                    
        }
        return NO; //
        
    }
        
	if (!*s)
		s = "delete";	
	else
		if (!strcmp(s,"\n"))
			s = "return";
    
    
    
	int c = findCode(s);
	debug_printf("text: %s code:%d\n",s,c);
	if (c>=0)
	{
		if (c & shiftKey)
			add_event_modifier(shiftKey);
		add_event_key(c&0xFF,0);
		NSNumber* ui = [[NSNumber alloc ]initWithInt:c];
		[NSTimer scheduledTimerWithTimeInterval:1.0/20.0 target:self selector:@selector(keyup:) userInfo:ui repeats:NO];
	}
	
    return NO;
}


- (BOOL) myTouchesBegan:(NSSet *)touches 
{
	
	UITouch* touch =[touches anyObject];
	CGPoint p = [touch locationInView:self.view];
	CGRect r  = self.diskView.frame;
	if (CGRectContainsPoint(r,p) && pMac->getSmartSwap() && !bLoading)
	{
	//	[self swapButton:self];
        [self addDiskSelection];
        [self diskSelectionButton:self];
        
	}
	
#ifdef ACTIVEGS_CUSTOMKEYS
    if (inputMode & INPUTMODE_CUSTOMKEYS)
        touch_begin_custom_key(p);
#endif
    
	if (!padEnabled || !([self isInputVisible:INPUTMODE_PAD]))
		return FALSE;
	
	// regarde si on touche le pad
	CGRect ro = self.padCircleOutter.frame;
	if (CGRectContainsPoint(ro,p) && !self.padTouch)
	{
		self.padTouch = touch;
		posPad = [touch locationInView:self.view];
		self.padCircleInner.alpha = 1.0;
	}
	
	// ca button
	CGRect ca = self.caButtonView.frame;
	if (CGRectContainsPoint(ca,p) && !self.caButtonTouch)
	{
		add_event_key(0x3A, 0);

		for(int i=0;i<20;i++)
			add_event_delay();	
		caButton = TRUE;
		self.caButtonTouch = touch;;
		self.caButtonView.alpha = 1.0;
#ifdef GRAPHICAL_PAD
		self.padButton2Down.hidden = FALSE;
#endif
		
	}
	
	// oa button
	CGRect oa = self.oaButtonView.frame;
	if (CGRectContainsPoint(oa,p) && !self.oaButtonTouch)
	{
		add_event_key(0x37, 0);
		for(int i=0;i<20;i++)
			add_event_delay();	
		oaButton = TRUE;
		self.oaButtonTouch = touch;
		self.oaButtonView.alpha = 1.0;
#ifdef GRAPHICAL_PAD
		self.padButton1Down.hidden = FALSE;
#endif
		
	}
	
	
	
	return self.padTouch || caButton || oaButton ;
}

-(void)centerPad:(id)center
{
#define CENTER_STEP 0.01
	
	if (joyX == 0.0 && joyY == 0.0) return ;
	if (joyX < 0) joyX+=CENTER_STEP;
	if (joyX > 0) joyX-=CENTER_STEP;
	if (joyY < 0) joyY+=CENTER_STEP;
	if (joyY > 0) joyY-=CENTER_STEP;
	if (abs(joyX)<CENTER_STEP) joyX=0.0;
	if (abs(joyY)<CENTER_STEP) joyY=0.0;
	[self updatePadView];
	[NSTimer scheduledTimerWithTimeInterval:1.0/20.0 target:self selector:@selector(centerPad:) userInfo:nil repeats:NO];
}

-(void)enablePad:(BOOL)_enable
{
	if (_enable==padEnabled)
		return ;
	debug_printf("enablePad:%d",_enable);
	padEnabled = _enable;
	
}

- (BOOL) myTouchesEnded:(NSSet *)touches 
{
#ifdef ACTIVEGS_CUSTOMKEYS
    if (inputMode & INPUTMODE_CUSTOMKEYS)
        touch_end_custom_key();
#endif    
	
	if ([touches containsObject:self.padTouch])
	{
		self.padCircleInner.alpha = 0.5;
		self.padTouch = nil;
		[self centerPad:nil];
	}
	
	if ([touches containsObject:self.caButtonTouch])
	{
		add_event_key(0x3A, 1); // Option
		
		caButton = FALSE;
		
		[UIView beginAnimations:nil context:NULL];
		[UIView setAnimationDuration:0.2];
		self.caButtonView.alpha = 0.5;
		[UIView commitAnimations];
		
#ifdef GRAPHICAL_PAD
		self.padButton2Down.hidden = TRUE;
#endif
		self.caButtonTouch = nil;
	}
	
	if ([touches containsObject:self.oaButtonTouch])
	{
		add_event_key(0x37, 1); // COMMAND
		oaButton = FALSE;
		[UIView beginAnimations:nil context:NULL];
		[UIView setAnimationDuration:0.2];
		self.oaButtonView.alpha = 0.5;
		[UIView commitAnimations];
#ifdef GRAPHICAL_PAD
		padButton1Down.hidden = TRUE;
#endif
        self.oaButtonTouch = nil;
	}
		
	return self.padTouch || caButton || oaButton ;
}


- (BOOL) myTouchesMoved:(NSSet *)touches 
{
//	debug_printf("kbd myTouchesMoved");

	if ([touches containsObject:self.padTouch])
	{

		CGPoint newpos = [self.padTouch locationInView:self.view];
		
		joyX += (newpos.x - posPad.x)/(outterSize-innerSize);
		joyY += (newpos.y - posPad.y)/(outterSize-innerSize);	
		if (joyX<-1.0) joyX=-1.0;
		if (joyX>1.0) joyX=1.0;
		if (joyY<-1.0) joyY=-1.0;
		if (joyY>1.0) joyY=1.0;
		posPad = newpos;
		[self updatePadView];
	}
	
	return self.padTouch || caButton || oaButton ;

}

- (void)notifyPaddle:(id)sender
{
    if (x_lock_zoom == LOCKZOOM_ARCADE)
        [self setInputMode:(INPUTMODE_ARCADE + INPUTMODE_PAD)];
    else
        [self setInputMode:(INPUTMODE_PAD + INPUTMODE_MINIACCESS)];
}


-(void)setInputMode:(int)_mode
{

	debug_printf("setting inputMode to : %X (was:%X)\n",_mode,inputMode);
	inputMode = _mode;
	
	if ([self isInputVisible:INPUTMODE_PAD])
		padEnabled = TRUE;
	

	BOOL b = [self.textField isFirstResponder];
	if ([self isInputVisible:INPUTMODE_KBD] || isHardwareKeyboard() /*(inputMode & INPUTMODE_HARDWAREKBD)*/)
	{
        if (isHardwareKeyboard())
            debug_printf("*enabling HW keyboard\n");
        else
            debug_printf("*enabling keyboard\n");
        
		// A faire avant le becomeFirstResponder

        debug_printf("setInputAccessoryView:accessView");
        // IOS8
        if ( !isHardwareKeyboard() ) {
            [self.accessView removeFromSuperview];
            [self.textField setInputAccessoryView:self.accessView];
        }
		
		[self.textField becomeFirstResponder];
	}
	else 
	{
		if (b)
			[self.textField resignFirstResponder];
	
        debug_printf("setInputAccessoryView:nil");
        [self.textField setInputAccessoryView:nil];

		debug_printf("addSubview:accessView");
		[self.interfaceView addSubview:self.accessView];

	}
	
	
	[[pManager getEmulatorView] updateView];

}

-(void)showDebug:(BOOL)_visible
{
	
	self.debugIndicator.hidden = !_visible;
	self.swipeIndicator.hidden = !_visible;
	self.zoomIndicator.hidden = !_visible;
	self.padIndicator.hidden = !_visible;
	self.mouseButtonIndicator.hidden = !_visible;
	self.inputIndicator.hidden = !_visible;
}


-(void)setDebugIndicatorText:(const char*)_text
{
	NSString* str = [NSString stringWithUTF8String:_text];
	self.debugIndicator.text  = str;
}


const int cycles[]= 
{
	
	INPUTMODE_PAD+INPUTMODE_MINIACCESS,
	INPUTMODE_ACCESS,
	INPUTMODE_KBD+INPUTMODE_ACCESS,
	INPUTMODE_ACCESS+INPUTMODE_PAD
#ifdef ACTIVEGS_CUSTOMKEYS
    ,INPUTMODE_CUSTOMKEYS
#endif
};


- (void)swipeDown
{
	debug_printf("swipeDown");

    
    if ( !(inputMode & INPUTMODE_ARCADE) && [self isInputVisible:INPUTMODE_ALL] )
	{
		[self hideInput];
		return ;
	}
	
	if (!bMenuBarVisibility)
	{
		debug_printf("*displaying menubar\n");
		[self setMenuBarVisibility:TRUE];
		[self showDebug:FALSE];
	
	}
	else
	{
		if (option.getIntValue(OPTION_DEBUGMODE))
			[self showDebug:TRUE];
	}

	
}

- (void)swipeUp
{

	debug_printf("swipeUp");

	// si la barre est cachée, fait la apparaitre
	
	if (bMenuBarVisibility)
	{
		debug_printf("*hidding menubar\n");
		[self setMenuBarVisibility:FALSE];
		return;
	}
	
    // en mode arcade, on ne peut pas cycler dans les interfaces
    
	if (inputMode & (INPUTMODE_CUSTOMKEYS | INPUTMODE_ARCADE))
        return ;
    
	// si l'interface était cachée, fait la réapparaitre
	
	if (inputMode & INPUTMODE_HIDDEN)
	{
		[self setInputMode:inputMode &~INPUTMODE_HIDDEN];
		return ;
	}
		
	// sinon, cycle dans les interfaces de saisies
	[self cycleInput];

}


-(void)cycleInput
{
	
	
	const int nbcycle = sizeof(cycles)/sizeof(int);
	int m = cycles[0] ; // default value
	for(int i =0;i<nbcycle;i++)
	{
		if (cycles[i]==(inputMode&~INPUTMODE_HIDDEN))
		{
			m = cycles[(i+1)%nbcycle];

			if (m == INPUTMODE_ACCESS && isHardwareKeyboard())
				m = cycles[(i+2)%nbcycle]; 
			break;
		}
	}
	
	[self setInputMode:m];
	
	
}

- (void)setLoading:(NSNumber*)_param
{
    int v = [_param intValue];
    BOOL _visible = v &0x80;
    int diskid = v & 0x7F;
    if (_visible)
        [self.diskLoadingView setImage:[self.diskIcons objectAtIndex:diskid]];
    
	if (bLoading == _visible) 
		return ;

    bLoading = _visible;
    
    [UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:0.7];
    if (bLoading)
        self.diskLoadingView.alpha = 0.8;
    else
        self.diskLoadingView.alpha = 0.0;
    
    [UIView commitAnimations];

    /*
	if (_visible)
		debug_printf("Motor On\n");
	else
		debug_printf("Motor Off\n");
	
	bLoading = _visible;
     */
}


- (void)setDownLoading:(NSNumber*)_percent
{
	int pc =[_percent intValue];
	self.loader.hidden=FALSE;
	[self.loader startAnimating];
	[self.loaderTimer invalidate];
	self.loaderTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(hideLoader:) userInfo:nil repeats:NO];
	self.loaderLabel.text = [NSString stringWithFormat:@"%d",pc];
	debug_printf("setDownLoading:%d",pc);
}

- (void)hideLoader:(id)_id
{
	
	self.loader.hidden=TRUE;
	[self.loader stopAnimating];
	self.loaderTimer = nil;
	
}

/*- (void)toggleMenuBarVisibility
{
	[self setMenuBarVisibility:!bMenuBarVisibility];
}
 */





- (void)hideMenuBar:(id)_id
{
	[self setMenuBarVisibility:FALSE];
}
 

- (void)setMenuBarVisibility:(int)_visible
{

	[UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:0.2];
	
	CGRect r = self.barView.frame ;
	float barAlpha;
	if (_visible)
	{
		r.origin. y = 0.0;
		barAlpha=1.0;
	}
	else
	{
		r.origin. y = - r.size.height ;
		barAlpha=0.0;
	}
	[self.barView setFrame:r];
	[self.barView setAlpha:barAlpha];
	
		
	[UIView commitAnimations];
	bMenuBarVisibility = _visible ; 
	
	if (_visible && [pManager hasSecondary]==false)
	{
	//	[NSTimer scheduledTimerWithTimeInterval:10.0 target:self selector:@selector(hideMenuBar:) userInfo:nil repeats:NO];
	}

}

- (void) keyboardWillHide:(NSNotification*)aNotification
{
    NSLog(@"Keyboard will hide");

    
	if ( ( inputMode & INPUTMODE_KBD ) && !(inputMode & INPUTMODE_HIDDEN)  )
	{
		printf("user removed keyboard on iPad!\n");
		//inputMode &= ~INPUTMODE_KBD;
		inputMode |= INPUTMODE_HIDDEN;
	}
    
   
	keyboardDelta = CGRectZero;
	keyboardScale = 1.0;
	[[pManager getEmulatorView] updateView];
	
}

- (void) keyboardDidHide:(NSNotification*)aNotification
{
	debug_printf("keyboardDidHide\n");
        

    debug_printf("inputAccessoryView=nil");
    self.textField.inputAccessoryView=nil;
	
    debug_printf("addSubview:accessView");
	[self.interfaceView addSubview:self.accessView];

    if (bForceOnScreenKeyboard)
    {
        [self OnScreenKeyboard:FALSE];
    }

}

-(void)OnScreenKeyboard:(BOOL)_enabled
{
    
    if (!_enabled)
    {
        ASSERT (bForceOnScreenKeyboard);
            
        
        printf("disabling on-screen keyboard\n");
        bForceOnScreenKeyboard = FALSE;
#ifndef ACTIVEGS_NOHARDWAREKEYBOARDETECTION
        GSEventSetHardwareKeyboardAttached(YES);
        UIKeyboardOrderOutAutomatic();
#endif
        [self setInputMode:INPUTMODE_KBD|INPUTMODE_HIDDEN];
    }
    else
    {
        ASSERT (!bForceOnScreenKeyboard);
        
        printf("enabling on-screen keyboard\n");
        bForceOnScreenKeyboard = TRUE;
#ifndef ACTIVEGS_NOHARDWAREKEYBOARDETECTION
        GSEventSetHardwareKeyboardAttached(NO);
        UIKeyboardOrderInAutomatic(); 
#endif
        [self setInputMode:INPUTMODE_KBD+INPUTMODE_ACCESS];
    }
        
}

- (void) keyboardDidShow:(NSNotification*)aNotification
{
	
	debug_printf("keyboardDidShow\n");
	
	BOOL isPortrait = abs([pManager getAngle])==90 ;
	
	CGPoint n = [self.textField.inputAccessoryView convertPoint:CGPointZero toView:self.view];	
	float accessY = n.y;
	float accessHeight = self.textField.inputAccessoryView.frame.size.height;
	float frameY ;
	float deltaY;
	CGSize ns = self.view.frame.size;

	if (isPortrait)
	{
		frameY = ns.height;
		deltaY = 0;
	}
	else
	{
		frameY = ns.width;
		deltaY = 0; //accessHeight;
	}
	
	float err = fabs(keyboardDelta.size.height - (frameY-accessY-accessHeight+deltaY) ); 
	if (err > ACCESS_HEIGHT)
	{
		// Bug!
		printf("resizing keyboard to compensate bug in UIKeyboardFrameEndUserInfoKey\n");
        
        //
        if (bForceOnScreenKeyboard)
        {
            // on-screen keyboard has been dismissed
            printf("on-screen keyboard has been dismissed...");
            [self OnScreenKeyboard:FALSE];       
        }
        
		[self updateViewForKeyboard:deltaY];
	}
	else
	{
// A ELIMINER QD ON POURRA DETECTER LES CLAVIERS HARDWARE AVEC LE SDK
#ifdef ACTIVEGS_NOHARDWAREKEYBOARDDETECTION
		if (keyboardDelta.size.height)
		{
			if (option.getIntValue(OPTION_EXTERNALKBD))
			{
				printf("*** reverting option ***");
				option.setIntValue(OPTION_EXTERNALKBD,0);
				option.saveOptions(1);
				x_refresh_panel(PANEL_RUNTIME);
				[self setInputMode:inputMode]; // pour forcer un refresh!
				x_alert("No external keyboard detected! Disabling option.");
			}
		}
#endif
	}

	

}

- (void)updateViewForKeyboard:(int)_height
{
    
	
	BOOL isPortrait = abs([pManager getAngle])==90 ;
	
	keyboardDelta.origin.x = 0; 
	keyboardDelta.origin.y = 0;
	keyboardDelta.size.width = 0; 
	keyboardDelta.size.height = _height;
	if (isPortrait)
		keyboardScale = 1;
	else
	{
		float h = self.view.bounds.size.width ;
		keyboardScale = (h - _height) / h ;
	}
	debug_printf("updateViewForKeyboard: h:%d s:%f",_height,keyboardScale);
	[[pManager getEmulatorView] updateView];
	
}

- (void) keyboardWillShow:(NSNotification*)aNotification
{
	debug_printf("keyboardWillShow\n");
    
	CGFloat _keyboardHeight;
	CGRect _keyboardEndFrame;
	BOOL isPortrait = abs([pManager getAngle])==90 ;
	

	NSDictionary *info = [aNotification userInfo];
	[[info valueForKey:UIKeyboardFrameEndUserInfoKey] getValue:&_keyboardEndFrame];			
		
		if (isPortrait)
			_keyboardHeight = _keyboardEndFrame.size.height;
		else 
			_keyboardHeight = _keyboardEndFrame.size.width;
		
		// enleve l'accessory
		_keyboardHeight -= self.specialView.frame.size.height;
		

	debug_printf("keyboardheight:%f\n",	_keyboardHeight);
	[self updateViewForKeyboard:_keyboardHeight];
	
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


- (void)menuButton:(id)sender
{
	[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromLeft];

}


 
- (void)mountButton:(UIButton*)sender
 {
     int  i = sender.tag;
     int active  = i / 100;
     int disk = (i - active*100)/10;
     int slot = (i - active*100)%10;
   
     pMac->swapDisk(slot,disk,active);
     [self diskSelectionButton:self]; // pour fermer la fenetre
     
     
 }
 /*
- (void)swapButton:(id)sender
{
	NSString* str = nil;
	int sw = pMac->getSmartSwap();
	if (!sw)
		str = @"Nothing to swap";
	else
	{
		pMac->smartSwap();
		CSlotInfo& info =pMac->getLocalIMGInfo(sw/10,sw%10);
		
		MyString s;
		s.Format("Disk %s inserted",info.shortname.c_str());
		str = [NSString  stringWithUTF8String: s.c_str()];
	}
	UIAlertView *alert = [[[UIAlertView alloc] initWithTitle:@"ActiveGS" message:str delegate:self cancelButtonTitle:@"Done" otherButtonTitles:nil] autorelease];
	[alert show];
	
}
 */


- (void)setEmulatorTitle:(const char*)_name 
{
	printf("setEmulatorTitle:%s",_name);
	if (_name && *_name)
	{
		NSString* str = [NSString  stringWithUTF8String: _name];
		[self.emulatorNavItem setTitle:str];
	}
}

- (void)restoreButton:(id)sender
{
	//CEmulator* p = CEmulator::theEmulator;
	option.setDefaultOptions();
	option.saveOptions(1);
	[self refreshControls:nil];
}

-(void)enableKeyboard:(int)_enable
{
    if (_enable)
        inputMode |= INPUTMODE_KBD;
    else
        inputMode &= ~INPUTMODE_KBD;
    [self setInputMode:inputMode];

}

-(int)isInputVisible:(int) _im
{
	if (inputMode & INPUTMODE_HIDDEN)
		return 0;
	return inputMode & _im;
}

-(void)restoreInput
{
	[self setInputMode:inputMode & ~INPUTMODE_KBD];
}


-(void)hideInput
{
	[self setInputMode:inputMode | INPUTMODE_HIDDEN];
}

- (void)optionsButton:(id)sender
{

	// vire le clavier
	/*
	if ([self isInputVisible:INPUTMODE_KBD]) 
		[self hideInput];
     */
  //  [self enableKeyboard:FALSE];
    [self restoreInput];


	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.3];
	float a = self.runtimeControls.alpha;
	if (a==1.0)
		a=0.0;
	else 
	{
		[self.uit flashScrollIndicators];
		a=1.0;
	}
	self.runtimeControls.alpha = a;

	self.runtimeControls.hidden = NO;
	[UIView commitAnimations];
}

- (void)diskSelectionButton:(id)sender
{
    

    [self restoreInput];
    
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.3];
	float a = self.diskSelection.alpha;
	if (a==1.0)
		a=0.0;
	else
	{
	//	[self.uit flashScrollIndicators];
		a=1.0;
	}
	self.diskSelection.alpha = a;
    
	self.diskSelection.hidden = NO;
	[UIView commitAnimations];
}




- (void)dealloc 
{
    
    NSLog(@"---kbd dealloc");
    self.textField.delegate = nil;
    

	
}


@end

