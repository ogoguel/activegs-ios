/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "activegsAppDelegate.h"
#import "activegsViewController.h"
#import "activegsList.h"
#include "../kegs/Src/sim65816.h"


@implementation activegsViewController  
@synthesize navItem = _navItem;
@synthesize navView = _navView;
@synthesize docList = _docList;
@synthesize l0 = _l0;
@synthesize loader = _loader;
@synthesize tabView = _tabView;


-(void)viewWillAppear:(BOOL)animated
{
	NSLog(@"viewWillAppear -- activegsViewController");
	[self updateView];
}


-(void)activateLoader:(BOOL)_on
{
	if (_on)
	{
		[self.loader startAnimating];
        self.loader.hidden=false;
        
	}
	else
	{
		[self.loader stopAnimating];
        self.loader.hidden=true; 
	}
    
}

/*
- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController
*/

- (void)UITabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController 
{
	
	activegsList* l = (activegsList*)viewController;
#ifndef   ACTIVEGS_BACKGROUNDIMAGE
	if (l.sourceName)
		[(UIButton*)self.navItem.titleView setTitle:l.sourceName forState:UIControlStateNormal];
#endif
	[self updateView];
#ifdef ACTIVEGS_SAVELASTTAB
	// sauve la config
	option.setIntValue(OPTION_LASTTAB,tabBarController.selectedIndex);
	option.saveOptions(1);
	#endif
}


//#define WHEELSIZEREF 100
//float WHEELSIZE = WHEELSIZEREF ;

