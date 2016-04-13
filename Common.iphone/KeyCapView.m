//
//  KeyCapView.m
//  activegs
//
//  Created by Yoshi Sugawara on 4/9/16.
//
//

#import "KeyCapView.h"

@implementation KeyCapView

+ (instancetype)createViewWithKeyDef:(NSArray*)keyDef
{
    KeyCapView *keyCapView = [[[UINib nibWithNibName:@"KeyCapView" bundle:nil] instantiateWithOwner:nil options:nil] lastObject];
    
    if ([keyCapView isKindOfClass:[KeyCapView class]]) {
        keyCapView.keyDef = keyDef;
        return keyCapView;
    } else {
        return nil;
    }
}

- (void)setupWithKeyMapper:(KeyMapper*)keyMapper {
    if ( ![[self.keyDef objectAtIndex:KeyCapIndexShiftedKey] isEqualToString:@""] ) {
        self.keyLabel.text = [self.keyDef objectAtIndex:KeyCapIndexShiftedKey];
        self.keyLabelAlt.text = [self.keyDef objectAtIndex:KeyCapIndexKey];
    } else {
        self.keyLabel.text = [self.keyDef objectAtIndex:KeyCapIndexKey];
        self.keyLabelAlt.text = @"";
    }
    self.mappedButtonLabel.text = @"";
    KeyMapMappableButton mappedButton = [keyMapper getControlForMappedKey:[[self.keyDef objectAtIndex:KeyCapIndexCode] intValue]];
    if ( mappedButton != NSNotFound ) {
        self.mappedButtonLabel.text = [KeyMapper controlToDisplayName:mappedButton];
    }
}

@end
