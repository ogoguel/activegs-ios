/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "activegsAppDelegate.h"
#include "../Common/svnversion.h"
#include "../Kegs/Src/adb.h"
#include "../Kegs/Src/sim65816.h"
#include "../Kegs/Src/driver.h"
#include "../Common/ki.h"
#import <AudioToolbox/AudioToolbox.h>
#include "asynccommand.h"
#import <sys/utsname.h>

// Application Singleton
activegsAppDelegate* pManager = nil;

//int runningOnIOS_3_2 = 0;
enum machineSpecsEnum machineSpecs = SPEC_DEFAULT;

CEmulatorMac* pMac = NULL;
CEmulatorConfig config;

 
#ifdef _DEBUG
// #define TEST_HANDLE_URL
#endif

#define ENABLE_AIRPLAY

MyString resourcesPath;

int padAlreadyDisplayedOnce = 0;


// mets à jour le framerate
extern char estimatedSpeed[256];
extern	int getFrameRate(int _id);
void x_update_framerate()
{
    char str[256];
    sprintf(str,"req:%d fps:%d speed:%s",getFrameRate(1),getFrameRate(0),estimatedSpeed);
    [[pManager getEmulatorView].kbdc setDebugIndicatorText:str];
}



void activegs_apply_default_options(void* _config)
{
	namevalue* options = ((COption*)_config)->options;

    if (machineSpecs & FPS_60HZ)
        options[OPTION_FRAMERATE].intvalue = 60;
    else
        options[OPTION_FRAMERATE].intvalue = 20;
 
    
    if (machineSpecs & AUDIO_44KHZ)
        options[OPTION_AUDIORATE].intvalue = 44100;
    else
        options[OPTION_AUDIORATE].intvalue = 22050;
        
}


void x_notify_eject()
{
    @autoreleasepool {
        [[pManager getEmulatorView].kbdc performSelectorOnMainThread:@selector(animateDiskEjection:) withObject:nil waitUntilDone:NO];
    }
    
}

void x_set_video_fx(videofxenum _vfx)
{
    	[pManager getEmulatorView].zv.crt.hidden = (_vfx==VIDEOFX_CRT?0:1);
}


void x_notify_paddle(int _on)
{
    
	// only once per view
	if (padAlreadyDisplayedOnce)
		return;
	
	printf("x_notify_paddle");
	padAlreadyDisplayedOnce = 1;
	
	@autoreleasepool {
		[[pManager getEmulatorView].kbdc  performSelectorOnMainThread:@selector(notifyPaddle:) withObject:nil waitUntilDone:NO];
	}	
	
}


void x_notify_download_failure(const char*_url)
{
    printf("download failed : %s\n",_url);
	x_display_alert(0,"Downloading Failed" ,"Check your Internet Connection!");
}

void x_loadinginprogress(int _on,int _s,int _d)
{
	@autoreleasepool {
    
    
        _d --;
        int param = (_s==5 ? _d : _d +2 );
        if (_on) param  |= 0x80;
        
        NSNumber* b = [NSNumber numberWithInt:param] ;
	[[pManager getEmulatorView].kbdc performSelectorOnMainThread:@selector(setLoading:) withObject:b waitUntilDone:NO];
	}
}

void x_refresh_panel(int _panel)
{
	if (!_panel & PANEL_RUNTIME) return ;
	@autoreleasepool {
  	  	[[pManager getEmulatorView].kbdc performSelectorOnMainThread:@selector(refreshControls:) withObject:nil waitUntilDone:NO];
	}
}

void x_downloadinginprogress(int _percent)
{
	@autoreleasepool {
	
		NSNumber* b = [NSNumber numberWithInt:_percent];
		[[pManager getEmulatorView].kbdc performSelectorOnMainThread:@selector(setDownLoading:) withObject:b waitUntilDone:NO];
	}
}

int		 alertAbort;
MyString alertTitle;
MyString alertMessage;




void x_init_persistent_path(MyString& hp)
{
	@autoreleasepool {
    
		NSArray *paths= NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
		NSString *dir = [paths objectAtIndex:0];
		const char *bar = [dir UTF8String]; 
		CFStringRef sd = __CFStringMakeConstantString(bar);
		hp= CFStringGetCStringPtr(sd,CFStringGetSystemEncoding());
	
	}	
}


@implementation ActiveGSBaseViewController
@end


@implementation activegsAppDelegate