- (void)loadView
{
	NSLog(@"activeGSViewController -- loadView");
    
	[super loadView];
	
	CGRect r = [UIScreen mainScreen].applicationFrame;

#if 0
	float s = [pManager resolutionRatio];
	WHEELSIZE = WHEELSIZEREF*s;
	CGRect rl = CGRectMake((self.view.frame.size.width - WHEELSIZE)/2,(self.view.frame.size.height -WHEELSIZE)/2,WHEELSIZE,WHEELSIZE);
    self.loader = [[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray] autorelease];
    
       [self.loader.layer setValue:[NSNumber numberWithFloat:2.0f*s] forKeyPath:@"transform.scale"];
    	self.loader = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
	
    	[self.loader setFrame:rl];
	
#endif
    	self.loader = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
    
    [self.view addSubview:self.loader];
    
    [self.loader startAnimating];
    self.loader.hidden=false;
    
    int safeY = 0;
	
    if (@available( iOS 11.0, * )) {
        if ([[[UIApplication sharedApplication] keyWindow] safeAreaInsets].bottom > 0) {
            // iPhone with notch
            safeY =[[[UIApplication sharedApplication] keyWindow] safeAreaInsets].bottom;
        }
    }
	
	
	self.navView = [[UINavigationBar alloc] initWithFrame:CGRectMake(0, safeY, r.size.width, BARVIEW_HEIGHT)];
	self.navView.barStyle = UIBarStyleBlack;
 //   self.navView.delegate = self;
	self.navItem = [[UINavigationItem alloc] initWithTitle:@ACTIVEGS_TITLE];
    
#ifdef ACTIVEGS_BACKGROUNDIMAGE
	[self.view addSubview:pManager.backgroundView];
	UILabel* label = [[[UILabel alloc]initWithFrame:CGRectMake(0,0,r.size.width, BARVIEW_HEIGHT)] autorelease];
	label.text = @ACTIVEGS_TITLE;
	label.font =  [UIFont systemFontOfSize:(CGFloat)16.0];
	label.backgroundColor = [UIColor clearColor];
	label.textColor = [UIColor whiteColor];
	label.textAlignment = UITextAlignmentCenter;
	
	self.navItem.titleView=label;
#else
	UIButton* titleButton = [UIButton buttonWithType:(UIButtonType)100]; 
	[titleButton addTarget:self action:@selector(titleButton:) forControlEvents:UIControlEventTouchUpInside];
	[titleButton setTitle:@ACTIVEGS_TITLE forState:UIControlStateNormal];
	self.navItem.titleView = titleButton;
	self.navItem.titleView.hidden=YES;
#endif
	[self.navView pushNavigationItem:self.navItem animated:FALSE];
	
	// Rectangle
#if 0
	UIBarButtonItem *browseItem = [[[UIBarButtonItem alloc] initWithTitle:@"Back" style:UIBarButtonItemStyleBordered
																  target:self
																  action:@selector(backButton:)] autorelease];
#else
	// left Arrow
	UIButton* backButton = [UIButton buttonWithType:(UIButtonType)101]; // left-pointing shape!
	[backButton addTarget:self action:@selector(backButton:) forControlEvents:UIControlEventTouchUpInside];
	[backButton setTitle:@"Back" forState:UIControlStateNormal];
	
	// create button item -- possible because UIButton subclasses UIView!
	UIBarButtonItem* browseItem = [[UIBarButtonItem alloc] initWithCustomView:backButton];
#endif
	
	self.navItem.leftBarButtonItem = browseItem;
	
	UIButton* infoButton = [UIButton buttonWithType:UIButtonTypeInfoLight]; 
	[infoButton addTarget:self action:@selector(infoButton:) forControlEvents:UIControlEventTouchUpInside];
	UIBarButtonItem* infoItem = [[UIBarButtonItem alloc] initWithCustomView:infoButton];
	
	self.navItem.rightBarButtonItem = infoItem;
	
	[self.view addSubview:self.navView];
    
    r.origin.y = self.navView.frame.size.height + self.navView.frame.origin.y;
	r.size.height -= r.origin.y;
    
    self.tabView = [[UITabBarController alloc] init];
	[self.tabView.view setFrame:r];
	self.tabView.delegate = self;
	NSString *imgSource;
    
	self.l0 = [[activegsList alloc] init];
	//imgSource = [[NSBundle mainBundle] pathForResource:@ACTIVEGS_FIRSTTABLIST ofType:@"activegsxml"] ;
	self.l0->listPath = pManager->firstTabXML.c_str();
	self.l0->filter= ALL_NO_INDEX ;
	self.l0->trackerName = ACTIVEGS_FIRSTTABTRACKER;
	imgSource = [[NSBundle mainBundle] pathForResource:@ACTIVEGS_FIRSTTABICON ofType:@"png"];
	self.l0.tabBarItem = [[UITabBarItem alloc] initWithTitle:@ACTIVEGS_FIRSTTABTITLE image:[UIImage imageWithContentsOfFile: imgSource] tag:0];
	
#ifdef ACTIVEGS_SINGLETAB
    [self.l0.view setFrame:r];
	[self.view addSubview:self.l0.view];	
#else
	activegsList* l3 = [[activegsList alloc] init];
	l3->listPath = "http://www.freetoolsassociation.com/xml/list.activegsxml";
    l3->versionPath = "http://www.freetoolsassociation.com/xml/version.dat";
	l3->filter= ALL; 
	l3.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"FTA Website" image:[UIImage imageWithContentsOfFile: imgSource] tag:0]; ;
	l3->trackerName = "FTA";
    
	activegsList* l1 = [[activegsList alloc] init];
	l1->listPath = "http://www.virtualapple.org/xmlfiles/list.zip";
    l1->versionPath = "http://www.virtualapple.org/xmlfiles/version.dat";
	l1->filter = ONLY_APPLE2 ;
	l1->trackerName = "VirtualApple2";
	/*NSString */imgSource = [[NSBundle mainBundle] pathForResource:@"Tab][" ofType:@"png"];
	l1.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"Apple II" image:[UIImage imageWithContentsOfFile: imgSource] tag:0];
	
	activegsList* l2 = [[activegsList alloc] init];
	l2->listPath =  "http://www.virtualapple.org/xmlfiles/list.zip";;
    //	l2.title = @"Apple IIGS";
	l2->filter = ONLY_2GS;
	l2->trackerName = "VirtualApple2GS";
	imgSource = [[NSBundle mainBundle] pathForResource:@"Tab2GS" ofType:@"png"];
	l2.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"Apple IIGS" image:[UIImage imageWithContentsOfFile: imgSource] tag:0];
    
#ifndef ACTIVEGS_ENABLE_DOCUMENT
	self.docList=  nil;
#else
	self.docList = [[activegsList alloc] init];
	self.docList->listPath.Empty();
	self.docList->filter = (int)(ALL_NO_INDEX|DOC_LIST);
	self.docList->trackerName = "My2GS";	
	//docList.tabBarItem = [[[UITabBarItem alloc] initWithTitle:@"My 2GS" image:[UIImage imageWithContentsOfFile: imgSource] tag:0] autorelease];
	self.docList.tabBarItem = [[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemFavorites tag:0];
#endif
    
	[self.tabView setViewControllers:[NSArray arrayWithObjects:self.l0, l3, l1, l2, self.docList,nil]];
	option.loadOptions();
	self.tabView.selectedIndex=option.getIntValue(OPTION_LASTTAB);
    
	[self.view addSubview:self.tabView.view];
#endif // SINGLETAB
	
    
        // Swipe left to go back to browsing
        UISwipeGestureRecognizer* grswipeleft = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeLeft:)];
        grswipeleft.direction  = UISwipeGestureRecognizerDirectionLeft;
        [self.view addGestureRecognizer:grswipeleft];
        
        // Swipe right to go back to browsing	
        UISwipeGestureRecognizer* grswiperight = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeRight:)];
        grswiperight.direction  = UISwipeGestureRecognizerDirectionRight;
        [self.view addGestureRecognizer:grswiperight];
	
	// pour etre sur qu'il soit visible
	[self.view bringSubviewToFront:self.loader];
    
	
}

