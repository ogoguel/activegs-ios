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

-(void)loadFromDefaults {
    NSData *data = [[NSUserDefaults standardUserDefaults] objectForKey:@"keyMapping"];
    if ( data == nil || ![data isKindOfClass:[NSData class]] ) {
        self.keyMapping = [self defaultMapping];
    } else {
        NSDictionary *fetchedDict = [NSKeyedUnarchiver unarchiveObjectWithData:data];
        self.keyMapping = [fetchedDict mutableCopy];
    }
}

- (id)copyWithZone:(NSZone *)zone {
    KeyMapper *copy = [[[self class] alloc] init];
    copy.keyMapping = [self.keyMapping mutableCopy];
    return copy;
}

-(NSMutableDictionary*) defaultMapping {
    return [@{ [NSNumber numberWithInteger:MFI_BUTTON_X] : [NSNumber numberWithInteger:KEY_OPTION],
               [NSNumber numberWithInteger:MFI_BUTTON_A] : [NSNumber numberWithInteger:KEY_APPLE],
               [NSNumber numberWithInteger:ICADE_BUTTON_1] : [NSNumber numberWithInteger:KEY_OPTION],
               [NSNumber numberWithInteger:ICADE_BUTTON_2] : [NSNumber numberWithInteger:KEY_APPLE]
               } mutableCopy];
}

-(void) resetToDefaults {
    self.keyMapping = [self defaultMapping];
}

-(void) saveKeyMapping {
    [[NSUserDefaults standardUserDefaults] setObject:[NSKeyedArchiver archivedDataWithRootObject:self.keyMapping] forKey:@"keyMapping"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

-(void) mapKey:(AppleKeyboardKey)keyboardKey ToControl:(KeyMapMappableButton)button {
    NSNumber *buttonKey = [NSNumber numberWithInteger:button];
    [self.keyMapping setObject:[NSNumber numberWithInteger:keyboardKey] forKey:buttonKey];
}

-(void) unmapKey:(AppleKeyboardKey)keyboardKey {
    NSArray *mappedButtons = [self getControlsForMappedKey:keyboardKey];
    for (NSNumber *button in mappedButtons) {
        [self.keyMapping removeObjectForKey:button];
    }
}

-(AppleKeyboardKey) getMappedKeyForControl:(KeyMapMappableButton)button {
    NSNumber *buttonKey = [NSNumber numberWithInteger:button];
    NSNumber *mappedKey = [self.keyMapping objectForKey:buttonKey];
    if ( mappedKey != nil ) {
        return [mappedKey intValue];
    } else {
        return NSNotFound;
    }
}

-(NSArray*) getControlsForMappedKey:(AppleKeyboardKey) keyboardKey {
    NSMutableArray *foundControls = [NSMutableArray array];
    for (NSNumber *buttonKey in self.keyMapping) {
        NSNumber *mappedKey = [self.keyMapping objectForKey:buttonKey];
        if ( mappedKey != nil && [mappedKey integerValue] == keyboardKey ) {
            [foundControls addObject:buttonKey];
        }
    }
    return foundControls;
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
            return @"⬆️";
            break;
        case MFI_DPAD_DOWN:
            return @"⬇️";
            break;
        case MFI_DPAD_LEFT:
            return @"⬅️";
            break;
        case MFI_DPAD_RIGHT:
            return @"➡️";
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
        case ICADE_DPAD_UP:
            return @"i⬆️";
            break;
        case ICADE_DPAD_DOWN:
            return @"i⬇️";
            break;
        case ICADE_DPAD_LEFT:
            return @"i⬅️";
            break;
        case ICADE_DPAD_RIGHT:
            return @"i➡️";
            break;
        default:
            return @"?";
            break;
    }
}


@end