@synthesize emulatorController  = _emulatorController;
@synthesize viewController = _viewController;
@synthesize infoController = _infoController;
@synthesize detailController = _detailController;
@synthesize currentRawReading = _currentRawReading;
@synthesize window = _window;
@synthesize secondaryWindow = _secondaryWindow;
@synthesize dpiRatio = _dpiRatio;
@synthesize resolutionRatio = _resolutionRatio;
@synthesize notificationTimer = _notificationTimer;
@synthesize notificationView = _notificationView;
@synthesize primaryViewController = _primaryViewController;
@synthesize secondaryViewController = _secondaryViewController;
@synthesize primaryScreen = _primaryScreen;
@synthesize secondaryScreen = _secondaryScreen;
@synthesize backgroundView = _backgroundView;
@synthesize backgroundImageView = _backgroundImageView;


- (void)displayAlert:(id)sender {	
	
	NSString* strMsg =  [NSString stringWithUTF8String:alertMessage.c_str()];
	NSString* strTitle =  [NSString stringWithUTF8String:alertTitle.c_str()];
	
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:strTitle message:strMsg delegate:self cancelButtonTitle:@"Done" otherButtonTitles:nil];
	[alert show];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if (!alertAbort)
		return ;
	
	// quite l'émulateur & revient au menu principal
	if (pMac)
	{
		delete pMac;
		pMac = NULL;
	}
	[self doStateChange:[self getBrowserView] type:kCATransitionPush sens:kCATransitionFromRight];	
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{    
    CDownload::initPersistentPath();
    
    MyString dir ;
	dir = CDownload::getPersistentPath();
	dir += ACTIVEGS_DIRECTORY_SEPARATOR;
	dir += IPHONETHUMB;
    CDownload::createDirectory(dir.c_str());
    

    NSString* resource = [[NSBundle mainBundle] pathForResource:@ACTIVEGS_FIRSTTABLIST ofType:@"activegsxml"] ;
    self->firstTabXML = [resource UTF8String];
    
#define THEVER "/"ACTIVEGS_TITLE "_" TOSTRING(ACTIVEGSMAJOR) "." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD)
    
	NSString* ver = [NSString stringWithUTF8String:THEVER];    
	NSLog(@"%@",ver);
    NSString* resPath = [[NSBundle mainBundle] resourcePath];
    resourcesPath =[resPath UTF8String];
    
	
#ifdef USE_GOOGLEANALYTICS
//	static const NSInteger kGANDispatchPeriodSec = 10;
    GAITrackerActiveGS = [[GAI sharedInstance ]trackerWithTrackingId:@"UA-89449-3"] ;
	[GAITrackerActiveGS sendView:ver];
#endif
	
	
#ifdef USE_CAPPTAIN
	[CapptainAgent registerApp:@ACTIVEGS_CAPTAINAPP identifiedBy:@ACTIVEGS_CAPTAINKEY];
#endif
    
#ifdef ACTIVEGS_MANUAL_ROM
	// setup the rom path (in the main thread)
	{
		extern void initRomPath();
		initRomPath();
	}
#endif
	
	// Modify global variables to tweak the emulator on iphone
	g_adb.g_warp_pointer = WARP_POINTER;
	config.bootslot=7;
    
    CGRect physicalRect = [[UIScreen mainScreen] bounds];
    self.window = [[UIWindow alloc] initWithFrame:physicalRect];
	
	[[UIApplication sharedApplication] setStatusBarHidden:TRUE];
	
	CGFloat screenScale;
	
	NSString* systemVer = [[UIDevice currentDevice] systemVersion];
	NSLog(@"iOS version : %@",systemVer);
	printf("Min SDK Version : " TOSTRING(__IPHONE_OS_VERSION_MIN_REQUIRED));
	printf("Max SDK Version : " TOSTRING(__IPHONE_OS_VERSION_MAX_ALLOWED));
    printf("%f x %f",physicalRect.size.width,physicalRect.size.height);
    
    struct utsname systemInfo;
    uname(&systemInfo);
    
     NSString* platformVer = [NSString stringWithCString:systemInfo.machine
                                                encoding:NSUTF8StringEncoding];
	NSLog(@"iOS platform : %@",platformVer);
	
    screenScale = [[UIScreen mainScreen] scale];
	self.dpiRatio = screenScale;
	
	
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        self.resolutionRatio = 1.6; // pour que la liste prenne toute la hauteur de l'ipad
    else
        self.resolutionRatio = 1.0;

    machineSpecs = SPEC_DEFAULT;
   
	if (self.resolutionRatio != 1.0 || screenScale != 1.0f)
	{
		printf("fps set to 60hz");
		machineSpecs =  (machineSpecsEnum)(machineSpecs | FPS_60HZ);
	}
    if ( [platformVer compare:@"iPhone5"]==NSOrderedDescending
        || [platformVer compare:@"iPad3"]==NSOrderedDescending )
	{
        printf("audio set to 44 khz");
		machineSpecs =  (machineSpecsEnum)(machineSpecs | AUDIO_44KHZ);
    }
	 
	self.currentRawReading = -90; // default mode = portrait
	pManager = self;
	

    extern void activegs_driver();
    g_driver.init(activegs_driver);
    g_driver.x_apply_default_options = activegs_apply_default_options;
    option.initOptions();
    
	self.emulatorController = [[activegsEmulatorController alloc] init ] ;    // =retain
	self.detailController = [[detailViewController alloc] init] ; // =retain
    
    

    NSString* infoXB = @"infoViewController";
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        infoXB = [infoXB stringByAppendingString:@"-ipad"];
    
    NSLog(@"Trying to load %@",infoXB);
	self.infoController = [[infoViewController alloc] initWithNibName:infoXB bundle:nil  ]; // =retain
    
	self.viewController = [[ACTIVEGS_LAUNCHVIEWCONTROLLER alloc] init  ]; // =retain


	
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(didRotate:)
												 name:@"UIDeviceOrientationDidChangeNotification" object:nil];
	
    // ajoute la détection de l"
    
    [[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(screenDidConnect:)
												 name:@"UIScreenDidConnectNotification" object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(screenDidDisconnect:)
												 name:@"UIScreenDidDisconnectNotification" object:nil];
    
    
    
#ifdef ACTIVEGS_BACKGROUNDIMAGE
	NSString* imgPath = @ACTIVEGS_BACKGROUNDIMAGE;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        imgPath = [imgPath stringByAppendingString:@"-ipad"];

	NSString *backgroundPath = [[NSBundle mainBundle] pathForResource:imgPath ofType:@"png"];
	UIImage* background = [UIImage imageWithContentsOfFile: backgroundPath] ;
    self.backgroundImageView = [[UIImageView alloc ]initWithImage:background];
	CGRect r = [UIScreen mainScreen].applicationFrame;
	
	self.backgroundView = [[UIView alloc ] initWithFrame:r];
	self.backgroundView.backgroundColor =  [UIColor whiteColor]; 
	[self.backgroundView addSubview:self.backgroundImageView];
	[self.window addSubview:self.backgroundView];
	;
#endif
	
    
    self.notificationView = [[UILabel alloc] initWithFrame:CGRectMake(8,48,200,32)];
    

    self.notificationView.alpha=0.0;
    self.notificationView.textColor =[UIColor blackColor];
    self.notificationView.backgroundColor = [UIColor grayColor];
    self.notificationView.layer.borderColor = [[UIColor whiteColor] CGColor];
    self.notificationView.layer.borderWidth = 1;
    self.notificationView.layer.cornerRadius = 5;
    self.notificationView.font = [UIFont systemFontOfSize:16];
    self.notificationView.textAlignment = UITextAlignmentCenter;
    
    [[pManager getBrowserView] updateView ];
    
    [self.window makeKeyAndVisible];
    
#ifdef HANDLE_URL
    
#   ifdef TEST_HANDLE_URL
    [self application:nil openURL:nil sourceApplication:nil annotation:nil];
    return YES;
#   else
    NSURL* url = (NSURL *)[launchOptions valueForKey:UIApplicationLaunchOptionsURLKey];
    if (url)
    {
        printf("will launch url %@",url);
        return  YES;
    }
#   endif
#endif
    
    
#ifdef ACTIVEGS_BACKGROUNDIMAGE
    
	//pour laisser le temps à ma view "splashscreen" de s'afficher
	[self performSelectorOnMainThread:@selector(firstLaunch:) withObject:nil waitUntilDone:NO];
#else

    [self firstLaunch:nil];
#endif
    
    // ready to accept option
    return NO;

    
}

