/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "activegsAppDelegate.h"
#include "../kegs/src/protos_macdriver.h"
#include "../kegs/src/sim65816.h"

#define TIME_BEFORE_REENABLING_GESTURES 1.0
#define TIME_BEFORE_DISABLING_GESTURES 0.5
#define DELAY_TO_EXECUTE_SWIPE 0.4
#define SWIPENBTOUCHES 1


#undef debug_printf
#define debug_printf(X,...) 

int	x_lock_zoom = 0;

@implementation customView

@synthesize grswipedown = _grswipedown;
@synthesize grswipeleft = _grswipeleft;
@synthesize grswiperight = _grswiperight;
@synthesize grswipeup = _grswipeup;



- (UIView *)hitTest:(CGPoint)_point withEvent:(UIEvent *)event
{
	
	KBDController *pKbd = [pManager getEmulatorView].kbdc;
	UIView* curHit;
	CGPoint p;
	
	if (!pKbd.runtimeControls.hidden && pKbd.runtimeControls.alpha!=0.0)
	{
		
		p = [self convertPoint:_point toView:pKbd.runtimeControls];
	
		curHit= [pKbd.runtimeControls hitTest:p withEvent:event];
		if ( [curHit isDescendantOfView:pKbd.runtimeControls]) 	
		{
			return curHit;
		}
	}

    
    if (!pKbd.diskSelection.hidden && pKbd.diskSelection.alpha!=0.0)
	{
		
		p = [self convertPoint:_point toView:pKbd.diskSelection];
        
		curHit= [pKbd.diskSelection hitTest:p withEvent:event];
		if ( [curHit isDescendantOfView:pKbd.diskSelection])
		{
			return curHit;
		}
	}

	
	if (!pKbd.barView.hidden)
	{

	p = [self convertPoint:_point toView:pKbd.barView];
	curHit= [pKbd.barView hitTest:p withEvent:event];
	
	if ( [curHit isDescendantOfView:pKbd.barView]) 
	{
		// debug_printf("intercepting event : barview\n");
		return curHit;
	}
	}
	
	if ([pKbd isInputVisible:(INPUTMODE_ACCESS+INPUTMODE_MINIACCESS)])
	{
		
		p = [self convertPoint:_point toView:pKbd.accessView];
		curHit= [pKbd.accessView hitTest:p withEvent:event];
			
		// regarde si on tape dans la barview
		if ( !curHit.hidden && [curHit isDescendantOfView:pKbd.accessView]) 
		{

			return curHit;
		}
	}

	
	// renvoie les évenements à l'emulator
	return [pManager getEmulatorView].zv;
}

- (id)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame]) 
	{		

		// Swipe down to hide keyboard	
		self.grswipeup = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeUp:)];
		self.grswipeup.direction  = UISwipeGestureRecognizerDirectionUp ;
		self.grswipeup.cancelsTouchesInView = NO;
		self.grswipeup.numberOfTouchesRequired = SWIPENBTOUCHES;
		[self addGestureRecognizer:self.grswipeup];
		
		
		// Swipe up to display keyboard	
		self.grswipedown = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeDown:)];
		self.grswipedown.direction  = UISwipeGestureRecognizerDirectionDown;
		self.grswipedown.cancelsTouchesInView = NO;
		self.grswipedown.numberOfTouchesRequired = SWIPENBTOUCHES;
		[self addGestureRecognizer:self.grswipedown];
        
		// Swipe left to go back to browsing	
		self.grswipeleft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeLeft:)];
		self.grswipeleft.direction  = UISwipeGestureRecognizerDirectionLeft;
		self.grswipeleft.cancelsTouchesInView = NO;
		self.grswipeleft.numberOfTouchesRequired = SWIPENBTOUCHES;
		[self addGestureRecognizer:self.grswipeleft];
		
		
		// Swipe Right to go back to browsing	
		self.grswiperight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeRight:)];
		self.grswiperight.direction  = UISwipeGestureRecognizerDirectionRight;
		self.grswiperight.cancelsTouchesInView = NO;
		self.grswiperight.numberOfTouchesRequired = SWIPENBTOUCHES;
		[self addGestureRecognizer:self.grswiperight];

		
	}
	return self;
}



-(void)invalidateTimers
{

	
	if ([enableTimer isValid])
	{
		debug_printf("deactivating enable gesture timer");
		[enableTimer invalidate];
		enableTimer=nil;
	}
	
	if ([disableTimer isValid])
	{
		debug_printf("deactivating disable gesture timer");
		[disableTimer invalidate];
		disableTimer=nil;
	}

}

