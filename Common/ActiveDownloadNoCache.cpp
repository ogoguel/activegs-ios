/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../common/cemulatorctrl.h"
#include "activedownload.h"
#include <sys/types.h>
#ifdef MAC
#include <dirent.h>
#endif
#include "../common/ki.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern const char * activegscachedir ;
extern	const char * activegsdownloaddir ;
