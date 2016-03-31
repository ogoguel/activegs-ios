/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "activegsAppDelegate.h"
#import "AsyncImageView.h"
#include "../common/CEmulatorCtrl.h"


static asyncImageQueue* queue = nil;	
static UIActivityIndicatorView* asyncloader=nil; 


@implementation asyncImageQueue

@synthesize  theQueue = _theQueue;
@synthesize  theConnection = _theConnection;


+(void) initialize{
	
	// singleton
	queue = [[asyncImageQueue alloc] init];
	queue.theQueue = [[NSMutableArray alloc] init ];
	queue.theConnection = nil;
	
	asyncloader = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
	
	
}

+(asyncImageQueue*)sharedQueue
{
	return queue;
}


-(void)prioritizeItem:(UIImageView*)_mytarget {
	
	int nb = [self.theQueue count];
	for(int i=0;i<nb;i++)
	{
		AsyncImageView* o = [self.theQueue objectAtIndex:i];
		if (o.target==_mytarget)
		{
            // le remet au début de la pile = A FAIRE DANS CETTE ORDRE
		    [self.theQueue removeObject:o];
            [self.theQueue addObject:o];
			debug_printf("image put on top of the cache %s\n",[o.url UTF8String]);
			return;
		}
	}
}

- (void)nextDownload {
	
    debug_printf("next download");
	if (self.theConnection)
	{
        debug_printf("*wait\n");
		return ;
	}
						  
	AsyncImageView* obj = [self.theQueue lastObject];
	if (obj)
	{
		[obj processDownload];
	}
	else 
	{
		debug_printf("*nothing to unqueue*\n");
	}
						  
}
-(void)removeObject:(id)_obj
{
    debug_printf("remove object");
	[self.theQueue removeObject:_obj];	
   // [self.theConnection release];
	self.theConnection = nil;
	[self nextDownload];
}			  
@end

@implementation AsyncImageView

@synthesize data = _data; 
@synthesize target = _target;
@synthesize url = _url;
							


- (void)processDownload {

//	printf("processDownload (%s)\n",[url UTF8String]);
	
	[asyncloader removeFromSuperview];

	CGRect r = self.target.frame;
	CGRect f = asyncloader.frame;
	f.origin.x = (r.size.width-f.size.width)/2;
	f.origin.y = (r.size.height-f.size.height)/2;
	asyncloader.frame = f;
	[ self.target addSubview:asyncloader];
	[asyncloader startAnimating];	
	
	NSURL* urlnoescape = [[NSURL alloc] initWithString:[ self.url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]] ;

	NSURLRequest* request = [NSURLRequest requestWithURL:urlnoescape cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:60.0] ;

    queue.theConnection  = [[NSURLConnection alloc] initWithRequest:request delegate:self]  ;
    
	if (!queue.theConnection)
		printf("NSURLConnection failed\n");
                             
 
}

- (void)initImage:(const char*)_thumb target:(UIImageView*)_mytarget width:(int)_width height:(int)_height
{
	thumb = _thumb;
	width = _width;
	height = _height;	
    self.target = _mytarget;
	
}

//- (void)loadImage:(const char*)_thumb target:(UIImageView*)_target width:(int)_width height:(int)_height
- (void)loadImage:(id)_sender
{

	
	if (thumb.IsEmpty()) return ;
	NSString* str = [NSString  stringWithUTF8String:thumb.c_str()];
	if ( self.data!=nil) 
    { 
        self.data =nil;
    }
	if (!str) return ;
	

    // regarde si l'image générée est dans le cache
						   
	MyString fnoext  =getfilenoext(getfile(thumb.c_str()));
	MyString cache;					   
	cache.Format("%s_%d_%d.%s",fnoext.c_str(),width,height,getext(thumb.c_str()));
    CDownload dl("***ERROR***");
    dl.getPersistentDirectoryFile(cache.c_str(),IPHONETHUMB,cachedURL);
	NSString* strc = [NSString stringWithUTF8String:cachedURL.c_str()] ;
	UIImage* imageView  = [UIImage imageWithContentsOfFile:strc] ; 
	if (imageView)
	{
		debug_printf("target image loaded from cache %s\n",getfile(cachedURL.c_str()));
        self.target.image = imageView;
		return ;
	}
				 
	// non : regarde si l'image original a déjà été téléchargé
	
	if ([str length] > 7 && [[str substringToIndex:7] compare:@"http://"])
	{
		NSString *str2;
		if (thumb.at(0) != '/')
		{
			// ressource image
			str2 = [[NSBundle mainBundle] pathForResource:str ofType:nil];
		}
		else
		{
			// absolute path
			str2 = str ; //[NSString stringWithUTF8String:str];
		}
		imageView = [UIImage imageWithContentsOfFile:str2]; // to mimic init 
		if (!imageView)
		{
			printf("init image failed (%s)\n",[str2 UTF8String]);
			return ;
		}
		debug_printf("image loaded from resource %s\n",getfile([str2 UTF8String]));
		
	}
	else 
	{
		self.url = str;
	
		// regarde si l'image est dans le cache
		
		dl.getPersistentDirectoryFile([self.url UTF8String],IPHONETHUMB,cache);
		NSString* strcache = [NSString stringWithUTF8String:cache.c_str()];
		imageView = [UIImage imageWithContentsOfFile:strcache];
		if (imageView)
		{
			debug_printf("image loaded from cache %s\n",getfile(cache.c_str()));
		}
		else
		{
			
			asyncImageQueue* q = [asyncImageQueue sharedQueue];
			[q.theQueue addObject:self];
			[q nextDownload];
			
			return ;
		}
		
	}

	[self setImageForCell:imageView];


}


