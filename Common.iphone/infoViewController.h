/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>

@interface infoViewController : UIVIEWCONTROLLERROOT {
	
// Properties
	UIView* _portraitView;
	UIView* _landscapeView;
	UILabel* _versionUI;
	UILabel* _versionUILandscape;
}

@property (nonatomic, retain) IBOutlet UILabel* versionUILandscape;
@property (nonatomic, retain) IBOutlet UILabel* versionUI;
@property (nonatomic, retain) IBOutlet UIView* portraitView;
@property (nonatomic, retain) IBOutlet UIView* landscapeView;

- (void)updateView:(UIDeviceOrientation)_orientation ;
- (IBAction) doneButton:(id)_sender;
- (IBAction) parametersButton:(id)_sender;
- (IBAction) emailButton:(id)_sender;
@end
