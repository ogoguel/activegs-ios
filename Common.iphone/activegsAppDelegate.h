/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#import <UIKit/UIKit.h>
#import <QuartzCore/CAAnimation.h>
#include "../Common.osx/cemulatorctrlmac.h"

#define IPHONETHUMB "thumb"

#ifdef USE_CAPPTAIN
#import "../Libraries/CapptainSDK/Headers/CapptainAgent.h"
#import "../Libraries/CapptainSDK/Headers/CapptainViewController.h"
	#define  UIVIEWCONTROLLERROOT CapptainViewController
#else
	#define  UIVIEWCONTROLLERROOT UIViewController
#endif

@interface ActiveGSBaseViewController : UIViewController
{
    
}

//- (void) update;
@end


//#import "activegsList.h"
#import "activegsEmulatorController.h"


//#import "activegsViewController.h"
#import "infoViewController.h"
#import "detailViewController.h"

#ifdef USE_GOOGLEANALYTICS
#import "../Libraries/GoogleAnalytics/GAI.h"
#endif


#import "activegsViewController.h"
#define ACTIVEGS_LAUNCHVIEWCONTROLLER activegsViewController

#ifdef BESTOFFTA


#define ACTIVEGS_TITLE "ActiveGS - Best Of FTA"	
#define ACTIVEGS_CAPPTAINAPP "act384142"
#define ACTIVEGS_CAPPTAINKEY "376e71d41db2422c88e1486a8a31e25b"
//#define ACTIVEGS_BACKGROUNDIMAGE "Default"
#define ACTIVEGS_FIRSTTABLIST "list" 
#define ACTIVEGS_FIRSTTABTITLE "Best Of FTA" 
#define ACTIVEGS_FIRSTTABTRACKER "BestOfFta"
#define ACTIVEGS_FIRSTTABICON "BestOfFta"
#define ACTIVEGS_SINGLETAB              // Une seule liste sera affichée
#define ACTIVEGS_IMAGEREQUIRED    // On ne peut pas lancer l'émulateur sans image
#define ACTIVEGS_NOAPPLEBRANDING // Eliminer les logos Apple 2 & Apple 2GS
#define ACTIVEGS_NOHARDWAREKEYBOARDETECTION // Pour passer la soumission Appstore
#define ACTIVEGS_NOHARDWAREKEYBOARDDETECTION
#define ACTIVEGS_NOTIFICATION
#define HANDLE_URL
#define ACTIVEGS_NOMFISUPPORT

 
#elif ACTIVEGSKARATEKA
#error unsupported

#elif BESTOF
#define ACTIVEGS_TITLE "ActiveGS - BestOf"
#define ACTIVEGS_CAPPTAINAPP "act037442"
#define ACTIVEGS_CAPPTAINKEY "881708d1abb44dbb97fcb0620b3f4ef0"
#define ACTIVEGS_BACKGROUNDIMAGE "karatekalaunch"
#define ACTIVEGS_FIRSTTABLIST "bestof" 
#define ACTIVEGS_FIRSTTABTITLE "Jordan's" 
#define ACTIVEGS_FIRSTTABICON "Tab2GS"
#define ACTIVEGS_FIRSTTABTRACKER "JM"

#else 
// Default ACTIVEGS
#define ACTIVEGS_TITLE "ActiveGS"	
#define ACTIVEGS_CAPTAINAPP "act651309"
#define ACTIVEGS_CAPTAINKEY "26ae7a388bf74c8c993d78bebb63fc41"
#undef  ACTIVEGS_BACKGROUNDIMAGE
#define ACTIVEGS_ENABLE_DOCUMENT
#define ACTIVEGS_NOTIFICATION
//#define ACTIVEGS_MANUAL_ROM
#define ACTIVEGS_SAVELASTTAB
#define ACTIVEGS_FIRSTTABLIST "list" 
#define ACTIVEGS_FIRSTTABTITLE "Best Of FTA" 
#define ACTIVEGS_FIRSTTABTRACKER "BestOfFta"
#define ACTIVEGS_FIRSTTABICON "TabFTA"
#define HANDLE_URL
#define ACTIVEGS_NOHARDWAREKEYBOARDETECTION // Pour passer la soumission Appstore

#endif


#define BARVIEW_HEIGHT 44

//extern UIView* backgroundView;
//extern UIImageView* backgroundImageView;

extern CEmulatorMac* pMac;
extern CEmulatorConfig config;

enum machineSpecsEnum
{
    SPEC_DEFAULT=0,
    FPS_60HZ = 1,
    AUDIO_44KHZ = 2
};

extern enum machineSpecsEnum machineSpecs;

@interface activegsAppDelegate : NSObject <UIApplicationDelegate,UIAlertViewDelegate> {
    UIWindow *_window;
    UIWindow *_secondaryWindow;
    
	UILabel* _notificationView;
    UIViewController* _primaryViewController ;
    UIViewController* _secondaryViewController ;
    UIView* _backgroundView;
    UIImageView* _backgroundImageView ;


	//controllers
	activegsEmulatorController* _emulatorController;
	ACTIVEGS_LAUNCHVIEWCONTROLLER* _viewController;
	infoViewController* _infoController;
	detailViewController* _detailController ;
	float _currentRawReading;
	float _dpiRatio ; 
	float _resolutionRatio;
    NSTimer* _notificationTimer;
    UIScreen* _secondaryScreen;
    UIScreen* _primaryScreen;
    UIDeviceOrientation curOrientation;
@public
    MyString firstTabXML;
#ifdef USE_GOOGLEANALYTICS
    id<GAITracker> GAITrackerActiveGS;
#endif
    
}


- (void) screenDidConnect:(NSNotification *)notification;
-(void) initSecondaryScreen:(UIScreen*) _screen;
-(bool) hasSecondary;
-(void)setSecondaryInputMode;


- (void) doStateChange:(UIViewController*)_viewController type:(NSString*)_type sens:(NSString*)_sens;
-(void)launchDiskImage:(ActiveGSElement*)_element pathname:(const char*)_pathname trackername:(const char*)_trackername;
- (void) didRotate:(NSNotification *)notification;
-(void)firstLaunch:(NSTimer*)timer;
-(void)setNotificationText:(NSString*) _text; 
-(void)updateNotificationView:(CGRect) newRect;
- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation;
@property(nonatomic,strong,getter=getEmulatorView) activegsEmulatorController* emulatorController;
@property(nonatomic,strong,getter=getBrowserView) ACTIVEGS_LAUNCHVIEWCONTROLLER* viewController;
@property(nonatomic,strong,getter=getInfoView) infoViewController* infoController;
@property(nonatomic,strong,getter=getDetailView) detailViewController* detailController;
@property(nonatomic,strong) UIViewController* primaryViewController;
@property(nonatomic,strong) UIViewController* secondaryViewController;
@property(nonatomic,strong) UIView* backgroundView;
@property(nonatomic,strong) UIImageView* backgroundImageView ;
@property(nonatomic,strong) UIScreen* secondaryScreen ;
@property(nonatomic,strong) UIScreen* primaryScreen ;

@property(nonatomic,strong) NSTimer* notificationTimer;
@property(nonatomic,strong) UILabel* notificationView;
@property(assign,getter=getAngle) float currentRawReading;
@property (nonatomic, strong)/* IBOutlet*/  UIWindow *window;
@property (nonatomic, strong) UIWindow *secondaryWindow;
@property(assign) float dpiRatio;
@property(assign) float resolutionRatio;

@end


extern activegsAppDelegate* pManager;
