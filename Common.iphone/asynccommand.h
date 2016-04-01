/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>


@interface AsyncCommand : NSObject
{
NSMutableData* _data; 
NSURLConnection* _theConnection;
NSString *_url;
id      callbackObj;
SEL     callbackSel;
}
@property (strong,nonatomic) NSMutableData* data;
@property (strong,nonatomic) NSURLConnection* theConnection;
@property (strong,nonatomic) NSString *url;

- (AsyncCommand*)initCommand: (const char*)url withObject:(id)_obj withSelector:(SEL)_sel;

@end
