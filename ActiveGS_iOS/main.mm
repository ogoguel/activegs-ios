/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#import "activegsAppDelegate.h" 

int main(int argc, char *argv[]) {
    
    @autoreleasepool {
 //   int retVal = UIApplicationMain(argc, argv, nil, nil);
        int retVal = UIApplicationMain(argc, argv, nil, NSStringFromClass([activegsAppDelegate class]));
        return retVal;
    }
}
