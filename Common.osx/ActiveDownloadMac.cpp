/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../common.osx/cemulatorctrlmac.h"
#include "activedownload.h"
#include <sys/types.h>
#include <dirent.h>
#include "../common/ki.h"
#include "../kegs/Src/sim65816.h"

#ifdef DRIVER_IOS
#include <CFNetwork/CFHTTPMessage.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFHTTPStream.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ACTIVEGSDOMAIN kUserDomain
MyString homePath;

extern const char * activegsdownloaddir;
extern const char * activegspersistentdir;

#define MAX_PATH 1024

bool CDownload::GetFile( const char *url, const char *filename)
{
    
    bool bSuccess=false;    
	
    CFStringRef          rawCFString=NULL;
    CFStringRef          normalizedCFString=NULL;
    CFStringRef          escapedCFString=NULL;
    CFURLRef      urlRef=NULL;
    CFHTTPMessageRef      messageRef    = NULL;
    CFReadStreamRef        readStreamRef  = NULL;
	
    #define HTTPBUFLEN    4096 
    UInt8 httpbuff[HTTPBUFLEN];
    CFIndex numbytes ;
    FILE* f = NULL;
	int nbread=0;
	int dwLength = 0 ;
	int pourcent=0;
	MyString str;
	
		if (filename)
			showProgress(url,0); 
	
			if ( strlen( url ) < 12 ) 
                goto Bail;
			
			rawCFString    = CFStringCreateWithCString( NULL, url, CFStringGetSystemEncoding() );               
			if ( rawCFString == NULL ) goto Bail;
			normalizedCFString  = CFURLCreateStringByReplacingPercentEscapes( NULL, rawCFString, CFSTR("") ); 
			if ( normalizedCFString == NULL ) goto Bail;
			escapedCFString  = CFURLCreateStringByAddingPercentEscapes( NULL, normalizedCFString, NULL, NULL, kCFStringEncodingUTF8 );
			if ( escapedCFString == NULL ) goto Bail;
			
			urlRef= CFURLCreateWithString( kCFAllocatorDefault, escapedCFString, NULL );
			
			CFRelease( rawCFString );
			CFRelease( normalizedCFString );
			CFRelease( escapedCFString );
            if ( urlRef == NULL ) 
			{
				goto Bail;
			}
		
		messageRef = CFHTTPMessageCreateRequest( kCFAllocatorDefault, CFSTR("GET"), urlRef, kCFHTTPVersion1_1 );
		if ( messageRef == NULL ) goto Bail;
		
	
        CFRelease(urlRef);
        urlRef = NULL;
		  
		// Create the stream for the request.
		readStreamRef  = CFReadStreamCreateForHTTPRequest( kCFAllocatorDefault, messageRef );
		if ( readStreamRef == NULL ) goto Bail;
	
	
	if (!CFReadStreamOpen(readStreamRef)) 
		goto Bail;
	/*
	 tr = (CFHTTPMessageRef)CFReadStreamCopyProperty(readStreamRef, kCFStreamPropertyHTTPResponseHeader);
	if (tr)
	{
		sizeStr = CFHTTPMessageCopyHeaderFieldValue (tr,	CFSTR("Content-Length"));
		if (sizeStr)
			str = CFStringGetCStringPtr(sizeStr,CFStringGetSystemEncoding());
	}
	*/
	
	

	if (filename)
	{	
		f =fopen(filename,"wb"); 
        if (!f)
		{
			printf("cannot write file %s\n",filename);
			goto Bail;
		}
	}


	while ( (numbytes = CFReadStreamRead (readStreamRef,httpbuff,HTTPBUFLEN ) ) != 0 )
	{
		
		/*
		tr = (CFHTTPMessageRef)CFReadStreamCopyProperty(readStreamRef, kCFStreamPropertyHTTPResponseHeader);
		if (tr)
		{
			dic = CFHTTPMessageCopyAllHeaderFields (tr);
						
			char *the_pcKeys[50],*the_pcValues[50];
			
			CFDictionaryGetKeysAndValues(dic, (const void
															   **)&the_pcKeys, (const void **)&the_pcValues);
			
			sizeStr = CFHTTPMessageCopyHeaderFieldValue (tr,	CFSTR("Content-Length"));
			if (sizeStr)
				str = CFStringGetCStringPtr(sizeStr,CFStringGetSystemEncoding());
		}
		 */
		
		if (numbytes<0)
		{
		//	CFErrorRef err = CFReadStreamCopyError (readStreamRef);
			
			goto Bail;
		}
		nbread+=numbytes;
		
		if (f)
			fwrite(httpbuff,1,numbytes,f);
		
		if (dwLength)
			pourcent = (nbread*100)/dwLength;
		else
			pourcent = -nbread ; //(pourcent+5)%100;
		
		if (filename)
			showProgress(url,pourcent); 			
			if (r_sim65816.should_emulator_terminate())
			{
				outputInfo("Download aborted (%s)\n",url);
				goto Bail;
			}
		//usleep(2000);
	}		

	if (nbread)
		bSuccess=true;
	
	{
	Bail:
	{
		if (f) fclose(f);
		if (filename && !bSuccess) deleteFile(filename);
		
		if ( messageRef != NULL ) CFRelease( messageRef );
		if ( readStreamRef != NULL )
		{
			//CFReadStreamSetClient( readStreamRef, kCFStreamEventNone, NULL, NULL );
			//CFReadStreamUnscheduleFromRunLoop( readStreamRef, CFRunLoopGetCurrent(), kCFRunLoopCommonModes );
			CFReadStreamClose( readStreamRef );
			CFRelease( readStreamRef );
		}
        if ( urlRef != NULL )
            CFRelease( urlRef );
        
	}
	}
#ifdef DRIVER_IOS
	if (!bSuccess && bNotifyDownloadFailure)
	{
		extern void x_notify_download_failure(const char*);
		x_notify_download_failure(url);
	}
#endif
  return bSuccess;
}

