/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#import "activegsAppDelegate.h"
#import "activegsList.h"
#include "../Common/svnversion.h"
#import "asyncimageview.h"
#import "activegsViewController.h"
//#include "../Common/3rdpartylib/tinyxml/tinyxml.h"
#include "../kegs/src/sim65816.h"
#include "asynccommand.h"

extern  void x_notify_download_failure(const char*_url);


@implementation itemClass 
@end

@implementation UILabelMargin 

- (CGRect)textRectForBounds:(CGRect)bounds limitedToNumberOfLines:(NSInteger)numberOfLines
{
	return [super textRectForBounds:UIEdgeInsetsInsetRect(bounds,UIEdgeInsetsMake(15,15,15,15)) limitedToNumberOfLines:numberOfLines];
}

-(void)drawTextInRect:(CGRect)rect
{
	CGRect r = [self textRectForBounds:rect limitedToNumberOfLines:self.numberOfLines];
	[super drawTextInRect:r];
}

@end


void xmltostring(simplexml* _this,MyString& _output)
{
	

	MyString s;
	s.Format("<%s",_this->key());
	_output += s;
	
	for (keyvalue_rec *ptr = _this->properties(); ptr!=NULL; ptr=ptr->next) 
	{
		s.Format(" %s=\"%s\"",ptr->key,ptr->value);
		_output += s;
	}
	
	_output +=">\n";
	const char *v = _this->value();
	if (v)
		_output +=v;
	int nc  = _this->number_of_children();
	for(int i=0;i<nc;i++)
		xmltostring(_this->child(i),_output);
	s.Format("</%s>\n",_this->key());
	_output += s;
	
}


static	UIImage* defaultImage2GS = nil;
static	UIImage* defaultImageII = nil;


@implementation activegsList

@synthesize listOfItems = _listOfItems;
@synthesize warningLabel = _warningLabel;
@synthesize sourceRevision = _sourceRevision;
@synthesize sourceName = _sourceName;
@synthesize searchArray = _searchArray ;


+(void)initialize
{
	[super initialize];
	
	float rr = [pManager resolutionRatio];
	
	NSString *imgSource = [[NSBundle mainBundle] pathForResource:@"logo_apple2gs" ofType:@"png"];
	if (imgSource)
	{
		defaultImage2GS = [UIImage imageWithContentsOfFile: imgSource] ;
		defaultImage2GS = [AsyncImageView processImage:defaultImage2GS width:64*rr height:40*rr];
	}
	else
		defaultImage2GS = nil;
	
	NSString *imgSource2 = [[NSBundle mainBundle] pathForResource:@"logo_apple2" ofType:@"png"];
	if (imgSource2)
	{
		defaultImageII = [UIImage imageWithContentsOfFile: imgSource2];
		defaultImageII = [AsyncImageView processImage:defaultImageII width:64*rr height:40*rr];
	}
	else
		defaultImageII = nil;
	
}