//the URL connection calls this repeatedly as data arrives
- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)incrementalData {
	if (self.data==nil) { self.data = [[NSMutableData alloc] initWithCapacity:2048]; } 
	[self.data appendData:incrementalData];
}

//the URL connection calls this once all the data has downloaded
- (void)connectionDidFinishLoading:(NSURLConnection*)connection {
	
	[asyncloader removeFromSuperview];
	[asyncloader stopAnimating];
	
	UIImage* imageView = [[UIImage alloc] initWithData:self.data]  ;
	if (!imageView)
	{
		printf("init image failed (%s)\n",[self.url UTF8String]);
	}
	else
	{
		// process l'image
		NSData * png = self.data;
		MyString cache;
        CDownload dl("***ERROR***");
		dl.getPersistentDirectoryFile([self.url UTF8String],IPHONETHUMB,cache);
		FILE* f = fopen(cache.c_str(),"wb");
		if (f)
		{
			fwrite(png.bytes,1,png.length,f);
			fclose(f);
		}
		else 
		{
			printf("cannot update cache %s\n",cache.c_str());
		}

		debug_printf("image downloaded (%s)\n",getfile([self.url UTF8String]));
		[self performSelectorOnMainThread:@selector(setImageForCell:) withObject:imageView waitUntilDone:NO];
		 
	}

	self.data=nil;

	[[asyncImageQueue sharedQueue] removeObject:self];

}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	printf("didFailWithError (%s)\n",[self.url UTF8String]);
    
    [asyncloader removeFromSuperview];
	[asyncloader stopAnimating];
	
	self.data=nil;

	[[asyncImageQueue sharedQueue] removeObject:self];
		
}

+(UIImage*)processImage:(UIImage*)_img width:(int)_width height:(int)_height
{
#define IMAGESCALE  1
#define SHADOW 2
#define LEFTMARGIN 6
#define BOTTOMMARGIN 0
#define TOPMARGIN 0

	const float s = [pManager dpiRatio];
	
	CGRect r = CGRectMake(0,0,_width*s,_height*s);
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB(); 
	CGContextRef context = CGBitmapContextCreate(nil, (int)r.size.width,(int)r.size.height, 8, (int)r.size.width*4,colorSpace,kCGImageAlphaPremultipliedLast/*kCGImageAlphaNone*/);

	float ovalWidth=14*s,ovalHeight=14*s;
    CGContextSaveGState(context);
    CGContextTranslateCTM (context, CGRectGetMinX(r), CGRectGetMinY(r));
    CGContextScaleCTM (context, ovalWidth, ovalHeight);
    float fw = CGRectGetWidth (r) / ovalWidth;
    float fh = CGRectGetHeight (r) / ovalHeight;
    CGContextMoveToPoint(context, fw, fh/2);
    CGContextAddArcToPoint(context, fw, fh, fw/2, fh, 1);
    CGContextAddArcToPoint(context, 0, fh, 0, fh/2, 1);
    CGContextAddArcToPoint(context, 0, 0, fw/2, 0, 1);
    CGContextAddArcToPoint(context, fw, 0, fw, fh/2, 1);
    CGContextClosePath(context);
	CGContextRestoreGState(context);
	CGContextClip(context);
	//CGContextSetInterpolationQuality(context,kCGInterpolationHigh);
	CGContextDrawImage(context,r,[_img CGImage]);
	CGImageRef imageRef2 = CGBitmapContextCreateImage(context);
	// Crée une nouvelle image pour le shadow
	CGRect r2 = CGRectMake(0,0,r.size.width+(LEFTMARGIN+SHADOW)*s,r.size.height+(SHADOW*2+BOTTOMMARGIN+TOPMARGIN)*s);
	CGContextRef context2 = CGBitmapContextCreate(nil,(int)r2.size.width,(int)r2.size.height, 8, (int)r2.size.width*4,colorSpace,kCGImageAlphaPremultipliedLast/*kCGImageAlphaNone*/);

	CGContextSaveGState(context2);
	CGContextSetShadowWithColor(context2, CGSizeMake(SHADOW*s, -SHADOW*s), 4, [[UIColor grayColor] CGColor]);
	
	CGRect rdest = r;
	rdest.origin.x = LEFTMARGIN*s;
	rdest.origin.y = (BOTTOMMARGIN+SHADOW)*s;
	//CGContextSetInterpolationQuality(context2,kCGInterpolationHigh);
	
	CGContextDrawImage(context2,rdest,imageRef2);
	CGContextRestoreGState(context2);	
	
	CGImageRef imageRef = CGBitmapContextCreateImage(context2);
	UIImage* newImage = [UIImage imageWithCGImage:imageRef];
	
	CGColorSpaceRelease(colorSpace);
	CGContextRelease(context);
	CFRelease(imageRef);
	CGContextRelease(context2);
	CFRelease(imageRef2);
	
	return newImage;

}

-(void)setImageForCell:(UIImage*) _img
{
    debug_printf("setImageForCell");
    
	UIImage* img2 = [AsyncImageView processImage:_img width:width height:height];
	
	[self.target setNeedsDisplay];
	
	[UIView beginAnimations:@"ToggleSiblings" context:nil];
    [UIView setAnimationTransition:UIViewAnimationTransitionFlipFromLeft /*UIViewAnimationTransitionCurlUp*/ forView:self.target cache:YES];
    [UIView setAnimationDuration:1.0];
	self.target.image = img2;
    [UIView commitAnimations];
    
	// sauve dans le cache
	NSData *png = UIImagePNGRepresentation(img2);     
	
	FILE* f = fopen(cachedURL.c_str(),"wb");
	if (f)
	{
		fwrite(png.bytes,1,png.length,f);
		fclose(f);
	}
	else 
	{
		printf("cannot update cache %s\n",cachedURL.c_str());
	}
    
	
}
@end