-(void)firstLaunch:(NSTimer*)timer
{
	[self doStateChange:[self getBrowserView] type:kCATransitionFade sens:kCATransitionFromRight];
    
    self.primaryScreen = [UIScreen mainScreen];
    self.secondaryScreen =[UIScreen mainScreen];
    
    NSUInteger nbscreen  =[[UIScreen screens] count];
    printf("nbscreen:%d\n",nbscreen);
    if (nbscreen>1)
    {
        [self initSecondaryScreen:[[UIScreen screens]objectAtIndex:1]];
        
    }
}

-(void)launchDiskImage:(ActiveGSElement*)_element pathname:(const char*)_pathname trackername:(const char*)_trackername;
{
    
	// détruit l'émulateur pour qu'il redémarre
	if (pMac)
	{
		pMac->terminateEmulator();
		delete pMac ;
		pMac = NULL;
	}
	

	config.resetConfig(); // remets les valeur par défault
    
      
    //	config.xmlconfig = pt.c_str();
	getdir(_pathname,config.baseURL);
	config.pXML = _element->pXML;
	config.visibleName = _element->name.c_str();
	
    //is2GS = _element->is2GS;
	
	NSString* ntr = [NSString stringWithUTF8String:_trackername];
	NSString* nid = [NSString stringWithUTF8String:_element->theid.c_str()];
	
	// statistique sur les images
	NSString* track  = [NSString stringWithFormat:@"/%@/%@",ntr,nid];
    NSLog(@"Launching : %@",track);
#ifdef USE_CAPPTAIN
	[[pManager getEmulatorView] setTrackerName:track];
#endif
    
    
#ifdef USE_GOOGLEANALYTICS
	[pManager->GAITrackerActiveGS sendView:track];
#endif	
	// affichera le pad si une application le demande
	padAlreadyDisplayedOnce=0;
	
    
	// redémarre l'émulateur
	[pManager doStateChange:[pManager getEmulatorView] type:kCATransitionPush sens:kCATransitionFromRight];
}




