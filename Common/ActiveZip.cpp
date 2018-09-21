/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "CEMulatorCtrl.h"
#include "ki.h"
#include "../Libraries/unzip101e/unzip.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
*/

bool CDownload::unzipFile(const MYCHAR* _url,const MYCHAR* _zipPath)
{

	MyString persistentPath;
	MyString filename;
	MyString localPath;
	MyString cacheEntryName;

	//if (bUsePersistentPath)
	{
		// crée la directory
		persistentPath = getPersistentDir(_url);
		createDirectory(persistentPath.c_str());
	}

		unzFile zf = unzOpen(_zipPath);
		if (!zf)
		{
			outputInfo("could not open %s\n",getfile(_zipPath));
			return false;
		}
		outputInfo("zip: %s\n",_zipPath);
		unz_global_info zinfo;
		unzGetGlobalInfo(zf,&zinfo);
		
		unzGoToFirstFile(zf);
	
		int iz = 0;
		do
		{
			char fnfull[1024];
			unz_file_info finfo;
			unzGetCurrentFileInfo(zf,&finfo,fnfull,1024,NULL,0,NULL,0);
			
			// OG remove any subdirectory
			MyString fn = getfile(fnfull);
			MyString filename(fn);
			filename = filename.Left(8);
			if (!filename.compare("__MACOSX"))
				continue ;
			MyString ext(getext(fn));
			if (requiredExt)
			{
				if (ext.CompareNoCase(requiredExt))
					continue;
			}
			else
			{
				if ( ext.CompareNoCase("2mg") 
                    && ext.CompareNoCase("dsk") 
                    && ext.CompareNoCase("do")
                    && ext.CompareNoCase("po")
                    && ext.CompareNoCase("raw")
                    && ext.CompareNoCase("nib")
                    && ext.CompareNoCase("bin")
                    && ext.CompareNoCase("hdv") )
					continue ;
			}

		//	if (bUsePersistentPath)
			{
			//	localPath.Format("%s"ACTIVEGS_DIRECTORY_SEPARATOR"%d_%s",persistentPath.c_str(),iz,(const char*)fn);
				localPath = persistentPath.c_str();
				localPath += ACTIVEGS_DIRECTORY_SEPARATOR;
				MyString temp;
				temp.Format("%d_",iz);
				localPath += temp.c_str();
				localPath += (const char*)fn.c_str();
			}
			/*
			else
			{
				createCacheEntryName(_url,fn,iz,cacheEntryName);
				createCacheEntry(cacheEntryName.c_str(),localPath);
			}
			*/

			outputInfo("Extracting to %s\n",localPath.c_str());
			unzOpenCurrentFile(zf);

			FILE* f = fopen(localPath.c_str(),"wb");
			if (!f)
			{
				outputInfo("Cannot create %s\n",localPath.c_str());
				return false;
			}

#define UNZIP_LEN  16*1024
			char buf[UNZIP_LEN];
			int nbread;
			int len=0;
			while( (nbread=unzReadCurrentFile (zf,buf,UNZIP_LEN)) >0 )
			{
				size_t nbw = fwrite(buf,1,nbread,f);
				if (nbw!=nbread)
				{
					outputInfo("failed to write %s\n",localPath.c_str());
					fclose(f);
					deleteFile(localPath.c_str());
					return false;
				}
				len += nbread;
			}
			fclose(f);
			if (len==0)
			{
				outputInfo("failed to uncompress %s\n",_zipPath);
				deleteFile(localPath.c_str());
				return false;
			
			}

			int err = unzCloseCurrentFile(zf);
			if (UNZ_CRCERROR==err)
			{
				outputInfo("CRC Error for %s \n",_zipPath);
				deleteFile(localPath.c_str());
				return false;
			}

			/*
			if (!bUsePersistentPath)
				commitCacheEntry(cacheEntryName.c_str(),localPath.c_str());
			*/

		iz++;
		}
		while(unzGoToNextFile(zf)==UNZ_OK);

		unzClose(zf);


			return true;
}


/*
bool retrieveCachedZippedFile(const char* url,int _order,MyString& _path, MyString& _short)
{

	//outputInfo("retrieveCachedZippedFile %s %d\n",url,_order);

	int		defaultSize = 4096;
	BYTE*	CacheEntryInfoBuffer = (BYTE*)malloc(defaultSize);
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (INTERNET_CACHE_ENTRY_INFO*)CacheEntryInfoBuffer;
	
	MyString search; search.Format("%s_%d_",url,_order);
	HANDLE hFind = NULL;
	BOOL err;
	while(1)
	{
		DWORD	dwCacheEntryInfoBufferSize = defaultSize;
		if (!hFind)
		{
			hFind = FindFirstUrlCacheEntry(NULL,lpCacheEntryInfo,&dwCacheEntryInfoBufferSize);
			err = (hFind==NULL);
		}
		else
			err = !FindNextUrlCacheEntry(hFind,lpCacheEntryInfo,&dwCacheEntryInfoBufferSize);

		if (!err)
		{
			if (!_strnicmp(lpCacheEntryInfo->lpszSourceUrlName,search.c_str(),search.GetLength()))
			{
				outputInfo("Zip Found %s (%s)\n",lpCacheEntryInfo->lpszSourceUrlName,lpCacheEntryInfo->lpszLocalFileName);
				_path = lpCacheEntryInfo->lpszLocalFileName;
				_short = lpCacheEntryInfo->lpszSourceUrlName+search.GetLength();
				FindCloseUrlCache(hFind);
				free(CacheEntryInfoBuffer);
				return true;
			}
		}
		else
		switch (GetLastError())
		{
			case ERROR_NO_MORE_ITEMS:
				outputInfo("Item not found : %s\n",url);
				FindCloseUrlCache(hFind);
				free(CacheEntryInfoBuffer);
				return false;
			case ERROR_INSUFFICIENT_BUFFER:
				CacheEntryInfoBuffer = (BYTE*)realloc(CacheEntryInfoBuffer,dwCacheEntryInfoBufferSize);
				defaultSize = dwCacheEntryInfoBufferSize;
				lpCacheEntryInfo = (INTERNET_CACHE_ENTRY_INFO*)CacheEntryInfoBuffer;
				break;
			default:
				outputInfo("failed to FindNextUrlCacheEntry : %s (%d)\n",url,GetLastError());
				FindCloseUrlCache(hFind);
				free(CacheEntryInfoBuffer);
				return false;
		}
	}
	
	// jamais	return false;

}
*/
