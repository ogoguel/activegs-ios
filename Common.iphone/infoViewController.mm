/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "activegsAppDelegate.h"
#import "infoViewController.h"
#import "activegsList.h"
#include "../Common/svnversion.h"
#import "asyncimageview.h"
#import "activegsViewController.h"

@implementation infoViewController

@synthesize versionUI=_versionIU;
@synthesize portraitView= _portraitView;

-(void)viewDidLoad
{

    NSString *appVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    NSString *buildVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
    
    
    self.versionUI.text = [NSString stringWithFormat: @"v%@.%@  -- ActiveGS "TOSTRING(ACTIVEGSMAJOR)"."TOSTRING(ACTIVEGSMINOR)"."TOSTRING(ACTIVEGSBUILD)"",appVersion,buildVersion];
    
    // [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
}

-(void)viewDidAppear:(BOOL)animated
{
	printf("viewDidAppear");
    [super viewDidAppear:animated];

	// Swipe left to go back to browsing	
	UISwipeGestureRecognizer* grswipeleft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeLeft:)];
	grswipeleft.direction  = UISwipeGestureRecognizerDirectionLeft;
	grswipeleft.cancelsTouchesInView = NO;
	[self.view addGestureRecognizer:grswipeleft];
	
	// Swipe Right to go back to browsing	
	UISwipeGestureRecognizer* grswiperight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeRight:)];
	grswiperight.direction  = UISwipeGestureRecognizerDirectionRight;
	grswiperight.cancelsTouchesInView = NO;
	[self.view addGestureRecognizer:grswiperight];

    [self.instructionsView flashScrollIndicators];
}


-(void)swipeLeft :(UISwipeGestureRecognizer*)_rec
{

	if (_rec.state == UIGestureRecognizerStateEnded)
	{
		[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromRight];
	}
}

-(void)swipeRight :(UISwipeGestureRecognizer*)_rec
{
	if (_rec.state == UIGestureRecognizerStateEnded)
	{
		[pManager doStateChange:[pManager getBrowserView]  type:kCATransitionPush sens:kCATransitionFromLeft];
	}
}

- (IBAction) doneButton:(id)_sender;
{
	[pManager doStateChange:[pManager getBrowserView] type:kCATransitionFade sens:kCATransitionFromLeft];

}

- (IBAction) emailButton:(id)_sender;
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.facebook.com/groups/187271854617450/"]];
	
}

- (IBAction) parametersButton:(id)_sender
{
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"ActiveGS" message:@"Not implemented yet" delegate:self cancelButtonTitle:@"Done" otherButtonTitles:nil];
	[alert show];	

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

@end
