/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "activedownload.h"
//#include <sys/stat.h>

#include "ki.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char * activegsdownloaddir = "_DOWNLOADCACHE";
const char * activegspersistentdir = "ActiveGSLocalData";


MyString CDownload::persistentPath ;
//CDownload g_download;

int calcCRC(const MYCHAR* ptr)
{
	int crc=0;
	int l = 0;
	while(*ptr)
	{
		crc ^= *ptr << (l%24);
		ptr++;
		l+=3;
	}
	return crc;
}


MyString getfilenoext(const MYCHAR *url)
{
	MyString tmp(getfile(url));
	int pos = tmp.ReverseFind('.');
	return tmp.substr(0,pos);
}	


MyString CDownload::getPersistentDir(const char* url)
{
	MyString dir;
//	dir.Format("%s"ACTIVEGS_DIRECTORY_SEPARATOR"%s.%08X",getPersistentPath(),getfilenoext(url),calcCRC(url));

	dir = getPersistentPath();
	dir += ACTIVEGS_DIRECTORY_SEPARATOR;
	dir += getfilenoext(url) ;
/*
	MyString urldir;
	getdir(url,urldir);
*/
	MyString temp;
	temp.Format(".%08X",calcCRC(url));
	dir += temp.c_str();
	return dir ;
}

bool	CDownload::retrieveZippedFile(const MYCHAR* url,int _order,MyString& _path,MyString& _short)
{
	return retrievePersistentZippedFile(url,_order,_path,_short);
	/*

	if (bUsePersistentPath)
		return retrievePersistentZippedFile(url,_order,_path,_short);
	else
		return retrieveCachedZippedFile(url,_order,_path,_short);
		*/
}


// *****************************************************************


// *****************************************************************


CDownload::CDownload(const char* _baseURL)
{
	requiredExt = NULL;
	bTestOnly = false;
	bNotifyDownloadFailure = true;
#if defined(DRIVER_ANDROID)
	fromMainThread = true;
#endif
	setBaseURL(_baseURL);


}

void CDownload::deleteDownloadDirectory()
{
	// détruit le répertoire de téléchargement
	MyString dir;
	dir = getPersistentPath();
	dir += ACTIVEGS_DIRECTORY_SEPARATOR;
	dir += activegsdownloaddir;	
	deleteDirectory(dir.c_str(),1);
	
}

CDownload::~CDownload()
{
}

bool CDownload::retrieveDistantFile(const char* _url,int _order,MyString& _path, MyString& _short)
{
	
	MyString url;
	normalize(_url,url);


	MyString ext(getext(url.c_str()));
	bool bZip = !ext.CompareNoCase("zip");
	bool bRet;

	if (bZip)
	{
		// essaie de rÈcupÈrer la version existante ?
		
		bRet =  retrieveZippedFile(url.c_str(),_order,_path,_short);

		if (!bRet)
		{
			if (bTestOnly)
				return false;

			// essaie de downloader
			bRet = downloadHTTPFile(url.c_str(),_path);

			if (bRet)
			{
				if (!unzipFile(url.c_str(),_path.c_str()))
				{
					// unzip failed...
					outputInfo("unzip failed (%s)\n",getfile(url.c_str()));
					bRet=false;
					deleteFile(_path.c_str());
					outputInfo("deleting file (%s) for recovery next time \n",getfile(_path.c_str()));
					
				}

				if (bRet)
					bRet =  retrieveZippedFile(url.c_str(),_order,_path,_short);
			}
		}
	}
	else
	{
	
	//	if (bUsePersistentPath)
		{
			// fichier distant non zip
			MyString dir;
			MyString local;
			
			dir = getPersistentDir(url.c_str());
			createDirectory(dir.c_str());
			local = dir.c_str();
			local += ACTIVEGS_DIRECTORY_SEPARATOR;
			local += getfile(url.c_str()) ;
		
			// regarde si le fichier existe ?
			// QUID SI LE FICHIER EST CORROMPU ?
            
			FILE* f = fopen(local.c_str(),"rb");
			if (f)
			{
				fclose(f);
				bRet=true;
			}
			else
			{
				// ret�l�charge le fichier
				if (bTestOnly)
						return false;
				bRet = GetFile(url.c_str(),local.c_str());
				
			}
			if (bRet)
				_path = local;
		}
		/*
		else
			bRet = downloadHTTPFile(url.c_str(),_path);
		*/

		if (bRet)
			_short = getfile(_path.c_str());

	}
	
	if (!bRet)
	{
	//	showProgress(url.c_str(),-1);
		::showStatus("Failed to download %s\n",getfile(url.c_str()));
	}
	return bRet;
		
}

