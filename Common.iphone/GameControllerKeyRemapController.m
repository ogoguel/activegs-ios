//
//  GameControllerKeyRemapController.m
//  activegs
//
//  Created by Yoshi Sugawara on 4/8/16.
//
//

#import <GameController/GameController.h>
#import "GameControllerKeyRemapController.h"
#import "KeyCapView.h"
#import "KeyMapper.h"

const CGFloat NUMBER_OF_KEYS_IN_ROW = 15.0f;

const CGFloat KEYCAP_WIDTH_PCT = 1.0f / NUMBER_OF_KEYS_IN_ROW;

const CGFloat KEYCAP_HORIZONTAL_PADDING = 2.0f;
const CGFloat KEYCAP_VERTICAL_PADDING = 2.0f;
const CGFloat KEYCAP_HEIGHT = 40.0f;


// Using a C struct here for really just convenience of typing the definitions out
// This gets converted into an obj-c object later
struct KeyCap {
    CGFloat widthMultiplier;
    const char* key1;
    int code1;
    const char* key2;
};

struct KeyCap keyCapDefinitions[] = {
    { 1.2,"caps",KEY_CAPS,0 },
    { 1.5,"opt",KEY_APPLE,0 },
    { 1.0,"",KEY_OPTION,0 },
    { 1.0,"`",KEY_TILDE,0 },
    { 6.3," ",KEY_SPACE,0 },
    { 1.0,"←",KEY_LEFT_CURSOR,0 },
    { 1.0,"→",KEY_RIGHT_CURSOR,0 },
    { 1.0,"↑",KEY_UP_CURSOR,0 },
    { 1.0,"↓",KEY_DOWN_CURSOR,0 },
    { -1,0,0,0 },
    { 2.5,"shift",KEY_SHIFT,0 },
    { 1.0,"Z",KEY_Z,0 },
    { 1.0,"X",KEY_X,0 },
    { 1.0,"C",KEY_C,0 },
    { 1.0,"V",KEY_V,0 },
    { 1.0,"B",KEY_B,0 },
    { 1.0,"N",KEY_N,0 },
    { 1.0,"M",KEY_M,0 },
    { 1.0,",",KEY_COMMA,"<" },
    { 1.0,".",KEY_PERIOD,">" },
    { 1.0,"/",KEY_FSLASH,"?" },
    { 2.5,"shift",KEY_SHIFT,0 },
    { -1,0,0,0 },
    { 2.0,"control",KEY_CTRL,0 },
    { 1.0,"A",KEY_A,0 },
    { 1.0,"S",KEY_S,0 },
    { 1.0,"D",KEY_D,0 },
    { 1.0,"F",KEY_F,0 },
    { 1.0,"G",KEY_G,0 },
    { 1.0,"H",KEY_H,0 },
    { 1.0,"J",KEY_J,0 },
    { 1.0,"K",KEY_K,0 },
    { 1.0,"L",KEY_L,0 },
    { 1.0,";",KEY_SEMICOLON,":" },
    { 1.0,"'",KEY_SQUOTE,"\""},
    { 2.0,"return",KEY_RETURN,0 },
    { -1,0,0,0 },
    { 2.0,"tab",KEY_TAB,0 },
    { 1.0,"Q",KEY_Q,0 },
    { 1.0,"W",KEY_W,0 },
    { 1.0,"E",KEY_E,0 },
    { 1.0,"R",KEY_R,0 },
    { 1.0,"T",KEY_T,0 },
    { 1.0,"Y",KEY_Y,0 },
    { 1.0,"U",KEY_U,0 },
    { 1.0,"I",KEY_I,0 },
    { 1.0,"O",KEY_O,0 },
    { 1.0,"P",KEY_P,0 },
    { 1.5,"[",KEY_LEFT_BRACKET,"{" },
    { 1.5,"]",KEY_RIGHT_BRACKET,"}" },
    { -1,0,0,0 },
    { 1.0,"esc",KEY_ESC,0 },
    { 1.0,"1",KEY_1,"!" },
    { 1.0,"2",KEY_2,"@" },
    { 1.0,"3",KEY_3,"#" },
    { 1.0,"4",KEY_4,"$" },
    { 1.0,"5",KEY_5,"%" },
    { 1.0,"6",KEY_6,"^" },
    { 1.0,"7",KEY_7,"&" },
    { 1.0,"8",KEY_8,"*" },
    { 1.0,"9",KEY_9,"(" },
    { 1.0,"0",KEY_0,")" },
    { 1.0,"-",KEY_MINUS,"_" },
    { 1.0,"=",KEY_EQUALS,"+" },
    { 2.0,"delete",KEY_DELETE,0 },
    { 0,0,0,0 }
};

