//
//  EmuWrapper.h
//  activegs
//
//  Created by Yoshi Sugawara on 1/17/21.
//

#ifndef EmuWrapper_h
#define EmuWrapper_h


#endif /* EmuWrapper_h */

#import <Foundation/Foundation.h>


@interface EmuWrapper: NSObject
+(unsigned char*) memory;
+(void)pause;
+(void)resume;
+(UIView*)getEmulatorView;
+(unsigned int)cpuGetProgramCounter;

@end