int CDownload::fileExists(const char * _path)
{
    FILE* f = fopen(_path,"rb");
    if (f)
    {
        fclose(f);
        return 1;
    }
    else
        return 0;
}
// *****************************************************************

long CDownload::retrieveLocalFile(const MYCHAR* _url,int _order,MyString& _path, MyString& _short)
{

	MyString url = _url;
	
	MyString ext(getext(url));	
	
	if (!ext.CompareNoCase("zip"))
	{
	
		if (!retrieveZippedFile(url,_order,_path,_short))
		{
			if (!unzipFile(url,url))
			{
				// unzip failed...
				outputInfo("unzip failed (%s)\n",_url);
				return 0;
			}
			if (!retrieveZippedFile(url,_order,_path,_short))
			{
				outputInfo("could not find zip (%s)\n",_url);
				return 0;
			}
		}
		
	}
	else
	{
		_path = url;
		
		_short = getfile(url);
	}



	FILE* f = fopen(_path.c_str(),"rb");
	if (f)
	{
		/*int err =*/ fseek(f,0,SEEK_END);
		long size = ftell(f);
		fclose(f);	
		return size;
	}
	else
	{
		outputInfo("could not find file (%s)\n",_url);
		::showStatus("Failed to load %s",getfile(_url));
		return 0;
	}

}

bool CDownload::makeAbsolutePath(const char* _myp, const char* _base,MyString& _dest)
{

	MyString p = _myp;
	MyString header5 = p.Left(5);
	MyString header17 = p.Left(17);
	MyString header8 = p.Left(8);
	MyString header7 = p.Left(7);

	bool	bAbsolute=  true;

	// OG 190110 Added \ for WinCE devices
	if ( ( p[0]=='\\') || ( p[0]=='/') || ( p[1]==':') || (!header5.CompareNoCase("http:")) )
		_dest = p; // absolute path
	else
		if (!header17.CompareNoCase("file://localhost/"))
		_dest = p.substr(17,p.length()-17); // absolute path for opera
	else
		if (!header8.CompareNoCase("file:///"))
#ifdef WIN32
		_dest = p.substr(8,p.length()-8); // absolute path
#else
	_dest = p.substr(7,p.length()-7); // keep the / on mac
#endif
	else
		if (!header7.CompareNoCase("file://"))
		_dest = p.substr(7,p.length()-7); // absolute path
	else
	{
	//	newurl.Format("%s"ACTIVEGS_DIRECTORY_SEPARATOR"%s",baseURL,p.c_str());
		_dest = _base;
		_dest += ACTIVEGS_DIRECTORY_SEPARATOR;
		_dest += p.c_str();
		bAbsolute = false;
	}	
	
	return bAbsolute;
}