@interface GameControllerKeyRemapController () <UIAlertViewDelegate>
@property (nonatomic, strong) NSMutableArray *keyCapViews;
@property (nonatomic, strong) UIAlertView *alertView;
@property (nonatomic, strong) KeyMapper *keyMapperWorkingCopy;
@end

@implementation GameControllerKeyRemapController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.keyCapViews = [NSMutableArray array];
    self.keyMapperWorkingCopy = [self.keyMapper copy];
    self.saveButton.layer.borderWidth = 1.0f;
    self.saveButton.layer.borderColor = [self.view.tintColor CGColor];
    self.cancelButton.layer.borderWidth = 1.0f;
    self.cancelButton.layer.borderColor = [[UIColor redColor] CGColor];
    self.defaultsButton.layer.borderWidth = 1.0f;
    self.defaultsButton.layer.borderColor = [self.view.tintColor CGColor];
    [self constructKeyboard];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

// convert the above c struct array into an objective c object to make it easier to deal with (for me, personally)
-(NSArray*) objc_keyCapsDefinitions {
    NSMutableArray *keyDefs = [NSMutableArray array];
    NSMutableArray *keyDefsRow = [NSMutableArray array];
    int i = 0;
    while (keyCapDefinitions[i].widthMultiplier) {
        struct KeyCap keyCap = keyCapDefinitions[i];
        if ( keyCap.widthMultiplier == -1.0 ) {
            [keyDefs addObject:[keyDefsRow copy]];
            [keyDefsRow removeAllObjects];
            i++;
            continue;
        }
        [keyDefsRow addObject:@[ [NSNumber numberWithFloat:keyCap.widthMultiplier],
                                 [NSString stringWithUTF8String:keyCap.key1],
                                 [NSNumber numberWithInt:keyCap.code1],
                                 keyCap.key2 != 0 ? [NSString stringWithUTF8String:keyCap.key2] : @""
                                 ]];
        i++;
    }
    [keyDefs addObject:keyDefsRow];
    return [keyDefs copy];
}

