/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#define ACTIVEGSMAJOR  3
#define ACTIVEGSMINOR  7
#ifdef KEGS_BETAVERSION
#define ACTIVEGSSTATE " BETA"
#else
#define ACTIVEGSSTATE ""
#endif

#define ACTIVEGSBUILD 1199
#define ACTIVEGSDATE "2015-06-02 22:41:06"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(_DEBUG)|| defined(_GLIBCXX_DEBUG)
#define ACTIVEGSRELEASE " [Debug]"
#else
#define ACTIVEGSRELEASE
#endif

#if defined(_WIN32)
  #if defined(XP_WIN32)
    #define ACTIVEGSAPP "Win32/Plugin"
  #elif defined(_USRDLL)
    #define ACTIVEGSAPP "Win32/ActiveX"
  #else
    #define ACTIVEGSAPP "Win32"
  #endif
#elif defined(ACTIVEGS_ANDROID)
  #define ACTIVEGSAPP "Android"
 #elif  defined(ACTIVEIPHONE)
    #	define ACTIVEGSAPP "iPhone"
  #else
  // MAC
    #	if defined(ACTIVEGSPLUGIN)
      #define ACTIVEGSAPP "MacOSX/Plugin"
    #else
     #define ACTIVEGSAPP "MacOSX"
    #	endif
 #endif

#ifdef __LP64__
#define ACTIVEGSARCHI " (64BITS)"
#else
#define ACTIVEGSARCHI
#endif 
#define ACTIVEGSBUILDSTR TOSTRING(ACTIVEGSMAJOR) "." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD) ACTIVEGSRELEASE
#define ACTIVEGSVERSIONSTR "ActiveGS v" TOSTRING(ACTIVEGSMAJOR) "." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD) ACTIVEGSRELEASE ACTIVEGSSTATE 
#define ACTIVEGSVERSIONSTRFULL "ActiveGS " ACTIVEGSAPP " v" TOSTRING(ACTIVEGSMAJOR) "." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD) ACTIVEGSRELEASE ACTIVEGSSTATE ACTIVEGSARCHI

