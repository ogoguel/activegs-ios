//
//  KeyMapper.h
//  activegs
//
//  Created by Yoshi Sugawara on 4/9/16.
//
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, AppleKeyboardKey) {
    KEY_CAPS = 0x39,
    KEY_OPTION = 0x37,
    KEY_APPLE = 0x3A,
    KEY_TILDE = 0x12,
    KEY_SPACE = 0x31,
    KEY_RIGHT_CURSOR = 0x3C,
    KEY_LEFT_CURSOR = 0x3B,
    KEY_UP_CURSOR = 0x3E,
    KEY_DOWN_CURSOR = 0x3D,
    KEY_SHIFT = 0x38,
    KEY_Z = 0x06,
    KEY_X = 0x07,
    KEY_C = 0x08,
    KEY_V = 0x09,
    KEY_B = 0x0B,
    KEY_N = 0x2D,
    KEY_M = 0x2E,
    KEY_COMMA = 0x2B,
    KEY_PERIOD = 0x2F,
    KEY_FSLASH = 0x2C,
    KEY_CTRL = 0x36,
    KEY_A = 0x00,
    KEY_S = 0x01,
    KEY_D = 0x02,
    KEY_F = 0x03,
    KEY_G = 0x05,
    KEY_H = 0x04,
    KEY_J = 0x26,
    KEY_K = 0x28,
    KEY_L = 0x25,
    KEY_SEMICOLON = 0x29,
    KEY_SQUOTE = 0x27,
    KEY_RETURN = 0x24,
    KEY_TAB = 0x30,
    KEY_Q = 0x0C,
    KEY_W = 0x0D,
    KEY_E = 0x0E,
    KEY_R = 0x0F,
    KEY_T = 0x11,
    KEY_Y = 0x10,
    KEY_U = 0x20,
    KEY_I = 0x22,
    KEY_O = 0x1F,
    KEY_P = 0x23,
    KEY_LEFT_BRACKET = 0x21,
    KEY_RIGHT_BRACKET = 0x1E,
    KEY_ESC = 0x35,
    KEY_1 = 0x12,
    KEY_2 = 0x13,
    KEY_3 = 0x14,
    KEY_4 = 0x15,
    KEY_5 = 0x17,
    KEY_6 = 0x16,
    KEY_7 = 0x1A,
    KEY_8 = 0x1C,
    KEY_9 = 0x19,
    KEY_0 = 0x1D,
    KEY_MINUS = 0x1B,
    KEY_EQUALS = 0x18,
    KEY_DELETE = 0x33,
    KEY_SPECIAL_TOGGLE = 0x50
};

typedef NS_ENUM(NSInteger, KeyMapMappableButton) {
    MFI_BUTTON_X,
    MFI_BUTTON_A,
    MFI_BUTTON_B,
    MFI_BUTTON_Y,
    MFI_BUTTON_LT,
    MFI_BUTTON_RT,
    MFI_BUTTON_LS,
    MFI_BUTTON_RS,
    MFI_DPAD_UP,
    MFI_DPAD_DOWN,
    MFI_DPAD_LEFT,
    MFI_DPAD_RIGHT,
    ICADE_BUTTON_1,
    ICADE_BUTTON_2,
    ICADE_BUTTON_3,
    ICADE_BUTTON_4,
    ICADE_BUTTON_5,
    ICADE_BUTTON_6,
    ICADE_BUTTON_7,
    ICADE_BUTTON_8,
    ICADE_DPAD_UP,
    ICADE_DPAD_DOWN,
    ICADE_DPAD_LEFT,
    ICADE_DPAD_RIGHT
};

typedef NS_ENUM(NSInteger, KeyCapIndex) {
    KeyCapIndexWidthMultiplier = 0,
    KeyCapIndexKey = 1,
    KeyCapIndexCode = 2,
    KeyCapIndexShiftedKey = 3
};

@interface KeyMapper : NSObject<NSCopying>

-(void)loadFromDefaults;
-(void) resetToDefaults;
-(void) saveKeyMapping;
-(void) mapKey:(AppleKeyboardKey)keyboardKey ToControl:(KeyMapMappableButton)button;
-(void) unmapKey:(AppleKeyboardKey)keyboardKey;
-(AppleKeyboardKey) getMappedKeyForControl:(KeyMapMappableButton)button;
+(NSString*) controlToDisplayName:(KeyMapMappableButton)button;
-(NSArray*) getControlsForMappedKey:(AppleKeyboardKey) keyboardKey;

@end
