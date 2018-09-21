/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once


#if defined(__LP64__)  || defined(DRIVER_IOS)  || defined(ACTIVEGSPLUGIN)  || defined(ACTIVEGSCOCOA)
#else
	#define ENABLEQD
#endif

#ifndef DRIVER_IOS
#ifdef ACTIVEGSPLUGIN
#include "../common.xpi/npapiPlugin.h"
#endif
#endif

#include "../Common/CEMulatorCtrl.h"

#ifdef DRIVER_IOS

#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGBitmapContext.h>
#include <CoreFoundation/CFURL.h>
//extern void retrievePersistentPath(MyString&,CFStringRef);
#else
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
extern int isEventCocoa;
//extern void retrievePersistentPath(MyString&);
#endif

void x_invalidrect();
//extern int x_video_need_to_be_refreshed;

#ifdef ENABLEQD
extern CGrafPtr mac_window_port;
#endif

class PluginInstance;

class CEmulatorMac : public CEmulator
{
public:
//	int		usingCoreGraphics;
	PluginInstance* pi;
	CGContextRef theContext;
//	void* npWindow;
#ifdef ENABLEQD
	CGrafPtr window_port;
#endif
	int		bInit;
//	virtual	void	writeOptions(COption& _option);
//	virtual	COption	loadOptions() ;	
//	virtual	void	x_terminateEmulator();
//	virtual	void	x_launchEmulator();
	// To be defined in son
	virtual void	x_LaunchThread() ;
	virtual void	x_WaitForEndThread() ;
	
	CEmulatorMac(CEmulator** _parent, int usingCoreGraphics=0,PluginInstance* pi=NULL);
	void getEngineString(MyString& engine);
	virtual void loadingInProgress(int _motorOn,int _slot, int _drive);
	virtual void showStatus(const char* _msg)  ;
	~CEmulatorMac();
};