-(void)scheduleDisableAllGestures
{

	debug_printf("scheduleDisableAllGestures");
	
	[self invalidateTimers];
		
	disableTimer = [NSTimer scheduledTimerWithTimeInterval:TIME_BEFORE_DISABLING_GESTURES target:self selector:@selector(disableAllGestures) userInfo:nil repeats:NO];
	[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:MODE_ALL color:[UIColor blueColor]];

}


-(void)scheduleEnableAllGestures
{

	debug_printf("scheduleEnableAllGestures");
	
	
	[self enableGestures:MODE_PAD];
	[self disableGestures:MODE_SWIPE+MODE_ZOOM];
	
	enableTimer = [NSTimer scheduledTimerWithTimeInterval:TIME_BEFORE_REENABLING_GESTURES target:self selector:@selector(reenableAllGestures) userInfo:nil repeats:NO];	
	[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:(MODE_SWIPE+MODE_ZOOM) color:[UIColor orangeColor]];

}

-(void)disableAllGestures
{

	debug_printf("disableAllGestures\n");
	[self disableGestures:MODE_ALL];

}

-(void)disableGestures:(int)_mode
{

	debug_printf("disableGestures: %d",_mode);
	
	[self invalidateTimers];	

	if (_mode & MODE_SWIPE)
	{
		
		self.grswipeup.enabled = FALSE;
		self.grswipedown.enabled = FALSE;
		self.grswipeleft.enabled = FALSE;
		self.grswiperight.enabled = FALSE;
	}
	if (_mode & MODE_PAD)
		[[pManager getEmulatorView].kbdc enablePad:FALSE];
	
	if (_mode & MODE_ZOOM)
		[[pManager getEmulatorView].zv disableZoom];
	
	if (_mode & MODE_EMULATOR)
		[[pManager getEmulatorView].zv disableInput];
	
	[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:_mode color:[UIColor redColor]];

}



-(void)enableGestures:(int)_mode
{

	debug_printf("enableGestures %d\n",_mode);
	
	if (_mode & MODE_SWIPE)
	{
		[self invalidateTimers];
		
		self.grswipeup.enabled = TRUE;
		self.grswipedown.enabled = TRUE;
		self.grswipeleft.enabled = TRUE;
		self.grswiperight.enabled = TRUE;
	}

	if (_mode & MODE_ZOOM)
	{
		if (x_lock_zoom)
		{
		//	[self disableGestures:MODE_ZOOM];
			[[pManager getEmulatorView].zv disableZoom];
		}
		else
			[[pManager getEmulatorView].zv enableZoom];
	}
	
	if (_mode & MODE_PAD)
		[[pManager getEmulatorView].kbdc enablePad:TRUE];
	if (_mode & MODE_EMULATOR)
		[[pManager getEmulatorView].zv enableInput];
	
	[[pManager getEmulatorView].kbdc setGestureIndicatorVisibility:_mode color:[UIColor greenColor]];

}

-(void)reenableAllGestures
{

	debug_printf("reenableAllGestures");
	[self enableGestures:(MODE_ALL+MODE_EMULATOR)];

}


-(void)processGesturesOnTouchesEnded
{
#if SWIPENBTOUCHES == 1
	debug_printf("processGesturesOnTouchesEnded");
	
	// Execute les gestures en attente
	double curtime =  [[NSDate date] timeIntervalSince1970];
	
	if (possibleSwipeLeftTime  > curtime)
	{

		[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromRight];

		return ;
	}
	
	if (possibleSwipeRightTime  > curtime)
	{
		if ([pManager getDetailView]->diskSelected)
			[pManager doStateChange:[pManager getDetailView] type:kCATransitionPush sens:kCATransitionFromLeft];
		else
        {
			[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromLeft];
		}
		return ;
	}	
	
	if (possibleSwipeDownTime  > curtime)
	{
		[[pManager getEmulatorView].kbdc swipeDown];
		// We remain on the currentView: immediatly turn on gestures
		[self reenableAllGestures];
		return ;
	}
	
	if (possibleSwipeUpTime > curtime)
	{
		
		[[pManager getEmulatorView].kbdc swipeUp];
		// We remain on the currentView: immediatly turn on gestures
		[self reenableAllGestures];
		return ;
	}
#endif
	// no gestures
	[self scheduleEnableAllGestures];
	[self enableGestures:MODE_ZOOM]; // pour éviter l'interprétation en click!
}

-(void)swipeLeft :(UISwipeGestureRecognizer*)_rec
{
	if (_rec.state == UIGestureRecognizerStateEnded) 
	{

#if SWIPENBTOUCHES == 2
		[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromRight];

#else
		if ([_rec locationInView:self].y < 44 * [pManager resolutionRatio])
		{
			possibleSwipeLeftTime = [[NSDate date] timeIntervalSince1970] + DELAY_TO_EXECUTE_SWIPE;
			debug_printf("possibleSwipeLeftTime %f\n",possibleSwipeLeftTime);
		}
#endif
	}
}