- (void) doStateChange:(ACTIVEGS_LAUNCHVIEWCONTROLLER*)_newViewController type:(NSString*)_type sens:(NSString*)_sens
{
    debug_printf("changing view\n");
    
    if ([self.window rootViewController ] == _newViewController)
    {
        printf("same root");
        [_newViewController viewWillAppear:FALSE];
        return ;
    }

    
    // set up an animation for the transition between the views on 1 screen configuration
    if (1)// [self hasSecondary]==false || _newViewController != (UIViewController*)[pManager getEmulatorView])
    {
        
        CATransition *animation = [CATransition animation];
        [animation setDuration:0.3];
        [animation setType:_type /*kCATransitionPush*/];
        
        
        if (self.primaryViewController)
            [self.primaryViewController.view removeFromSuperview];
        else 
        {
            if (self.backgroundView)
                [self.backgroundView removeFromSuperview];
        }
        
        [self.notificationView removeFromSuperview];
        [_newViewController.view addSubview:self.notificationView];
        
        [self.window setRootViewController:_newViewController];
  
        // required for IOS4.3
         [self.window makeKeyAndVisible];
        
        
        self.primaryViewController = _newViewController;
        
        
        debug_printf("finalizing view\n");
        if (self.primaryViewController == (UIViewController*)[pManager getEmulatorView])
        {
        }
        else
        {
            // enleve toute l'interface
            [[pManager getEmulatorView].kbdc hideInput];
        }
        
        // Réajuste le sens de l'animation en fonction de l'orientation du mobile
        //UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
        switch(curOrientation)
        {
            case UIDeviceOrientationPortrait:
                break;
            case UIDeviceOrientationPortraitUpsideDown:
                if (_sens == kCATransitionFromRight)
                    _sens = kCATransitionFromLeft;
                else 
                    _sens = kCATransitionFromRight;
                break;
            case UIDeviceOrientationLandscapeLeft:
                if (_sens == kCATransitionFromRight)
                    _sens = kCATransitionFromTop;
                else 
                    _sens = kCATransitionFromBottom;
                break;
                break;
            case UIDeviceOrientationLandscapeRight:
                if (_sens != kCATransitionFromRight)
                    _sens = kCATransitionFromTop;
                else 
                    _sens = kCATransitionFromBottom;
                break;
            default:
                break;
        }		
        
        [animation setSubtype:_sens];
        [[self.window layer] addAnimation:animation forKey:@"SwitchToView1"];

    }
    /*
    else
    {
        // setting up the secondary screen
        if (self.secondaryViewController)
            [self.secondaryViewController.view removeFromSuperview];
        self.secondaryViewController = _newViewController;
        [self.secondaryWindow addSubview:_newViewController.view];
    }
     */

    
	printf("manual rotate");
	[self didRotate:nil];

}