int CDownload::parseFilenameAndMakeAbsolute(const char* _myp, int& _order, MyString &newurl)
{
	
	MyString p(_myp);

	// ignore #id= dans l'url
	int pos = p.ReverseFind("#id=");
	if (pos!=-1)
		p = p.Left(pos);

	// récupère le #order si présent
	 pos = p.ReverseFind("#pos=");
	if (pos!=-1)
	{
		MyString orderstr = p.Mid(pos+5).c_str();
		_order = atoi(orderstr.c_str());
		if (_order<0 || _order>9)
			_order = 0;
		p = p.Left(pos);
	}
	else
		_order = 0;

	ASSERT (!baseURL.IsEmpty());
	makeAbsolutePath(p.c_str(),baseURL.c_str(),newurl);	
	
	// OG 03-NOV-03
	// Added files network support
	if ( (newurl[0]=='/') || (newurl[1]==':') || ( (newurl[0]=='\\') /*&& (newurl[1]=='\\')*/ ) )
		return 1; //localPath = true; 
	else 
		return 0;

}

// *****************************************************************

size_t CDownload::retrieveFile(const char* _myp,MyString& _path,MyString& _short)
{
	size_t ret;


	_path.clear();	
	if (!_myp || !_myp[0] )	return false;
			
	int _order;
	MyString newurl ;
	int localPath = parseFilenameAndMakeAbsolute(_myp,_order,newurl);
	
	/*
	MyString normalized;
	normalize(newurl.c_str(),normalized);
	*/

	if (localPath)
		ret = retrieveLocalFile(newurl.c_str(),_order,_path,_short);
	else
	{
		/*
		if (_bSizeOnly)
		{
			outputInfo("size only not implemented for distant file (%s)\n",newurl.c_str());
			return 0;
		}
		*/
		ret = retrieveDistantFile(newurl.c_str(),_order,_path,_short);
	}
	return ret;
}





const char* getext(const char* _filename)
{
	MyString tmp(_filename);
	int pos = tmp.ReverseFind('.');
	return _filename+pos+1;
}


void geturldomain(const char* p,MyString& _domain)
{
	if (strncasecmp(p,"http:",5))
		_domain = "(local)";
	else
	{
		int l = 7;
		while(p[l] && p[l]!='/') l++;
		char ldir[1024];
		strncpy(ldir,p+7,l-7);
		ldir[l-7]=0;
		_domain = ldir;
	}
}



void geturldir(const char* p,MyString& _dir)
{
	// avance jusqu'à un possible'?'
	int l = 0;
	while(p[l] && p[l]!='#') l++;
	if (!l) return ; // chaine vide
	l--;
//	int l=(int)strlen(p)-1;
	while(l>=0) {
		if ( (p[l]=='\\') || (p[l]=='/') )
			break;
		l--;
	}
	char ldir[1024];
	if (l<0)l=0;
	strncpy(ldir,p,l);
	ldir[l]=0;
	_dir = ldir;
	
}


void getdir(const char* p,MyString& _dir)
{
	int l=(int)strlen(p)-1;
	while(l>=0) {
		if ( (p[l]=='\\') || (p[l]=='/') )
			break;
		l--;
	}
	char ldir[512];
	if (l<0)l=0;
	strncpy(ldir,p,l);
	ldir[l]=0;
	_dir = ldir;
	
}

const MYCHAR* getfile(const MYCHAR* p)
{
	MyString tmp(p);
	int lastslash = tmp.ReverseFind('\\');
	int lastantislash = tmp.ReverseFind('/');
	int pos = lastslash>lastantislash?lastslash:lastantislash;
	return p+pos+1;

}

void normalize(const MYCHAR* file,MyString& dest)
{
	MyString work(file);
	work.Replace("\\","/");
	
	/*
	// TODO
	const MYCHAR*p;

	const MYCHAR* find=_MYT("/../");
	while(1)	//p=strstr(work.c_str(),find))
	{
		int index = work.Find(find);
		if (index==-1) break;
		
		MyString before(work.Left(index));

		int last = work.ReverseFind('/');
		if (last==-1) break ;

		
		MyString after(work.Mid(index + 4 )); //4=strlen(find)

		work = before.Left((last-(const MYCHAR*)before.c_str())+1);
		work += after;
	}
	*/
	
	dest=work;
}
/*
const char*  CDownload::retrieveURL()
{
	return myURL.c_str();
}
void CDownload::setURL(const char* _url )
{
	myURL = _url;
	myURL.Replace("%20"," ");
}
*/

