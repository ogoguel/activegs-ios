//
//  KeyCapView.h
//  activegs
//
//  Created by Yoshi Sugawara on 4/9/16.
//
//

#import <UIKit/UIKit.h>
#import "KeyMapper.h"

@interface KeyCapView : UIView

@property (nonatomic, strong) IBOutlet UILabel *keyLabel;
@property (nonatomic, strong) IBOutlet UILabel *keyLabelAlt;
@property (nonatomic, strong) IBOutlet UILabel *mappedButtonLabel;
@property (nonatomic, strong) NSArray *keyDef;

+ (instancetype)createViewWithKeyDef:(NSArray*)keyDef;
- (void)setupWithKeyMapper:(KeyMapper*)keyMapper;

@end
