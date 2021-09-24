//
//  GameControllerKeyRemapController.h
//  activegs
//
//  Created by Yoshi Sugawara on 4/8/16.
//
//

#import <UIKit/UIKit.h>
#import "KeyMapper.h"

@interface GameControllerKeyRemapController : UIViewController

@property (nonatomic, strong) IBOutlet UIView *keyboardContainerView;
@property (nonatomic, strong) IBOutlet UIButton *saveButton;
@property (nonatomic, strong) IBOutlet UIButton *cancelButton;
@property (nonatomic, strong) IBOutlet UIButton *defaultsButton;
@property (nonatomic, strong) KeyMapper *keyMapper;

@property(nonatomic, copy) void (^onDismissal)();

@end