// Construct the keyboard key views using auto layout constraints entirely
// It gets constructed from the bottom up (pinned to the bottom)
-(void) constructKeyboard {
    
    NSArray *keyDefinitions = [self objc_keyCapsDefinitions];
    UIView *keyboardContainer = self.keyboardContainerView;
    UIView *lastVerticalView = nil;
    UIView *lastHorizontalView = nil;
    NSUInteger keyboardRow = 0;

    for (NSArray *keyDefsRow in keyDefinitions) {
       
        KeyCapView *keyCapView = nil;
        
        NSUInteger keyIndex = 0;
        
        for (NSArray *keyDef in keyDefsRow) {
            keyCapView = [KeyCapView createViewWithKeyDef:keyDef];
            keyCapView.translatesAutoresizingMaskIntoConstraints = NO;
            keyCapView.layer.borderWidth = 1.0f;
            keyCapView.layer.borderColor = [[UIColor blackColor] CGColor];
            [keyCapView setupWithKeyMapper:self.keyMapperWorkingCopy];
            UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onKeyTap:)];
            [keyCapView addGestureRecognizer:tap];
            [self.keyCapViews addObject:keyCapView];
            
            CGFloat widthMultiplier = [[keyDef objectAtIndex:KeyCapIndexWidthMultiplier] floatValue] * KEYCAP_WIDTH_PCT;
            
            NSDictionary *metrics = @{@"height" : @(KEYCAP_HEIGHT), @"padding" : @(KEYCAP_HORIZONTAL_PADDING)};
            [keyboardContainer addSubview:keyCapView];
            
            // vertical
            if ( keyboardRow == 0 ) {
                NSDictionary *bindings = NSDictionaryOfVariableBindings(keyCapView);
                // pin to super view bottom
                [keyboardContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[keyCapView(height)]-0@750-|" options:0 metrics:metrics views:bindings]];
            } else {
                NSDictionary *bindings = NSDictionaryOfVariableBindings(keyCapView,lastVerticalView);
                // pin bottom to last vertical view
                [keyboardContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[keyCapView(height)]-4@750-[lastVerticalView]" options:0 metrics:metrics views:bindings]];
            }
            
            // horizontal
            NSLayoutConstraint *widthConstraint = [NSLayoutConstraint constraintWithItem:keyCapView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:keyboardContainer attribute:NSLayoutAttributeWidth multiplier:widthMultiplier constant:KEYCAP_HORIZONTAL_PADDING * -1.0];
            
            if ( lastHorizontalView == nil ) {
                NSDictionary *bindings = NSDictionaryOfVariableBindings(keyCapView);
                [keyboardContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-padding@750-[keyCapView]" options:0 metrics:metrics views:bindings]];
            } else {
                NSDictionary *bindings = NSDictionaryOfVariableBindings(keyCapView,lastHorizontalView);
                if ( keyIndex ==  (int)NUMBER_OF_KEYS_IN_ROW-1 ) {
                    // last key in row
                    [keyboardContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[lastHorizontalView]-padding@750-[keyCapView]-padding@750-|" options:0 metrics:metrics views:bindings]];
                } else {
                    [keyboardContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[lastHorizontalView]-padding@750-[keyCapView]" options:0 metrics:metrics views:bindings]];
                }
            }
            if ( keyIndex != (int)NUMBER_OF_KEYS_IN_ROW-1 ) {
                [keyboardContainer addConstraint:widthConstraint];
            }
            
            keyIndex++;
            lastHorizontalView = keyCapView;
        }
        
        lastVerticalView = keyCapView;
        lastHorizontalView = nil;
        keyboardRow++;
    }
}

- (void) refreshAllKeyCapViews {
    for (KeyCapView *view in self.keyCapViews) {
        [view setupWithKeyMapper:self.keyMapperWorkingCopy];
    }
}

- (void) onKeyTap:(UITapGestureRecognizer*)sender {
    KeyCapView *view = (KeyCapView*) sender.view;
    self.alertView = [[UIAlertView alloc] initWithTitle:@"Remap Key" message:[NSString stringWithFormat:@"Press a button to map the [%@] key",[view.keyDef objectAtIndex:KeyCapIndexKey]] delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Unbind",nil];
    [self.alertView show];
    self.alertView.tag = [[view.keyDef objectAtIndex:KeyCapIndexCode] integerValue];
    [self startRemappingControlsForMfiControllerForKey:[view.keyDef objectAtIndex:KeyCapIndexCode]];
}

- (void) startRemappingControlsForMfiControllerForKey:(NSNumber*)keyCode {
    AppleKeyboardKey keyboardKey = [keyCode intValue];
    if ( [[GCController controllers] count] == 0 ) {
        NSLog(@"Could not find any mfi controllers!");
        return;
    }
    GCController *controller = [[GCController controllers] firstObject];
    
    if ( controller.extendedGamepad ) {
        controller.extendedGamepad.valueChangedHandler = ^(GCExtendedGamepad *gamepad, GCControllerElement *element) {
            if ( gamepad.buttonA.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_A];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.buttonB.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_B];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.buttonX.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_X];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.buttonY.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_Y];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.leftShoulder.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_LS];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.rightShoulder.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_RS];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.xAxis.value > 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_RIGHT];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.xAxis.value < 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_LEFT];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.yAxis.value > 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_UP];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.yAxis.value < 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_DOWN];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.rightTrigger.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_RT];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.leftTrigger.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_LT];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
        };
    } else {
        controller.gamepad.valueChangedHandler = ^(GCGamepad *gamepad, GCControllerElement *element) {
            if ( gamepad.buttonA.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_A];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.buttonB.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_B];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.buttonX.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_X];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.buttonY.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_Y];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];;
                return;
            }
            if ( gamepad.leftShoulder.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_LS];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.rightShoulder.pressed ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_BUTTON_RS];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.xAxis.value > 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_RIGHT];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.xAxis.value < 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_LEFT];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.yAxis.value > 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_UP];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
            if ( gamepad.dpad.yAxis.value < 0.0f ) {
                [self.keyMapperWorkingCopy mapKey:keyboardKey ToControl:MFI_DPAD_DOWN];
                [self.alertView dismissWithClickedButtonIndex:0 animated:YES];
                return;
            }
        };
    }
}

-(void) stopRemappingControls {
    if ( [[GCController controllers] count] == 0 ) {
        return;
    }
    GCController *controller = [[GCController controllers] firstObject];
    if ( controller.extendedGamepad ) {
        controller.extendedGamepad.valueChangedHandler = nil;
    } else {
        controller.gamepad.valueChangedHandler = nil;
    }
}

-(IBAction)saveButtonTapped:(id)sender {
    self.keyMapper = [self.keyMapperWorkingCopy copy];
    [self.keyMapper saveKeyMapping];
    self.keyMapperWorkingCopy = nil;
    [self.presentingViewController dismissViewControllerAnimated:YES completion:^{
        self.onDismissal();
    }];
}

-(IBAction)cancelButtonTapped:(id)sender {
    self.keyMapperWorkingCopy = nil;
    [self.presentingViewController dismissViewControllerAnimated:YES completion:^{
        self.onDismissal();
    }];    
}

-(IBAction) defaultsButtonTapped:(id)sender {
    [self.keyMapperWorkingCopy resetToDefaults];
    [self refreshAllKeyCapViews];
}

#
# pragma mark - UIAlertViewDelegate
#

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    [self stopRemappingControls];
    if ( buttonIndex == 1 ) {
        AppleKeyboardKey mappedKey = alertView.tag;
        [self.keyMapperWorkingCopy unmapKey:mappedKey];
    }
}

- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex {
    [self stopRemappingControls];
    [self refreshAllKeyCapViews];
}

@end
