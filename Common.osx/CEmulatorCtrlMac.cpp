/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../common.osx/CEmulatorCtrlMac.h"
#include "../common/svnversion.h"
#include <sys/stat.h>
#include "../kegs/Src/sim65816.h"
#include <AudioToolbox/AudioToolbox.h>
extern int macUsingCoreGraphics;

extern int calcCRC(const char* ptr);



#undef printf
extern "C" int outputInfo(const char* format,...)
{
	va_list argptr;
	va_start( argptr, format );  
	MyString st;
	st.FormatV(format,argptr);
	va_end(argptr);
	puts(st.c_str());
	return 1;
}

int outputInfoInternal(const char* _str)
{
	puts(_str);
	return 1;
}

extern "C" int fOutputInfo(FILE*,const char* format,...)
{
	return 1;
}

#define printf outputInfo


void* PosixThreadMainRoutine(void* data)
{
	CEmulatorMac* p = (CEmulatorMac*)data;
	p->theThread();
    return NULL;
}



void	CEmulatorMac::x_LaunchThread()
{

	
// lance le thread
pthread_attr_t  attr;
pthread_t       posixThreadID;

assert(!pthread_attr_init(&attr));
assert(!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));

/*int     threadError =*/ pthread_create(&posixThreadID, &attr, &PosixThreadMainRoutine, this);

assert(!pthread_attr_destroy(&attr));
}		
	
void CEmulatorMac::x_WaitForEndThread()
{
	while(bInThread)
		usleep(100);
	
}



extern void my_exit(int);


void mac_get_resource_path(const char *_path,MyString& _resPath)
{
    CFURLRef resourceURL;
    _resPath.Empty();
    
    // Look for the resource in the main bundle by name and type.
    
#ifdef ACTIVEGSPLUGIN
    CFBundleRef bundle =  CFBundleGetBundleWithIdentifier(__CFStringMakeConstantString("com.freetoolsassociation.activegs"));
#else
    CFBundleRef bundle = CFBundleGetMainBundle();

#endif
    CFStringRef ce =   CFStringCreateWithCString(
                                                 NULL,
                                                 _path,
                                                 kCFStringEncodingASCII);
    resourceURL = CFBundleCopyResourceURL(
    bundle,
    ce,
    NULL,
    NULL
    );
    
    if(!resourceURL)
    {
        printf("Failed to locate a file in the loaded bundle! (%s)",_path);
        return;
    }
    
    char fileurl[1024]; 
    
    if(!CFURLGetFileSystemRepresentation(
                                         resourceURL,
                                         true,
                                         (UInt8*)
                                         fileurl,
                                         1024))
        
    {
        printf("Failed to turn a bundle resource URL into a filesystem path representation!");
        return;
    }
    

    
    _resPath = fileurl;
        
    
}


CEmulatorMac::CEmulatorMac(CEmulator** _parent, int _usingCoreGraphics,PluginInstance* _pi) : CEmulator(_parent)
{

    
	macUsingCoreGraphics = _usingCoreGraphics;
	pi = _pi;

    
}



CEmulatorMac::~CEmulatorMac()
{
	outputInfo("~CEmulatorMac (%X)\n",this);
}

void CEmulatorMac::loadingInProgress(int _motorOn,int _slot, int _drive)
{
#ifdef DRIVER_IOS
	extern void x_loadinginprogress(int,int,int);
	x_loadinginprogress(_motorOn,_slot,_drive);
#endif
}

void CEmulatorMac::getEngineString(MyString& engine)
{
	
	if (macUsingCoreGraphics)
	{
#if defined(ACTIVEGSPLUGIN) && !defined(DRIVER_IOS)
		if (isEventCocoa)
			engine="CG/Cocoa";
		else 
			engine="CG/Carbon";
#else
		engine="CG";
#endif
	}
	else
		engine  = "QD";
}

void CEmulatorMac::showStatus(const char* _str)
{
#ifdef ACTIVEGSPLUGIN
	extern	MyString messageLine;
	extern	int		 messageLineVBL;
	
	messageLine = _str;
	if (_str)
	{
		messageLineVBL=100;
		x_refresh_video();
	}

#endif
}