-(void)swipeRight :(UISwipeGestureRecognizer*)_rec
{
	if (_rec.state == UIGestureRecognizerStateEnded)
	{
#if SWIPENBTOUCHES == 2
		if ([pManager getDetailView].diskSelected)
			[pManager doStateChange:[pManager getDetailView] type:kCATransitionPush sens:kCATransitionFromLeft];
		else 
			[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromLeft];
#else

		if ([_rec locationInView:self].y < 44 * [pManager resolutionRatio])
		{
			possibleSwipeRightTime = [[NSDate date] timeIntervalSince1970] + DELAY_TO_EXECUTE_SWIPE;
			debug_printf("possibleSwipeRightTime %f\n",possibleSwipeRightTime);
		}
#endif
	}

}


-(void)swipeDown : (UISwipeGestureRecognizer*)_rec
{
	debug_printf("swipeDown : %d",_rec.state);
	
	if (_rec.state == UIGestureRecognizerStateEnded)
	{

#if SWIPENBTOUCHES == 2
		[[pManager getEmulatorView].kbdc swipeDown];
#else
		 possibleSwipeDownTime = [[NSDate date] timeIntervalSince1970] + DELAY_TO_EXECUTE_SWIPE;
		debug_printf("possibleSwipeDownTime %f\n",possibleSwipeDownTime);
#endif
	}
	
}

-(void)swipeUp : (UISwipeGestureRecognizer*)_rec
{
	debug_printf("swipeUp : %d",_rec.state);
	
	if (_rec.state == UIGestureRecognizerStateEnded)
	{
#if SWIPENBTOUCHES == 2
		[[pManager getEmulatorView].kbdc swipeUp];
#else
		possibleSwipeUpTime = [[NSDate date] timeIntervalSince1970] + DELAY_TO_EXECUTE_SWIPE;
		debug_printf("possibleSwipeUpTime %f\n",possibleSwipeUpTime);
#endif
	}
		
}


@end

@implementation activegsEmulatorController


#ifdef USE_CAPPTAIN
-(NSString*)getCapptainActivityName
{
	NSLog(@"loggin capptain:%@",_trackerName);
	return _trackerName;
}
#endif

- (void)createZoomEmulatorView
{
   
    CGRect rv = CGRectMake(0,0,100.0,100.0);
    self.zv = [[zoomEmulatorView alloc] initWithFrame:rv ratio:1.0];

}

- (void)loadView 
{

    attachedTo = ATTACH_NONE;
    
	CGRect apprect = [[UIScreen mainScreen] applicationFrame];
    printf("mainScreen apprect %d x %d\n",(int)apprect.size.width,(int)apprect.size.height);

	self.contentView = [[customView alloc] initWithFrame:apprect];
#ifdef SHOW_COLOR
	self.contentView.backgroundColor = [UIColor yellowColor];
#else
    self.contentView.backgroundColor = [UIColor blackColor];
#endif
	
	
    [self createZoomEmulatorView];
    
    [self.zv setUserInteractionEnabled:TRUE];

	self.kbdc = [KBDController alloc]; 
	[self.contentView addSubview:self.kbdc.view];
	
	self.view = self.contentView;
 

    [self.zv disableZoom];
}