-(void)viewDidUnload
{
	
    self.tabView.delegate = nil;
	self.tabView = nil;
	self.navItem = nil;
	self.navView = nil;
	self.loader = nil;
	
	[super viewDidUnload];
}


-(void)updateDialog:(NSString*) newVersion
{
    
	activegsList* l = (activegsList*) self.tabView.selectedViewController;
    NSString* fmt  = [NSString stringWithFormat:@"Database version v%@ is available!\n(current installed version is v%@)\n\nDo you want to update ?",newVersion,l.sourceRevision];
   
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:l.sourceName message:fmt delegate:self cancelButtonTitle:@"Later" otherButtonTitles:@"Update Now",nil];
	[alert show];
}

- (void)titleButton:(id)sender
{
	
	activegsList* l = (activegsList*) self.tabView.selectedViewController;
	NSString* fmt = @"No Database present";
	NSString* title = @"ActiveGS";
	if (l.sourceName) 
	{
        fmt  = [NSString stringWithFormat:@"Database v%@\n\nPress Refresh to reload the database.",l.sourceRevision];
        title = l.sourceName;
	}
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:title message:fmt delegate:self cancelButtonTitle:@"OK" otherButtonTitles:@"Refresh",nil];
	[alert show];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if (buttonIndex==1)
	{
        
		activegsList* l = (activegsList*) self.tabView.selectedViewController;
		if (!l) l=self.l0; // BestOfFTA
        
		// Force redownloading of asset
		[l reloadData:YES];
		
        
	}
	
}


-(void)updateView
{
    
	CGRect rv2 = [[UIScreen mainScreen] applicationFrame];
	
	// Ajuste la barre de navigation
	CGRect rc = self.navView.frame; 
	rc.size.width  = rv2.size.width;
	[self.navView setFrame:rc];
	
	// Ajuste la position du loader
    CGRect r = 	self.loader.frame;
	r = CGRectMake((rv2.size.width - r.size.width)/2,(rv2.size.height -r.size.height)/2,r.size.width,r.size.height);
	[self.loader setFrame:r];
    
#ifdef ACTIVEGS_ENABLE_DOCUMENT
	// ajuste la position du warning
	if (self.docList.warningLabel)
	{
		CGSize s = CGSizeMake(rv2.size.width,rv2.size.height-44.0*2);
		CGRect r = CGRectMake((s.width-LABELWIDTH)/2,(s.height-LABELHEIGHT)/2,LABELWIDTH,LABELHEIGHT);
		self.docList.warningLabel.frame = r;
		[self.docList.warningLabel.superview bringSubviewToFront:self.docList.warningLabel];
	}
#else
	// ajuste la position du background
	CGRect rb = pManager.backgroundImageView.frame;
	rb.origin.x = (rv2.size.width - rb.size.width)/2;
	rb.origin.y = (rv2.size.height - rb.size.height)/2;
	pManager.backgroundImageView.frame = rb ;
	pManager.backgroundView.frame = rv2;
#endif
    
    [pManager updateNotificationView:rv2];
    
    /*
    activegsList* l = self ; //(activegsList*)viewController;
	if (l.sourceName)
		[(UIButton*)self.navItem.titleView setTitle:l.sourceName forState:UIControlStateNormal];
     */
	
}


-(void)swipeLeft :(UISwipeGestureRecognizer*)_rec
{
	if (_rec.state == UIGestureRecognizerStateEnded)
	{
        
		[pManager doStateChange:[pManager getEmulatorView] type:kCATransitionPush sens:kCATransitionFromRight];
	}
}

-(void)swipeRight :(UISwipeGestureRecognizer*)_rec
{
	if (_rec.state == UIGestureRecognizerStateEnded)
	{
		[pManager doStateChange:[pManager getEmulatorView]type:kCATransitionPush sens:kCATransitionFromLeft];
	}
}

- (void)backButton:(id)sender {
    
#ifdef ACTIVEGS_IMAGEREQUIRED
	// vérifie que l'émulateur fonctionne
	if (!pMac)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"ActiveGS" message:@"Select a program in the list first." delegate:nil cancelButtonTitle:@"Done" otherButtonTitles:nil];
		[alert show];
		return ;
	}
    
#endif
    [pManager doStateChange:[pManager getEmulatorView] type:kCATransitionPush sens:kCATransitionFromLeft];
	
}

- (void)infoButton:(id)sender {
    
	[pManager doStateChange:[pManager getInfoView] type:kCATransitionFade sens:kCATransitionFromLeft];
}


// IOS 5 Compatibility

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   	return YES;
}

// IOS6
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


@end