bool CDownload::retrievePersistentZippedFile(const char* url,int _order,MyString& _path, MyString& _short)
{
	
	MyString dir = getPersistentDir(url);
	DIR* pdir = opendir(dir.c_str());
	if (!pdir) 
		return false;
		struct dirent* dp;
	MyString search;
	search.Format("%d_",_order);
	while( (dp=readdir(pdir)) != NULL )
	{
		if (strstr(dp->d_name,search.c_str())==dp->d_name)
		{
			_path = dir;
			_path += ACTIVEGS_DIRECTORY_SEPARATOR ;
			_path += dp->d_name ;
			_short = dp->d_name;
			closedir(pdir);
			return true;
		}
	}
	closedir(pdir);
	return false;
	
}


void	CDownload::deleteFile(const char* _dir)
{
	remove(_dir);

}

void	CDownload::deleteDirectory(const char* _dir,int keepdir)
{
	
	DIR* pdir = opendir(_dir);
	if (!pdir) 
		return ;
	struct dirent* dp;
	while( (dp=readdir(pdir)) != NULL )
	{
		MyString name=_dir;
		name+=ACTIVEGS_DIRECTORY_SEPARATOR;
		name+=dp->d_name;
		printf("delete file:%s\n",name.c_str());
		deleteFile(name.c_str());
	}
	closedir(pdir);
	
	if (!keepdir)
		rmdir(_dir);
	
}

void CDownload::createDirectory(const char *_subdir)
{
	
#ifdef DRIVER_IOS
	// rend le chemin absolu
	if (strstr(_subdir,homePath.c_str())==_subdir)
		_subdir += homePath.length()+1;
	MyString fullpath(homePath.c_str());
	fullpath+='/';
	fullpath+=_subdir;
	if (mkdir(fullpath.c_str(),0777))
    {
	//	printf("cannot create %s\n",fullpath.c_str());
    }
#else
	UniChar nameData[1024];
	OSStatus err;
	FSRef domain;
	
	// élimine le chemin absolu
	if (strstr(_subdir,homePath.c_str())==_subdir)
		_subdir += homePath.length()+1;
	
	err = FSFindFolder(kUserDomain, kDomainTopLevelFolderType ,kDontCreateFolder, &domain); 
	
	const char* file = getfile(_subdir);
	// cherche le chemin parent
	if (file!=_subdir)
	{
		
		MyString dir;
		UniChar ename[1024];
		FSRef subdomain;

		getdir(_subdir,dir);
		CFStringRef ed = __CFStringMakeConstantString(dir.c_str());
		int l =  CFStringGetLength(ed);
		CFStringGetCharacters(ed, CFRangeMake(0, l), ename);
		
		err = FSMakeFSRefUnicode (&domain, l, (const UniChar*)ename,0,&subdomain);
		
		domain = subdomain;
		_subdir = file;
	}
	
	
	CFStringRef sd = __CFStringMakeConstantString(_subdir);
    int lend =  CFStringGetLength(sd);
	CFStringGetCharacters(sd, CFRangeMake(0, lend), nameData);
	err = FSCreateDirectoryUnicode(&domain, lend, nameData, 0, NULL, NULL, NULL, NULL);
	if (err !=0)
		printf("cannot create %s\n",_subdir);
	else
		printf("%s created\n",_subdir);

#endif
}

							
void CDownload::initPersistentSystemPath(MyString& path)
{
	
	extern void x_init_persistent_path(MyString& hp);
	
	x_init_persistent_path(homePath);
	
	if (homePath.GetAt(homePath.GetLength()-1)=='/')
	{
		MyString tmp (homePath.Left(homePath.GetLength()-1));
		homePath = tmp;
	}
	
	path = homePath;
    /*
	CDownload::createDirectory(activegspersistentdir);
	
	MyString c;
	c = activegspersistentdir;
	c +='/';
	c += activegsdownloaddir ;
	
	CDownload::createDirectory(c.c_str());
	
	persistentPath = homePath.c_str() ;
	persistentPath += '/';
	persistentPath += activegspersistentdir;
	
	printf("mac persistent directory :%s\n",persistentPath.c_str());
//	CDownload::setPersistentPath(persistentPath.c_str());
     */
}

#ifndef DRIVER_IOS
void x_init_persistent_path(MyString& hp)
{
	
	OSStatus err;
	FSRef homeDir;
				
	err = FSFindFolder(ACTIVEGSDOMAIN, kDomainTopLevelFolderType,kDontCreateFolder, &homeDir); 
			
	CFURLRef url = CFURLCreateFromFSRef (NULL,&homeDir);

	CFStringRef fullpath  = CFURLCopyFileSystemPath (url,kCFURLPOSIXPathStyle);
	
	hp = CFStringGetCStringPtr(fullpath,CFStringGetSystemEncoding());
}
#endif