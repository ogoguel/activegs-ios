/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>

@interface infoViewController : UIVIEWCONTROLLERROOT {
	
// Properties
	UIView* _portraitView;
	UILabel* _versionUI;
}

@property (nonatomic, strong) IBOutlet UILabel* versionUI;
@property (nonatomic, strong) IBOutlet UIView* portraitView;
@property (nonatomic, strong) IBOutlet UIScrollView *instructionsView;

- (IBAction) doneButton:(id)_sender;
- (IBAction) parametersButton:(id)_sender;
- (IBAction) emailButton:(id)_sender;
@end