void 	ios_load_roms()
{
    extern byte *g_rom_fc_ff_ptr;
    extern byte *g_rom_fc_ff_ptr;
    extern unsigned char gsrom01[];
    extern unsigned char apple2e_rom[];
    extern unsigned char apple2e_disk[];
    
	g_sim65816.g_mem_size_base = 256*1024;
	memset(&g_rom_fc_ff_ptr[0], 0, 2*65536);
    
#ifndef ACTIVEGS_MANUAL_ROM
    
#ifdef USE_APPLE2_ROM
    // main
    memcpy(&g_rom_fc_ff_ptr[4*65536-16*1024], &apple2e_rom[16*1024],16*1024);
    
    // aux
    memcpy(&g_rom_fc_ff_ptr[3*65536-16*1024], &apple2e_rom[0],16*1024);
    
    // disk
    memcpy(&g_rom_fc_ff_ptr[3*65536+0xC600], &apple2e_disk[0],1*256);
    g_sim65816.g_rom_version = -2;
    
#ifdef ACTIVEGS_NOAPPLEBRANDING
    for(int i =0;i<9;i++)
        g_rom_fc_ff_ptr[0x03ff0A+i]=0xA0; // Apple //e
    for(int i =0;i<3;i++)
        g_rom_fc_ff_ptr[0x03fbe9+i]=0xEA; // System sound
#endif
    
#else
	extern unsigned char gsrom01[];
	memcpy(&g_rom_fc_ff_ptr[2*65536], gsrom01,128*1024);
    
    // Patch la rom
	
#ifdef ACTIVEGS_NOAPPLEBRANDING
    
#define TOGS(MEM,STR)	\
{					\
char* d = (char*)MEM;		\
const char* s = STR;		\
char c;				\
while((c=*s++)!=0) *d++=(c|0x80);	\
*d=0;	\
}
    
    
    //	g_rom_fc_ff_ptr[0x03bb97]=0x60; // Ret == fully disable display
    g_rom_fc_ff_ptr[0x03bb11]=0x0D; // Number of space
    TOGS(&g_rom_fc_ff_ptr[0x03bb13],"Please Wait...");
    g_rom_fc_ff_ptr[0x03bb8e]=0x0B; // Center Line
    g_rom_fc_ff_ptr[0x03bbb8]=0x60;	// Abort display
#endif
	g_sim65816.g_rom_version = 1;
    
#endif
    
#else
	extern int readROMFromFile();
	
	int ret = readROMFromFile();
	if (!ret)
	{
		fatal_printf("ROM file not found\n");
		x_fatal_exit("You need to install first a file named \"ROM\" containing a dump of an Apple IIGS™ ROM01 (128KB).\nConnect your device to your computer, and from iTunes, copy the ROM file to MyDevice>Apps\n>ActiveGS>Documents directory.\n\nMore info on the ActiveGS website.");
		return ;
	}
    g_sim65816.g_rom_version = 1;
    
#endif
    
}


#ifdef ACTIVEGS_MANUAL_ROM

MyString romPath;

void initRomPath()
{
	NSArray *dopaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [dopaths objectAtIndex:0];
	romPath = [documentsDirectory UTF8String];
	romPath += '/';
	romPath += "ROM";
	
}

const char* getRomPath()
{
	return romPath.c_str();
}



int readROMFromFile()
{
    
	extern byte* g_rom_fc_ff_ptr;
	
	const char* rp = getRomPath();
	if (!rp)
	{
		printf("ROM Path missing\n");
		return 0;
	}
	
    int fd = open(rp, O_RDONLY | O_BINARY);
    if(fd < 0)
    {
        printf("Open ROM file %s failed:%d, errno:%d\n",rp , fd, errno);
        return 0;
    }
    struct stat stat_buf;
    int ret = fstat(fd, &stat_buf);
    if(ret != 0)
    {
        printf("fstat returned %d on fd %d, errno: %d\n",ret, fd, errno);
        close(fd);
        return 0;
    }
    
    int len = stat_buf.st_size;
    if(len != 128*1024)
    {
        printf("Only ROM01 is supported\n");
        close(fd);
        return 0;
    }
    
	
    ret = read(fd, &g_rom_fc_ff_ptr[2*65536], len);
    if(ret != len)
    {
        printf("error while reading not ROM: %d\n", errno);
        close(fd);
        return 0;
    }
    
    close(fd);
    
    // calcul un checksum rapide sur
    int crc=0;
    int l = 0;
    const char* ptr = (const char*)&g_rom_fc_ff_ptr[2*65536];
	int nb = len;
	while(nb)
    {
        crc ^= *ptr << (l%24);
        ptr++;
        l+=3;
        nb--;
    }
    if (crc != 0xFE2CE93D)
    {
        printf("ROM01 checksum failed : only original ROM01 is supported!");
        return 0;
    }
    
    return 1;
}

#endif
