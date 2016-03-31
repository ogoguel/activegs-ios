/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "activegsAppDelegate.h"
#include "../common/CEmulatorCtrl.h"
#include "asynccommand.h"

@implementation AsyncCommand

@synthesize theConnection = _theConnection;
@synthesize url = _url;
@synthesize data = _data;
							

- (AsyncCommand*)initCommand: (const char*)_myurl withObject:(id)_obj withSelector:(SEL)_sel 
{
	
    callbackObj = _obj;
    callbackSel = _sel;
    
   self.url = [NSString stringWithUTF8String:_myurl];
    
	NSURL* urlnoescape = [[NSURL alloc] initWithString:[self.url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];

	NSURLRequest* request = [NSURLRequest requestWithURL:urlnoescape cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:60.0];
	self.theConnection = [[NSURLConnection alloc] initWithRequest:request delegate:self]; 

	
    if (!self.theConnection)
    {
		printf("NSURLConnection failed\n");
        // appelle la callback
        [callbackObj performSelector:callbackSel withObject:nil];
        return nil;
    }
    {
         // ajoute une référence
        return self;
    }
}


//the URL connection calls this repeatedly as data arrives
- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)incrementalData {
	if (self.data==nil) { self.data = [[NSMutableData alloc] initWithCapacity:2048]; } 
	[self.data appendData:incrementalData];
}

//the URL connection calls this once all the data has downloaded
- (void)connectionDidFinishLoading:(NSURLConnection*)connection {
	    
    printf("callback received\n");
    [callbackObj performSelector:callbackSel withObject:self.data];
	self.data=nil;
	
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	
	printf("callback failed\n",[self.url UTF8String]);
    [callbackObj performSelector:callbackSel withObject:nil];
	self.data=nil;
}

@end
