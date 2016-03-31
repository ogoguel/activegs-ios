/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#include "activegsAppDelegate.h"
#include "../kegs/iOS/emulatorView.h"
#import "activegsList.h"

/*
@interface activeUITabBarController : UITabBarController

@end

@interface activeUINavigationBar : UINavigationBar

@end
*/

@interface activegsViewController : UIVIEWCONTROLLERROOT <UITabBarControllerDelegate,UIAlertViewDelegate>
{
	
    //	NSMutableArray* listViews;
	UITabBarController* _tabView;
	UINavigationBar* _navView;
	UIActivityIndicatorView* _loader;
	activegsList* _docList; // pour retrouver le label!
	activegsList* _l0;
	UINavigationItem* _navItem;
	
}

@property (strong,nonatomic) UINavigationItem* navItem;
@property (strong,nonatomic) UINavigationBar* navView;
@property (strong,nonatomic) UITabBarController* tabView;
@property (strong,nonatomic) UIActivityIndicatorView* loader;
@property (strong,nonatomic) activegsList* docList;
@property (strong,nonatomic) activegsList* l0;

- (void)loadView;
-(void)updateView;
-(void)swipeLeft :(UISwipeGestureRecognizer*)_rec ;
-(void)swipeRight :(UISwipeGestureRecognizer*)_rec ;
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation ;
-(void)activateLoader:(BOOL)_on;
//+(void)initialize;
-(void)updateDialog:(NSString*) newVersion;

@end

