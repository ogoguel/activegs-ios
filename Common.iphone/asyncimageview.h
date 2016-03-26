/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#include "activegsList.h"


@interface asyncImageQueue : NSObject
{
	NSURLConnection* _theConnection ; 
	NSMutableArray* _theQueue ;
}


@property (retain,nonatomic) NSURLConnection* theConnection ; 
@property (retain,nonatomic) NSMutableArray* theQueue ;

-(void)prioritizeItem:(UIImageView*)_mytarget;
+(void) initialize;
- (void)nextDownload;
+(asyncImageQueue*)sharedQueue;
@end


@interface AsyncImageView : NSObject
{
	NSMutableData* _data; 
	UIImageView* _target;
	NSString *_url;
	int width,height;
	MyString thumb;
	MyString cachedURL;
}

@property (retain,nonatomic) NSMutableData* data; 
@property (retain,nonatomic) UIImageView* target;
@property (retain,nonatomic) NSString *url;


- (void)processDownload;
- (void)initImage:(const char*)_thumb /*theCell:(UITableViewCell*)_cell*/ target:(UIImageView*)_target width:(int)_width height:(int)_height; 
-(void)loadImage:(id)_sender;
+(UIImage*)processImage:(UIImage*)_img width:(int)_width height:(int)_height;
-(void)setImageForCell:(UIImage*) _img;


@end