-(void) updateView
{
	static int inUpdateView=0;
    if (inUpdateView) 
        return ;
    inUpdateView++;
	debug_printf("viewController updateView\n");
	
	
	// Transforme la fenetre root view

	[UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:0.3];

	CGAffineTransform matv = CGAffineTransformIdentity;
//	matv = CGAffineTransformRotate (matv,(270-[pManager getAngle])*M_PI/180);	//Avec
	self.contentView.transform = matv;
	
    
	    
    CGRect emulatorRect; // emulator rect
    CGRect boundRect; // emulator rect rotate
    int     requiredOrientation = -1 ;
    CGRect kbddelta = CGRectZero;
    
	CGAffineTransform mat = CGAffineTransformIdentity;
#ifdef VIDEO_SINGLEVLINE
	mat = CGAffineTransformScale(mat,1,-2);
#else
	mat = CGAffineTransformScale(mat,1,-1);

#endif
	self.zv.transform = mat;
	
	// Mets à jour la taille de la fenetre en fonction de l'affichage du clavier
    if ([pManager hasSecondary ])
    {
        CGRect secondaryapprect = [pManager secondaryScreen].bounds;
        emulatorRect = CGRectMake(0.0,0.0,secondaryapprect.size.width,secondaryapprect.size.height);
        
        if (attachedTo != ATTACH_SECONDARY )
        {
            printf("attaching to secondary...");
            
            attachedTo = ATTACH_SECONDARY;
            
            [self.zv removeFromSuperview];
            [pManager.secondaryWindow addSubview:self.zv];
            
            float ratio = [[pManager secondaryScreen] scale];
        
              // recalcule la vue car la résolution/ratio a changé
            [self.zv updateScreen:emulatorRect ratio:ratio];
            
            [self.kbdc setMenuBarVisibility:true];

            [pManager   setSecondaryInputMode];
        }
        
        // Force L'affichage par défault
        requiredOrientation = 0;
        boundRect = emulatorRect;
    }
    else
    {
        CGRect primaryapprect = [UIScreen mainScreen].bounds;
        emulatorRect = CGRectMake(0.0,0.0,primaryapprect.size.width,primaryapprect.size.height);
        NSLog(@"emulatorRect : %f*%f\n",emulatorRect.size.width,emulatorRect.size.height);
        if (attachedTo != ATTACH_PRIMARY )
        {
            printf("attaching to primay...");
            attachedTo = ATTACH_PRIMARY;
            
            [self.zv removeFromSuperview];
            
            
            float ratio = [[UIScreen mainScreen] scale];

            // recalcule la vue car la résolution/ratio a changé
            [self.zv updateScreen:emulatorRect ratio:ratio];
        
            [self.contentView addSubview:self.zv];
            
            // fait apparaite l'inputr
            [self.kbdc restoreInput];
            
            // le remts en premier
            [self.contentView bringSubviewToFront:self.kbdc.view];
            
                       
            
        }
        
        // primary parameters
        
        requiredOrientation = ([pManager getAngle]/90)+1;
        kbddelta =  [self.kbdc getEmulatorDelta];
        
        NSString* systemVer = [[UIDevice currentDevice] systemVersion];
        if ([systemVer compare:@"8.0"] == NSOrderedDescending)
            boundRect = emulatorRect;
        else
        {
            boundRect = CGRectApplyAffineTransform(emulatorRect,matv);
            boundRect.origin.x = 0;
            boundRect.origin.y = 0;
        }
    
        [self.contentView setBounds:boundRect];
    }
	
 
    
    [self.zv updateFrame: boundRect kbdRect:kbddelta];
    [self.zv switchOrientation:requiredOrientation]; // a faire après le boundRect
    
    printf("*** setbound rect %f,%f\n",boundRect.size.width,boundRect.size.height);

	[UIView commitAnimations];

	
	// l'interface est repositionnées mais non animée
    
    CGRect uirectrotate = [[UIScreen mainScreen] applicationFrame];
	[self.kbdc updateView:uirectrotate];
    [pManager updateNotificationView:uirectrotate];
    
     inUpdateView--;
}


// IOS 5 Compatibility

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (BOOL)shouldAutorotate
{
    return YES;
}

// IOS 8
- (NSUInteger)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskAll;
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    [coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        [self updateView];
        
    } completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        
    }];
}



- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


-(void)viewWillAppear:(BOOL)animated
{
    printf("view will appear");
	[[pManager getEmulatorView].contentView reenableAllGestures];
	
	[[pManager getEmulatorView].kbdc setEmulatorTitle:config.visibleName.c_str()];

    if ([pManager hasSecondary])
    {
        [pManager setSecondaryInputMode];
    }
    
	if (pMac)
	{
        // réactive l'interface
        if ([pManager hasSecondary])
        {
        //    [[pManager getEmulatorView].kbdc restoreInput];
        }
        else
        {
            debug_printf("### unpausing emulator");
            r_sim65816.resume();
        }
	}
	else 
	{
		debug_printf("### starting new emulator instance");


		[[pManager getEmulatorView].kbdc resetSpecialKeys];
		pMac = new CEmulatorMac(NULL,1);
		config.xmlAlreadyLoaded=0;
		pMac->setConfig(&config);
		pMac->launchEmulator();
		
		// remets un ratio de 1.0
		[self.zv setRatioToOne];
        [self.zv switchOrientation:self.zv->curOrientation];
	}
     	
	[self.zv.ew activateEmulatorRefresh:option.getIntValue(OPTION_FRAMERATE)];

}

-(void)viewWillDisappear:(BOOL)animated
{
	
	// elimine les gestures (et les timers)
	[[pManager getEmulatorView].contentView disableGestures:MODE_ALL];
	if (pMac)
	{
        if ([pManager hasSecondary]==false)
        {
            debug_printf("### pausing emulator");
            r_sim65816.pause();	
        }
	}
	[self.zv.ew deactivateEmulatorRefresh];
}

- (void)dealloc {

    self.zv;
    self.zv = nil;

}

@end
