//
//  KeyMapper.m
//  activegs
//
//  Created by Yoshi Sugawara on 4/9/16.
//
//

#import "KeyMapper.h"

@interface  KeyMapper()
@property (nonatomic, strong) NSMutableDictionary *keyMapping;
@end

@implementation KeyMapper

-(instancetype) init {
    if ( self = [super init] ) {
        self.keyMapping = [[[NSUserDefaults standardUserDefaults] dictionaryForKey:@"keyMapping"] mutableCopy];
        if ( self.keyMapping == nil ) {
            self.keyMapping = [NSMutableDictionary dictionary];
        }
    }
    return self;
}

-(void) saveKeyMapping {
    [[NSUserDefaults standardUserDefaults] setObject:self.keyMapping forKey:@"keyMapping"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

-(void) mapKey:(AppleKeyboardKey)keyboardKey ToControl:(KeyMapMappableButton)button {
    NSNumber *buttonKey = [NSNumber numberWithInteger:button];
    [self.keyMapping setObject:[NSNumber numberWithInteger:keyboardKey] forKey:buttonKey];
}

-(AppleKeyboardKey) getMappedKeyForControl:(KeyMapMappableButton)button {
    NSNumber *buttonKey = [NSNumber numberWithInteger:button];
    NSNumber *mappedKey = [self.keyMapping objectForKey:buttonKey];
    if ( mappedKey != nil ) {
        return [mappedKey integerValue];
    } else {
        return NSNotFound;
    }
}

-(KeyMapMappableButton) getControlForMappedKey:(AppleKeyboardKey) keyboardKey {
    for (NSNumber *buttonKey in self.keyMapping) {
        NSNumber *mappedKey = [self.keyMapping objectForKey:buttonKey];
        if ( mappedKey != nil && [mappedKey integerValue] == keyboardKey ) {
            return [buttonKey integerValue];
        }
    }
    return NSNotFound;
}

+(NSString*) controlToDisplayName:(KeyMapMappableButton)button {
    switch (button) {
        case MFI_BUTTON_A:
            return @"A";
            break;
        case MFI_BUTTON_B:
            return @"B";
            break;
        case MFI_BUTTON_X:
            return @"X";
            break;
        case MFI_BUTTON_Y:
            return @"Y";
            break;
        case MFI_BUTTON_LS:
            return @"LS";
            break;
        case MFI_BUTTON_LT:
            return @"LT";
            break;
        case MFI_BUTTON_RS:
            return @"RS";
            break;
        case MFI_BUTTON_RT:
            return @"RT";
            break;
        case MFI_DPAD_UP:
            return @"mUP";
            break;
        case MFI_DPAD_DOWN:
            return @"mDOWN";
            break;
        case MFI_DPAD_LEFT:
            return @"mLEFT";
            break;
        case MFI_DPAD_RIGHT:
            return @"mRIGHT";
            break;
        case ICADE_BUTTON_1:
            return @"i1";
            break;
        case ICADE_BUTTON_2:
            return @"i2";
            break;
        case ICADE_BUTTON_3:
            return @"i3";
            break;
        case ICADE_BUTTON_4:
            return @"i4";
            break;
        case ICADE_BUTTON_5:
            return @"i5";
            break;
        case ICADE_BUTTON_6:
            return @"i6";
            break;
        case ICADE_BUTTON_7:
            return @"i7";
            break;
        case ICADE_BUTTON_8:
            return @"i8";
            break;
        default:
            return @"?";
            break;
    }
}


@end