-(void)retrieveDocumentList:(MyString&) tempXML withBaseURL:(MyString&) _baseUrl;
{
		
	tempXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<list version=\"1\">";
	tempXML += "<source>My 2GS</source>";
	
	NSArray *dopaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
    NSString *documentsDirectory = [dopaths objectAtIndex:0];
	NSFileManager *manager = [NSFileManager defaultManager];
//    NSArray *fileList = [manager directoryContentsAtPath:documentsDirectory];
	NSError* err;
	NSArray *fileList = [manager contentsOfDirectoryAtPath:documentsDirectory error:&err];
	
	MyString ignoreList;
	
	const char* utf8docdir = [documentsDirectory UTF8String];
    listPath.Format("%s/LOCAL.ACTIVEGSXML",utf8docdir);
    _baseUrl =listPath;
	
    CDownload dl(utf8docdir);
	dl.bNotifyDownloadFailure = true;
	
	
	// parse déjà les fichier .activegsxml
	
    for (NSString *s in fileList)
	{
		const char* fn = [s UTF8String];
		const char* ext = getext(fn);
		
		if (strcasecmp(ext,"activegsxml"))
			continue;
	
		/*
		MyString fullpathname = utf8docdir;
		fullpathname += "/";
		fullpathname += fn;
		*/
		MyString path;
		MyString shortname;
	
		dl.retrieveFile(fn,path,shortname);
		FILE* f= fopen(path.c_str(),"rb");
		if (!f) continue;
		fseek(f,0,SEEK_END);
		int si = ftell(f);

		
		fseek(f,0,SEEK_SET);
		char *buf = new char[si+1];
		memset(buf,0,si+1);
		fread(buf,1,si,f);
		simplexml* p = new simplexml(buf);
			if (!p) 
			{
				delete buf;
				continue;
			}
			if (!strcasecmp(p->key(),"config"))
			{
				MyString temp;
				xmltostring(p,temp);
				printf("%s",temp.c_str());
				tempXML += temp.c_str();
				
				// rajoute les images dans une liste blackliste
				for(int i=0;i<p->number_of_children();i++)
				{
					simplexml* child = p->child(i);
					if (!strcmp(child->key(),"image"))
					{
						int order;
						MyString path;
						int isLocal = dl.parseFilenameAndMakeAbsolute(child->value(), order, path);
						if (isLocal)
						{
							ignoreList+="?";
							ignoreList+=getfile(path);
							ignoreList+="?";
					//		printf("ignoring:%s",getfile(path.c_str()));
						}
					}
				}
			}
		delete buf;
		fclose(f);
	}
	
	// parse les autres fichiers
	for (NSString *s in fileList)
	{
		const char* fn = [s UTF8String];
		const char* ext = getext(fn);
		
		if (    strcasecmp(ext,"zip")
            &&  strcasecmp(ext,"2mg")
            &&  strcasecmp(ext,"raw")
            && strcasecmp(ext,"dsk")
            && strcasecmp(ext,"po")
            && strcasecmp(ext,"do")
            && strcasecmp(ext,"nib")
            && strcasecmp(ext,"bin"))
			continue;
		
		// si le fichier est dans la liste des blacklistée : ignore
		MyString pat;
		pat.Format("?%s?",fn);
		if (ignoreList.Find(pat)!=-1)
			continue;
		
		/*
		MyString fullpathname = utf8docdir;
		fullpathname += "/";
		fullpathname += fn;
		*/
		
		MyString path;
		MyString shortname;
	//	CDownload dl;
	//	dl.bNotifyDownloadFailure = true;
		dl.retrieveFile(fn,path,shortname);
		FILE* f= fopen(path.c_str(),"rb");
		if (!f) continue;
		fseek(f,0,SEEK_END);
		int si = ftell(f);
	
			int slot;
			if (si < 800*1024)
				slot = 6;
			else
				if (si < 900*1024)
					slot = 5;
				else 
					slot = 7;
			
			tempXML += "<config version=\"2\">";
			tempXML += "<name>";
			tempXML += getfilenoext(fn);
			tempXML += "</name>";
			
			if (slot==6)
				tempXML += "<format>APPLE 2</format>";
			else
				tempXML += "<format>2GS</format>";
			
			tempXML += "<pic type=\"thumbnail\">";
//			tempXML += utf8docdir;
//			tempXML += "/";
			tempXML += getfilenoext(fn);
			tempXML += ".png</pic>";
			
			tempXML += "<image slot=\"";
			MyString slotstr;
			slotstr.Format("%d",slot);
			tempXML += slotstr;
			tempXML += "\" disk=\"1\">";
			tempXML += fn;
			tempXML += "</image>";
			tempXML += "<bootslot>";
			tempXML += slotstr;
			tempXML += "</bootslot>";
			tempXML += "</config>";
			
		
		fclose(f);
    }
	
	tempXML+="</list>";
	printf(tempXML.c_str());
}

- (simplexml*)addList:(const char*)_listPath
{
	
	double currTime = [[NSDate date] timeIntervalSince1970];

	MyString path;
	MyString shortname;
//	const char* pxmlstring ;
//	char* s = NULL;
	MyString xmlString;


    list.reset();
    
	if (!(filter & DOC_LIST))
	{
        list.loadFromFile(listPath.c_str());
    }
	else 
	{
        MyString baseURL;
		[self retrieveDocumentList:xmlString withBaseURL:baseURL];
        list.pathName = baseURL;
        list.processString(xmlString.c_str());

	}

	
    NSString* name = [NSString stringWithUTF8String:list.sourceName];
    NSString* rev = [NSString stringWithUTF8String:list.sourceRevision];
	self.sourceName=name;
	self.sourceRevision=rev;
    
    printf("nb element before filter %d\n",list.elements.size());
    for(int i=0;i<list.elements.size();i++)
    {
        ActiveGSElement* el = &list.elements.at(i);
        itemClass* ic = [itemClass alloc];
//        itemClass* ic = [itemClass alloc];

        ic->element = el;
        if (    (filter & ALL)
            || (filter & ALL_NO_INDEX)
            ||(filter & ONLY_APPLE2 && !el->is2GS)
            || (filter & ONLY_2GS && el->is2GS) )
            [self.listOfItems addObject:ic];
        ic = nil;
    }
  	double d = [[NSDate date] timeIntervalSince1970] - currTime;
	printf("loaded %s in %f\n",listPath.c_str(),d);
	//return pXMLList;
    return NULL;
}



