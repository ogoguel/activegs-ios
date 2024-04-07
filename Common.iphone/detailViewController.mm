/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "activegsAppDelegate.h"
#import "detailViewController.h"
#import "activegsList.h"
#include "../Common/svnversion.h"
#import "asyncimageview.h"
#import "activegsViewController.h"


@implementation detailViewController

@synthesize diskName=_diskName;
@synthesize diskCompany = _diskCompany;
@synthesize diskYear=_diskYear;
@synthesize diskDescription = _diskDescription;
@synthesize diskApple2 = _diskApple2;
@synthesize diskApple2GS = _diskApple2GS;
@synthesize diskWebView = _diskWebView;
@synthesize screenShot1 = _screenShot1;
@synthesize screenShot2 = _screenShot2;
@synthesize screenShot3 = _screenShot3;
@synthesize navView = _navView;
@synthesize navItem = _navItem;
/*
@synthesize diskSelected = _diskSelected;
@synthesize list = _list;
  */

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
	
	requestURL = [ request URL ];
	
    // Check to see what protocol/scheme the requested URL is.
    if ( ( [ [ requestURL scheme ] isEqualToString: @"http" ]
		  || [ [ requestURL scheme ] isEqualToString: @"https" ] )
    	&& ( navigationType == UIWebViewNavigationTypeLinkClicked ) ) {
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"iPhone" message:@"Link will open in Safari, continue?" delegate:self cancelButtonTitle:@"Yes" otherButtonTitles:@"No",nil];
		[alert show];
		return NO;
   	}
   
    return YES;
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if (buttonIndex==0)
		[[ UIApplication sharedApplication ] openURL: requestURL ];

}

- (void)viewWillAppear:(BOOL)animated
{
	
	printf("settitle2:%s\n",self->diskSelected->name.c_str());

	[self.navItem setTitle:[NSString  stringWithUTF8String: self->diskSelected->name.c_str()]];
	self.diskName.text = [NSString  stringWithUTF8String: self->diskSelected->name.c_str()];
	self.diskCompany.text = [NSString  stringWithUTF8String: self->diskSelected->publisher.c_str()];
	NSString* desc = [NSString  stringWithUTF8String: self->diskSelected->desc.c_str()];
	float rr = [pManager resolutionRatio];
	float fs1 = .9 * rr;
	float fs2 = 1.5 ;//* rr;
	NSString* html = [NSString stringWithFormat:@"<HTML><head><style type=\"text/css\">p { font: %fem/%fem helvetica; text-align: justify; } </style></head><body><div id='ContentDiv'><p>%@</p></div></body></HTML>",fs1,fs2,desc];

	[self.diskWebView loadHTMLString:html  baseURL:nil];
	
	self.diskYear.text = [NSString  stringWithUTF8String: self->diskSelected->year.c_str()];
	
#ifdef ACTIVEGS_NOAPPLEBRANDING
	self.diskApple2.hidden = TRUE;
	self.diskApple2GS.hidden = TRUE;
#else
	self.diskApple2.hidden = (self->diskSelected->is2GS);
	self.diskApple2GS.hidden = !(self->diskSelected->is2GS);
#endif
	self.screenShot1.image = nil;
	self.screenShot2.image = nil;
	self.screenShot3.image = nil;
	
	AsyncImageView* asyncImage1 = [[AsyncImageView alloc] init];
	[asyncImage1 initImage:self->diskSelected->screenShots[0].c_str() target:self.screenShot1 width:128 height:80];
	[asyncImage1 loadImage:nil];
	
	AsyncImageView* asyncImage2 = [[AsyncImageView alloc] init];
	[asyncImage2 initImage:self->diskSelected->screenShots[1].c_str() target:self.screenShot2 width:128 height:80];
	[asyncImage2 loadImage:nil];
	
	AsyncImageView* asyncImage3 = [[AsyncImageView alloc] init] ;
	[asyncImage3 initImage:self->diskSelected->screenShots[2].c_str() target:self.screenShot3 width:128 height:80];
	[asyncImage3 loadImage:nil];
	
	self.diskWebView.backgroundColor = [UIColor whiteColor];
	
	
}

- (void)backButton:(id)sender {
	
	[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromLeft];
	
}

- (void)launchButton
{
	
	[pManager launchDiskImage:self->diskSelected pathname:self->list->pathName trackername:self->list->trackerName];

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
//        [self updateView];
        
    } completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        
    }];
}




-(void)viewDidLoad {


	CGRect r = [UIScreen mainScreen].applicationFrame;
	self.navItem = [[UINavigationItem alloc] initWithTitle:@"Detail"];
	[self.navView pushNavigationItem:self.navItem animated:FALSE];
	
	[self.view setFrame:r];
		
	// Rectangle
#if 0
	UIBarButtonItem *browseItem = [[UIBarButtonItem alloc] initWithTitle:@"DETAIL" style:UIBarButtonItemStyleBordered
																  target:self
																  action:@selector(backButton:)];
#else
	// left Arrow
	UIButton* backButton = [UIButton buttonWithType:(UIButtonType)101]; // left-pointing shape!
	[backButton addTarget:self action:@selector(backButton:) forControlEvents:UIControlEventTouchUpInside];
	[backButton setTitle:@"Back" forState:UIControlStateNormal];
	
	// create button item -- possible because UIButton subclasses UIView!
	UIBarButtonItem* browseItem = [[UIBarButtonItem alloc] initWithCustomView:backButton];
#endif
	
	self.navItem.leftBarButtonItem = browseItem;

	// Swipe left to go back to browsing	
	UISwipeGestureRecognizer* grswipeleft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeLeft:)] ;
	grswipeleft.direction  = UISwipeGestureRecognizerDirectionLeft;
	grswipeleft.cancelsTouchesInView = NO;
	[self.view addGestureRecognizer:grswipeleft];
	
	
	// Swipe Right to launch	
	UISwipeGestureRecognizer* grswiperight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeRight:)] ;
	grswiperight.direction  = UISwipeGestureRecognizerDirectionRight;
	grswiperight.cancelsTouchesInView = NO;
	[self.view addGestureRecognizer:grswiperight];

	
}

-(void)swipeRight :(UISwipeGestureRecognizer*)_rec
{

	if (_rec.state == UIGestureRecognizerStateEnded)
	{
		[pManager doStateChange:[pManager getBrowserView] type:kCATransitionPush sens:kCATransitionFromLeft];
	}
}

-(void)swipeLeft :(UISwipeGestureRecognizer*)_rec
{
	if (_rec.state == UIGestureRecognizerStateEnded)
	{
		[pManager doStateChange:[pManager getEmulatorView] type:kCATransitionPush sens:kCATransitionFromRight];
	}
}


- (IBAction) launchButton:(id)_sender
{
    
	[pManager launchDiskImage:self->diskSelected pathname:self->list->pathName.c_str() trackername:self->list->trackerName.c_str()];
}

@end
