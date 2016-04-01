/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#import "activegsAppDelegate.h"
#import "activegsList.h"



@interface detailViewController : UIVIEWCONTROLLERROOT <UIWebViewDelegate,UIAlertViewDelegate> {
	
    NSURL* requestURL ;
	
    
// Properties
	UINavigationItem* _navItem;
    
    UILabel* _diskName;
	UILabel* _diskYear;
	UILabel* _diskCompany;
	UITextView* _diskDescription;
	UIImageView* _diskApple2;
	UIImageView* _diskApple2GS;
	UIWebView* _diskWebView;
	UIImageView* _screenShot1;
	UIImageView* _screenShot2;
	UIImageView* _screenShot3;
	UINavigationBar* _navView;
    /*
	itemClass* _diskSelected;
	activegsList* _list;
	*/
@public
    ActiveGSElement* diskSelected;
    ActiveGSList* list;
}


@property (nonatomic, strong) IBOutlet UILabel* diskName;
@property (nonatomic, strong) IBOutlet UILabel* diskYear;
@property (nonatomic, strong) IBOutlet UILabel* diskCompany;
@property (nonatomic, strong) IBOutlet UITextView* diskDescription;
@property (nonatomic, strong) IBOutlet UIImageView* diskApple2;
@property (nonatomic, strong) IBOutlet UIImageView* diskApple2GS;
@property (nonatomic, strong) IBOutlet UIWebView* diskWebView;
@property (nonatomic, strong) IBOutlet UIImageView* screenShot1;
@property (nonatomic, strong) IBOutlet UIImageView* screenShot2;
@property (nonatomic, strong) IBOutlet UIImageView* screenShot3;
@property (nonatomic, strong) IBOutlet UINavigationBar* navView;
/*
@property (nonatomic, retain) itemClass* diskSelected;
@property (nonatomic, retain) activegsList* list;
 */
@property (nonatomic, strong) UINavigationItem* navItem;


- (IBAction) launchButton:(id)_sender;

@end