static NSInteger compareImagesUsingSelector(id p1, id p2, void *context)
{
	itemClass* obj1 = p1;
	itemClass* obj2 = p2;
	const char* c1 = obj1->element->name.c_str();
	const char* c2 = obj2->element->name.c_str();
	int ret = strcasecmp(c1,c2);
	if (ret>0)
		return NSOrderedDescending;
	else
		if (!ret)
			return NSOrderedSame;
		else 
			return NSOrderedAscending;
}

- (void)addListAsync:(NSTimer*)timer
{
	
		
    [self addList:listPath.c_str()];
    
	if (!(filter & ALL_NO_INDEX))
		[self.listOfItems sortUsingFunction:compareImagesUsingSelector context:nil];
	
	[(activegsViewController*)[pManager getBrowserView] activateLoader:FALSE];

#ifdef ACTIVEGS_ENABLE_DOCUMENT
	if (( filter & DOC_LIST) && [self.listOfItems count]==0)
	{
		CGSize s = self.view.frame.size;

		CGRect r = CGRectMake((s.width-LABELWIDTH)/2,(s.height-LABELHEIGHT)/2,LABELWIDTH,LABELHEIGHT);
		self.warningLabel = [[UILabelMargin alloc]initWithFrame:r ];
		self.warningLabel.backgroundColor = [UIColor colorWithWhite:0.8 alpha:0.5];
		self.warningLabel.lineBreakMode = UILineBreakModeWordWrap;
		self.warningLabel.numberOfLines = 0;
	//	label.textAlignment = UITextAlignmentCenter;
		self.warningLabel.font =  [UIFont systemFontOfSize:(CGFloat)12.0];
		self.warningLabel.text = @"To add Apple II/2GS disk images :\n\n1- Connect your device to iTunes\n2- Go the 'Apps' tab\n3- Copy files (.2MG, .DSK, .ZIP, ...) to the ActiveGS Documents directory\n4- Click on the \"Refresh\" button to update the list!\n\nMore info on the ActiveGS website.";
	/*	
		[[warningLabel layer] setCornerRadius:8];
		[[warningLabel layer] setMasksToBounds:NO];
		[[warningLabel layer] setShadowColor:[UIColor blackColor].CGColor];
		[[warningLabel layer] setShadowOpacity:1.0f];
		[[warningLabel layer] setShadowRadius:6.0f];
		[[warningLabel layer] setShadowOffset:CGSizeMake(0, 3)];
	*/	
		[self.view addSubview:self.warningLabel];

	}
	
    activegsViewController* v = (activegsViewController*)[pManager getBrowserView];
    
	if ([self.listOfItems count]==0)
		[(UIButton*)v.navItem.titleView setTitle:@"Refresh" forState:UIControlStateNormal];
	else
		[(UIButton*)v.navItem.titleView setTitle:self.sourceName forState:UIControlStateNormal];

	[(UIButton*)v.navItem.titleView setHidden:NO];
#endif

    // regarde si on doit raffraichir la liste
    
    if (!versionPath.IsEmpty())
    {
        [[AsyncCommand alloc] initCommand:versionPath.c_str() withObject:self withSelector:@selector(checkUpdateCallback:)];
    }    
    
	 bLoaded = 1;
	[(UITableView*)self.view reloadData];
    [self viewWillAppear:FALSE];


}