-(void)updateNotificationView:(CGRect) newRect
{
#define NOTIFICATION_WIDTH 240.0f
#define NOTIFICATION_HEIGHT 32.0f
    
    CGRect r ;
    

    r = CGRectMake(
                   (newRect.size.width-NOTIFICATION_WIDTH)/2,
                   (newRect.size.height-NOTIFICATION_HEIGHT)/2,
                   NOTIFICATION_WIDTH,
                   NOTIFICATION_HEIGHT
                   );
    
    [self.notificationView setFrame:r];

}

-(void)setNotificationText:(NSString*) _text 
{
    
    NSLog(@"### %@",_text);
#ifdef ACTIVEGS_NOTIFICATION
    
    [self.notificationView.layer removeAllAnimations];
    [self.notificationTimer invalidate];
    
    self.notificationView.text = _text;
    
    [UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:0.3];
    self.notificationView.alpha=1.0;
    [UIView commitAnimations];
    
    self.notificationTimer = [NSTimer scheduledTimerWithTimeInterval:3.0 target:self selector:@selector(dismissNotification:) userInfo:nil repeats:NO];
#endif
    
}
-(void)dismissNotification:(id)_obj
{
    [UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:0.3];
    self.notificationView.alpha=0.0;
    [UIView commitAnimations];
    self.notificationTimer= nil;
    
}


- (void) screenDidConnect:(NSNotification *)notification
{
    NSLog(@"Screen did connect");
     UIScreen* screen = [notification object];
    [self initSecondaryScreen:screen];
    
    // revient au menu principal si on ne l'était pas
    /*
    if (self.primaryViewController == [pManager getEmulatorView])
    {
      
        [self doStateChange:[self getBrowserView] type:kCATransitionPush sens:kCATransitionFromRight];	
    }
     */
    
}

- (void) screenDidDisconnect:(NSNotification *)notification
{
    NSLog(@"screenDidDisconnect");
    /*
    UIScreen* screen = [notification object];
    if (screen == self.secondaryScreen)
     */
    activegsEmulatorController* a = [pManager getEmulatorView];
    if (a && a->attachedTo == ATTACH_SECONDARY)
    {
        NSLog(@"about to detach secondary");
        
        self.secondaryScreen = self.primaryScreen;
        self.secondaryWindow.hidden = YES;
        self.secondaryWindow;
        self.secondaryWindow = nil;
        
        [a updateView];
    }
    
    [self setNotificationText:@"Airplay Disconnected"]; 
}

-(void) initSecondaryScreen:(UIScreen*) _screen
{
    [self setNotificationText:@"Airplay Detected"];
    
#ifdef ENABLE_AIRPLAY
    self.secondaryScreen = _screen;
    CGRect frame = _screen.bounds;
    printf("SecondaryScreen size %d x %d",(int)frame.size.width,(int)frame.size.height);
    
    self.secondaryWindow = [[UIWindow alloc] initWithFrame:frame];
    [self.secondaryWindow setScreen:_screen];
    self.secondaryWindow.hidden = NO;
#endif
    activegsEmulatorController* a = [pManager getEmulatorView];
    [a updateView];
  //  [pManager   setSecondaryInputMode];
    
}

-(void)setSecondaryInputMode
{
    NSLog(@"setSecondaryInputMode");
    [[pManager getEmulatorView].kbdc setInputMode:INPUTMODE_ACCESS+INPUTMODE_KBD];
}


-(bool) hasSecondary
{
    return self.secondaryWindow != nil;
}