void CDownload::setBaseURL(const char* _baseURL)
{
	if (!strncasecmp(_baseURL,"file:///",8))
#ifdef WIN32
		_baseURL+=8;
#else
		_baseURL+=7; // keep the / on mac
#endif
	else
		if (!strncasecmp(_baseURL,"file://",7))
			_baseURL+=7; // absolute path
	
	baseURL = _baseURL ;
//	baseURL.Replace("%20"," ");
	
	printf("CDownload::baseURL set to %s\n",_baseURL);
}

const char* CDownload::getPersistentPath()
{
	if (persistentPath.IsEmpty())
	{
		x_fatal_exit("missing persistent path!");
		return NULL;
	}
		//this->initPersistentPath();
	
	const char* p = persistentPath.c_str();
	return p;
}


void CDownload::initPersistentPath()
{
	if (!persistentPath.IsEmpty())
		return ;

	MyString systemPath ;
	initPersistentSystemPath(systemPath);
	createDirectory(systemPath.c_str());

	systemPath += ACTIVEGS_DIRECTORY_SEPARATOR;
	systemPath += activegspersistentdir;
	createDirectory(systemPath.c_str());

	persistentPath = systemPath;

	systemPath += ACTIVEGS_DIRECTORY_SEPARATOR;
	systemPath += activegsdownloaddir;
	createDirectory(systemPath.c_str());

}

/*
bool	CDownload::createCacheEntryName(const char* _url, const char* _pathname, int _index, MyString& _cacheEntryName)
{
	_cacheEntryName.Format("%s_%d_%s",_url,_index,_pathname);
	return true;
}
*/

void CDownload::deleteCachedFile(const MYCHAR* _path)
{

	MyString url ;
	int order;
	int localPath = parseFilenameAndMakeAbsolute(_path,order,url);
	if (!localPath)
	{
		// détruit le fichier downloadé
		MyString downloaded;
		getPersistentDownloadedFile(_path,downloaded);
		printf("delete getPersistentDownloadedFile %s\n",downloaded.c_str());
		deleteFile(downloaded.c_str());
	}
	
	
	MyString ext(getext(url));
	if (!localPath || !ext.CompareNoCase("zip"))
	{
		// détruit le répertoire d'extraction
		MyString dir = getPersistentDir(url);
		printf("delete dir %s\n",dir.c_str());
		deleteDirectory(dir.c_str());
	}
}

void CDownload::getPersistentDirectoryFile(const char* _file, const char* _dir, MyString &dir)
{
	dir = getPersistentPath();
	dir += ACTIVEGS_DIRECTORY_SEPARATOR;
	dir += _dir;
	dir += ACTIVEGS_DIRECTORY_SEPARATOR;
	
	MyString fnoext  =getfilenoext(getfile(_file));
	MyString temp;
	temp.Format("%s.%08X.%s",fnoext.c_str(),calcCRC(_file),getext(_file));
	dir += temp.c_str();
}


void CDownload::getPersistentDownloadedFile(const char* _file, MyString &dir)
{

	return getPersistentDirectoryFile(_file,activegsdownloaddir,dir);

}


bool CDownload::downloadHTTPFile(const char* filename,MyString& _gotfile)
{
	outputInfo("downloadHTTPFile %s\n",filename);
	
	getPersistentDownloadedFile(filename,_gotfile);
	
	// regarde si le fichier existe
	FILE* f = fopen(_gotfile.c_str(),"rb");
	if (f)
	{
		fclose(f);
	//	_gotfile = _gotfile;
		return true;
	}
	
	return GetFile(filename,_gotfile.c_str());
	
}