-(void)checkUpdateCallback:(NSData*)_data
{
    
    if (_data)
    {
        
        char buffer[1024];
        memset(buffer,0,1024);
        [_data getBytes:buffer length:1023];
        
        NSString* remoteVersion = [[NSString stringWithUTF8String:buffer] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
   //     NSString* remoteVersion = @"1.08";
        
        // vérifie que ca correspond bien à une version
        if ([remoteVersion characterAtIndex:1] == '.')
        {
            NSLog(@"source: %@ current:%@ vs.remote:%@ ",self.sourceName,self.sourceRevision,remoteVersion);
        
            if (![self.sourceRevision isEqualToString:remoteVersion])
            {
                printf("need to upgrade\n");
                [(activegsViewController*)[pManager getBrowserView] updateDialog:remoteVersion];
            }
            else
                printf("nothing to update\n");
        }
        else
            printf("bad format");

    }
    else
        printf("callback failed.");
}


/*
 
 // DOES NOT WORK ON UITABLEVIEW
 
-(void)viewWillAppear:(BOOL)animated
{
	NSLog(@"viewWillAppear -- activegsList");
	// change le titre
	[pManager getBrowserView]->navItem.title = sourceName; 
	
}
 */


-(void)reloadData:(BOOL)_forceDownload
{
	//Initialize the array.
	self.listOfItems = [[NSMutableArray alloc] init];
	
    list.reset();
//	pXML = NULL;
	
	[(activegsViewController*)[pManager getBrowserView] activateLoader:TRUE];
	
	//Set the title
	//self.navigationItem.title = @"IIGS";
	
	bLoaded = 0;

	// élimine le warning
	if (self.warningLabel)
	{
		[self.warningLabel removeFromSuperview];
		self.warningLabel= nil;
	}	
	
	// force le rafraichissement de la liste
	[(UITableView*)self.view reloadData];

	
	if (_forceDownload)
	{
		CDownload dl("***ERR***");
		if (!(filter & DOC_LIST))
		{
			dl.deleteCachedFile(listPath.c_str());
		}
	}
    
#ifdef ACTIVEGS_BACKGROUNDIMAGE
	self.tableView.backgroundColor = [UIColor clearColor];
	self.tableView.opaque = NO;
#endif
    
	[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(addListAsync:) userInfo:nil repeats:NO];
    
//	[self performSelectorOnMainThread:@selector(addListAsync:)withObject:self waitUntilDone:NO];

	
}

- (void)viewWillAppear:(BOOL)animated 
{
    NSLog(@"activeGSList viewWillAppear %@",self);


     if (self.sourceName)
     {
#ifndef   ACTIVEGS_BACKGROUNDIMAGE
         [(UIButton*)[pManager getBrowserView].navItem.titleView setTitle:self.sourceName forState:UIControlStateNormal];
#else
         	((UILabel*)[pManager getBrowserView].navItem.titleView).text = self.sourceName;
#endif
     }
}

- (void)viewDidLoad {
	
	
	NSLog(@"activeGSList viewDidLoad");
    
    
	[super viewDidLoad];
	
    
    // IOS8 ISSUE !!!!! DefaultRawHeight = UITableViewAutomaticDimension
    
	CGFloat h = 44 * [pManager resolutionRatio];
	self.tableView.rowHeight = h;
	self.tableView.delegate = self;
    self.tableView.dataSource = self;
    
	
	self.searchArray = [NSArray arrayWithObjects:
				   @"#", @"A", @"B", @"C", @"D", @"E", @"F", @"G", @"H", @"I", @"J", @"K", @"L",
					 @"M", @"N", @"O", @"P", @"Q", @"R", @"S", @"T", @"U", @"V", @"W", @"X", @"Z",nil];
	
	
    
	[self reloadData:NO];
	
	
}

- (int)findIndexFromRowSection:(int)row section:(char)section 
{
	
	if (filter & ALL_NO_INDEX)
		return row;
	
	// cherche le premier
	int count = 0;
	int index = 0;
	int nb = [self.listOfItems count];
	for(int i=0;i<nb;i++)
	{
		itemClass* ic = [self.listOfItems objectAtIndex:i]; 
		char c = ic->element->name.at(0);

		if (c>='0' && c<='9') c='#';
		if (c>='a' && c<='z') c+='A'-'a';
		if (c==section)
		{
			
			if (count == row)
			{
				index = i;
				break;
			}
			else count++;
		}
	}
	return index;
}


- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
	// remets le thumbnail en top priorité
	[[asyncImageQueue sharedQueue] prioritizeItem:cell.imageView];	

	int row = [indexPath row];
	if (row & 1)
		cell.backgroundColor = [UIColor colorWithWhite:0.8 alpha:0.6];
	else 
		cell.backgroundColor = [UIColor colorWithWhite:0.9 alpha:0.6];

 }
 

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath 
{	
	
	NSInteger row = [indexPath row];
	NSInteger section = [indexPath section];
	int index = [self findIndexFromRowSection:row section:*[[self.searchArray objectAtIndex:section] UTF8String]];
    
    NSLog(@"cellForRowAtIndexPath cell %d %d %d",(int)row,(int)section,index);
	itemClass* ic = [self.listOfItems objectAtIndex:index]; 
	NSString* cellValue = [NSString  stringWithUTF8String: ic->element->name.c_str()];
	NSString *CellIdentifier = [NSString stringWithFormat:@"Cell_%@",cellValue];
	
    UITableViewCell *cell = nil;

     cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
	if (cell != nil)
		return cell;
    
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
		
	cell.textLabel.text = cellValue;

    cell.textLabel.font = [UIFont fontWithName:@"ShastonHi640" size:16*[pManager resolutionRatio]]; 

	cell.accessoryType =  UITableViewCellAccessoryDetailDisclosureButton;
	
	if ( ic->element->is2GS)
		cell.imageView.image = defaultImage2GS;
	else
		cell.imageView.image = defaultImageII;
	
    
    
	AsyncImageView* asyncImage = [AsyncImageView alloc];
	float s = [pManager resolutionRatio]*[pManager dpiRatio];
	[asyncImage initImage:ic->element->thumb.c_str() target:cell.imageView width:64*s height:40*s];
//	[asyncImage performSelectorOnMainThread:@selector(loadImage:) withObject:nil waitUntilDone:NO];
	[asyncImage loadImage:self];
	
	return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{
	int row = [indexPath row];
	int section = [indexPath section];
	int index = [self findIndexFromRowSection:row section:*[[self.searchArray objectAtIndex:section] UTF8String]];
	itemClass* ic = [self.listOfItems objectAtIndex:index]; 
	[pManager launchDiskImage:ic->element pathname:list.pathName.c_str() trackername:list.trackerName.c_str()];
		
}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath
{
	int row = [indexPath row];
	int section = [indexPath section];
	int index = [self findIndexFromRowSection:row section:*[[self.searchArray objectAtIndex:section] UTF8String]];
	itemClass* ic = [self.listOfItems objectAtIndex:index]; 
	detailViewController* dv = [pManager getDetailView];
	dv->diskSelected = ic->element ;
	dv->list = &(self->list) ;
	
	[pManager doStateChange:[pManager getDetailView] type:kCATransitionPush sens:kCATransitionFromRight];

}


- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView {
	
    NSLog(@"sectionIndexTitlesForTableView");

	if (!bLoaded)
		return nil;
	else
	if (filter & ALL_NO_INDEX)
		return nil;
	else
		return self.searchArray;
}

- (NSInteger)tableView:(UITableView *)tableView sectionForSectionIndexTitle:(NSString *)title atIndex:(NSInteger)index {
	
        NSLog(@"sectionForSectionIndexTitle %d",(int)index);
	if (!bLoaded)
		return -1;
	else
		return index;
	
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
      NSLog(@"numberOfSectionsInTableView");
	if (!bLoaded)
		return 1;
	else
	if (filter & ALL_NO_INDEX)
		return 1;
	else
		return [self.searchArray count];
}


- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section 
{
	if (!bLoaded)
		return nil;
	else
	if (filter & ALL_NO_INDEX)
		return nil;
	else
		return [self.searchArray objectAtIndex:section];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    NSLog(@"numberOfRowsInSection");
    
	if (!bLoaded)
		return 0;
	if (filter & ALL_NO_INDEX)
		return [self.listOfItems count];
	
	const char* search = [[self.searchArray objectAtIndex:section] UTF8String];
	// cherche le premier
	int count = 0;
	NSInteger nb = [self.listOfItems count];
		
	for(int i=0;i<nb;i++)
	{
		itemClass* ic = [self.listOfItems objectAtIndex:i]; 
		char c = ic->element->name.at(0);
		if (c>='0' && c<='9') c='#';
		if (c>='a' && c<='z') c+='A'-'a';
		if (c==*search)
			count++;		
		else 
		if (count)
		{
	//		printf("count: %s = %d\n",search,count);
			return count;
		}
	}
	return count;
}

- (void)dealloc
{
    /*
	if (pXML)
	{
		delete pXML;
		pXML = NULL;
	}
     */
    list.reset();
        
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
 	return YES;
}

- (BOOL)shouldAutorotate
{
 	return YES;
}

@end
