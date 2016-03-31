/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import <UIKit/UIKit.h>
#import <QuartzCore/CALayer.h>
#include "../Common/CEMulatorCtrl.h"
#include "../Common/3rdpartylib/simplexml.h"
#include "../Common/ActiveGSList.h"

#define LABELWIDTH 260
#define LABELHEIGHT 200

@interface itemClass : NSObject
{
@public
    /*
	MyString name;
	MyString desc;
	MyString company;
	MyString year;
	MyString theid;
	MyString thumb;
//#define MAX_SCREENSHOTS 10
//	MyString screenShots[MAX_SCREENSHOTS];
	MyString screenShots0;
	MyString screenShots1;
	MyString screenShots2;
	simplexml* pXML;
	int		is2GS;
     */
    ActiveGSElement* element;
}
@end


@interface UILabelMargin : UILabel
{
	
}

@end

enum { ONLY_APPLE2=1, ONLY_2GS=2, ALL=4, ALL_NO_INDEX=8, DOC_LIST=128 };

@interface activegsList : UITableViewController <UITableViewDelegate,UITableViewDataSource> 
{
	
	
	NSArray *_searchArray ;
	NSString* _sourceName;
	NSString* _sourceRevision;
	UILabelMargin* _warningLabel;
	NSMutableArray *_listOfItems;
	int bLoaded;
    int updateCheckHasBeenDone;
//	simplexml*	pXML;
@public
    ActiveGSList list;

	MyString listPath;
    MyString versionPath;
	MyString trackerName;
    int	filter;
}

@property(nonatomic,strong) NSMutableArray* listOfItems;
@property(nonatomic,strong) UILabelMargin* warningLabel;
@property(nonatomic,strong) NSString* sourceRevision;
@property(nonatomic,strong) NSString* sourceName;
@property(nonatomic,strong) NSArray *searchArray ;

- (simplexml*)addList:(const char*)_listPath;
- (void)retrieveDocumentList:(MyString&) tempXML withBaseURL:(MyString&)baseURL;
-(void)reloadData:(BOOL)_forceDownload;
-(void)checkUpdateCallback:(NSData*)_data;
@end
