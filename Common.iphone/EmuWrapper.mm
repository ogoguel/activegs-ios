//
//  EmuWrapper.m
//  ActiveGS
//
//  Created by Yoshi Sugawara on 1/17/21.
//

#import <Foundation/Foundation.h>
#import "EmuWrapper.h"

#include "../Common.osx/cemulatorctrlmac.h"
#include "../kegs/Src/defc.h"
#include "../kegs/Src/sim65816.h"

@implementation EmuWrapper

+(unsigned char*) slowMemory {
    return g_slow_memory_ptr;
}

+(void)pause {
    r_sim65816.pause();
}

+(void)resume {
    r_sim65816.resume();
}

@end