- (void) didRotate:(NSNotification *)notification
{	
	
    
    
	UIViewController *v = self.primaryViewController;
    if (v==nil)
        return ;
    
    
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    /*
    BOOL valid = [v shouldAutorotateToInterfaceOrientation:orientation];
    if (valid==NO)
    {
        return ;
    }
     */
    
	float oldRotate = self.currentRawReading;
    
	switch(orientation)
	{
		case UIDeviceOrientationPortrait:
			NSLog(@"UIDeviceOrientationPortrait");
			self.currentRawReading = -90;
			break;
		case UIDeviceOrientationPortraitUpsideDown:
			NSLog(@"UIDeviceOrientationPortraitUpsideDown");
			self.currentRawReading = 90;
			break;
		case UIDeviceOrientationLandscapeLeft:
			NSLog(@"UIDeviceOrientationLandscapeLeft");
			self.currentRawReading = 180;
			break;
		case UIDeviceOrientationLandscapeRight:
			NSLog(@"UIDeviceOrientationLandscapeRight");
			self.currentRawReading = 0;
			break;
		case UIDeviceOrientationUnknown:
			NSLog(@"ignoring UIDeviceOrientationUnknown");
            return;
			break;
		default:
            NSLog(@"ignoring faceUp or faceDown");
			// ignore face up or down 
			return;
			break;
	}
    curOrientation = orientation;
	NSLog(@"didRotate");

    
	if (self.primaryViewController != [pManager getEmulatorView])
	{
        if (self.primaryViewController == [pManager getBrowserView])
			[[pManager getBrowserView] updateView ];
		if (self.primaryViewController == [pManager getInfoView])
			[[pManager getInfoView] updateView:orientation];
		return ;
	}
     
   
/*
	// Gère manuellement la rotation
	
	// Mets les interfaces systèmes (UIAlert) dans le bon mode
	
	if (self.currentRawReading==0)
		[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft];
	else
		if (self.currentRawReading==90)
			[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationPortraitUpsideDown];
		else
			if (self.currentRawReading==180)
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeRight];
			else
				//	if (currentRawReading==270)
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationPortrait];
	
*/
		
	// enleve le clavier si une rotation a eu lieu
	if (oldRotate != self.currentRawReading )//&& [self hasSecondary]==false)
	{
        [[pManager getEmulatorView].kbdc restoreInput];
	}
	
//	[[pManager getEmulatorView] updateView ];
    
	
}

- (void)applicationDidEnterBackground:(UIApplication *)application 
{
	printf("applicationDidEnterBackground");
	
}
- (void)applicationWillEnterForeground:(UIApplication *)application 
{
	printf("applicationWillEnterForeground");
    
}



- (void)dealloc {
	
	if (pMac)
	{
		pMac->terminateEmulator();
		delete pMac;
		pMac = NULL;
	}
	
}


- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
//- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
   
#ifdef HANDLE_URL
    NSLog(@"Custom Url %@",url);


 //   NSString* resource = [[NSBundle mainBundle] pathForResource:@ACTIVEGS_FIRSTTABLIST ofType:@"activegsxml"] ;
 //   MyString source = [resource UTF8String];
    MyString appid;
    MyString source;
    
#ifdef TEST_HANDLE_URL
    source = "http://www.freetoolsassociation.com/fta/undergroundlist.xml";
#else
    for (NSString *param in [[url query] componentsSeparatedByString:@"&"])
    {
        NSArray *elts = [param componentsSeparatedByString:@"="];
        if([elts count] < 2) continue;
        if ([[elts objectAtIndex:0] isEqualToString:@"id"])
            appid=[ [elts objectAtIndex:1] UTF8String];
        if ([[elts objectAtIndex:0] isEqualToString:@"source"])
            source =[[elts objectAtIndex:1] UTF8String];
        
    }
#endif

if (source.IsEmpty()==FALSE)
{
    self->firstTabXML= source;
    activegsViewController* ags = [self getBrowserView];
    if (ags)
    {
        activegsList* l = [ags l0];
        if (l)
        {
            l->listPath = self->firstTabXML;
            [l reloadData:TRUE];
        }
    }
    
}
    ActiveGSElement* element = NULL;
       ActiveGSList temp;
    if (appid.IsEmpty()==FALSE)
    {
     
        temp.loadFromFile(self->firstTabXML.c_str());
        for(int i =0;i<temp.elements.size();i++)
        {
            if (temp.elements.at(i).theid == appid)
            {
                element = &temp.elements.at(i);
                break;
            }
        }
    }
    if (element)
    {
        
        [self launchDiskImage:element pathname:temp.pathName.c_str() trackername:temp.trackerName.c_str()];
        
        return TRUE;
    }
    else
        [self doStateChange:[self getBrowserView] type:kCATransitionFade sens:kCATransitionFromRight];
    
#endif
    return FALSE;
}


@end


void x_display_alert(int _abort, const char* _title, const char* _message)
{
	alertAbort = _abort;
	alertTitle = _title;
	alertMessage = _message;
	@autoreleasepool {
		[pManager performSelectorOnMainThread:@selector(displayAlert:) withObject:nil waitUntilDone:NO];
	}
    
}
