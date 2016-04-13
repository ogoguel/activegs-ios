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
@property (nonatomic, strong) KeyMapper *keyMapper;

@end
