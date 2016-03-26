/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once


#ifdef _WIN32
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
//#define ssnprintf _ssnprintf
#endif

#include "../kegs/Src/StdString.h"



#ifdef UNICODE	
	#define TOSYSTEMCHAR(X,Y) CStdStringW Y(X);
	#define TOSINGLECHAR(X) (X)
#else
	#define TOSYSTEMCHAR(X,Y) CStdStringA Y(X);
	#define TOSINGLECHAR(X) (X)
#endif

#define MYCHAR char

#define ACTIVEGSCACHEPATH "_DOWNLOADCACHE"

#ifdef _WIN32
#define ACTIVEGS_DIRECTORY_SEPARATOR "\\"
#else
#define ACTIVEGS_DIRECTORY_SEPARATOR "/"
#endif


const char* getfile(const char* p);
const char* getext(const char* filename);
void getdir(const char* p,MyString& _dir);
void normalize(const char* file,MyString& dest);
void removeext(char* filename);
	MyString getfilenoext(const char *url);
int calcCRC(const char* ptr);


class CDownload
{
private:

	static void initPersistentSystemPath(MyString& path);
	static  MyString	persistentPath;

public:
	static void initPersistentPath();
	

	MyString		baseURL;	// URL of the calling page
	
	const MYCHAR* requiredExt;
		
	bool	retrieveDistantFile(const MYCHAR* url,int _order,MyString& _path, MyString& _short);
	long		retrieveLocalFile(const MYCHAR* url,int _order,MyString& _path,MyString& _short);
	bool	downloadHTTPFile(const MYCHAR* filename,MyString& _path);
	
	bool	unzipFile(const MYCHAR* _url,const MYCHAR* _zipPath);

	bool	retrieveZippedFile(const MYCHAR* url,int _order,MyString& _path, MyString& _short);
//	bool	retrieveCachedZippedFile(const MYCHAR* url,int _order,MyString& _path, MyString& _short);
	bool	retrievePersistentZippedFile(const MYCHAR* url,int _order,MyString& _path, MyString& _short);

public:
	void getPersistentDirectoryFile(const char* _file, const char* _dir, MyString &dir);
	 void	getPersistentDownloadedFile(const char* _file, MyString &dir);

	
	CDownload(const char* _baseURL);
	~CDownload();

	void	setRequiredExt(const MYCHAR* _ext) { requiredExt = _ext; }
	
	MyString	getPersistentDir(const MYCHAR* url);
	static void		setPersistentPath(const char* path);
//	MyString getPersistentPathname(const char* _subdir,const char* url);


	size_t	retrieveFile(const MYCHAR* name, MyString& path,MyString& file);
	bool	GetFile(const char *url, const char *filename);

	// cache
	/*
	bool	commitCacheEntry(const char* cacheEntryName, const char* _cacheEntryPath);
	bool	createCacheEntry(const char* cacheEntryName, MyString& cacheEntryPath);
	bool	createCacheEntryName(const char* _url, const char* _pathname, int _index, MyString& cacheEntryName);
	*/

	void	setBaseURL(const char* _baseURL);
	static const char*	getPersistentPath();
	static void	createDirectory(const MYCHAR* _dir);
	static void	deleteFile(const MYCHAR* _dir);
    static int fileExists(const char*);
	void	deleteDirectory(const MYCHAR* _dir, int keepDir=0);
	void	deleteDownloadDirectory();
	static bool makeAbsolutePath(const MYCHAR* _myp, const MYCHAR* _base,MyString& _dest);
	bool bNotifyDownloadFailure;
	bool bTestOnly;
	
	int parseFilenameAndMakeAbsolute(const char* _myp, int& _order, MyString &newurl);
	 void deleteCachedFile(const MYCHAR* _path);


#if defined(DRIVER_ANDROID)
	 bool fromMainThread;
#endif
};

//extern CDownload g_download;
