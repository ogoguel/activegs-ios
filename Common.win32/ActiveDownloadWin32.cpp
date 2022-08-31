#include "../common.win32/stdafx.h"
#include <afxinet.h>
#ifndef UNDER_CE
#include <sys/stat.h>
#include <io.h>
#endif
#include <oleidl.h.>
#include "../common/cemulatorctrl.h"
#include "../Libraries/unzip101e/unzip.h"
#include "../kegs/src/sim65816.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


bool CDownload::GetFile( const char *url, const char *filename)
{
     #define HTTPBUFLEN    4096 // Size of HTTP Buffer...
     char httpbuff[HTTPBUFLEN];

#if defined(ACTIVEGSKARATEKA)
	 if (!strncmp(url,"http://asset/",13))
	 {
		extern bool x_load_asset( const char *url, const char *filename);
		return x_load_asset(url,filename);
		
	 }
#endif

	CHttpConnection* pServer = NULL;
	//CFile*	remoteFile=NULL;
	CHttpFile* remoteFile=NULL;
	CFile* myfile = NULL;
	int remotefileopen=0;

	TOSYSTEMCHAR(filename,filenamesys);

	if (filename)
	{
		outputInfo("*** Downloading %s to %s \n",url,filename);
		showProgress(url,0);
	}

	DWORD	dwLength=0;

    TRY
    {
        	  
		CString serverName;
		CString serverFile;
		DWORD dwServiceType;
		INTERNET_PORT nPort;
		DWORD	dwRet;

		CInternetSession mysession(_T("ActiveGS"));

		TOSYSTEMCHAR(url,urlsys);

		if (!AfxParseURLEx(urlsys,dwServiceType,serverName,serverFile,nPort,CString(),CString(),ICU_NO_ENCODE))
		{
			outputInfo("*** misformed url/filename : %s",url);
			AfxThrowFileException(0,0,NULL);
			return false;
		}

		if (dwServiceType ==AFX_INET_SERVICE_FILE)
		{
		
			outputInfo("*** not an http file : %s",url);
			AfxThrowFileException(0,0,NULL);
			return false;
		}
	
		pServer = mysession.GetHttpConnection(serverName/*,INTERNET_FLAG_ASYNC*/, nPort);

		//outputInfo("*** GetHttpConnection %s\n",serverName);

		remoteFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, serverFile);
	
		//outputInfo("*** OpenRequest %s\n",serverFile);
	
		if (!remoteFile->SendRequest())
		{
			outputInfo("*** cannot SendRequest : %s",serverName);
			AfxThrowFileException(0,0,NULL);
			return false;
		}
		remotefileopen=1;

		if (!remoteFile->QueryInfoStatusCode(dwRet))
		{
			outputInfo("*** QueryInfoStatusCode failed (%d)\n",GetLastError());
		}		

		if (dwRet != HTTP_STATUS_OK)
		{
			outputInfo("*** cannot query http serveur : %s (status=%d)\n",serverName,dwRet);
			AfxThrowFileException(0,0,NULL);
			return false;
		}

		DWORD BufferLength=sizeof(dwLength);

		if (!remoteFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH|HTTP_QUERY_FLAG_NUMBER, &dwLength, &BufferLength,  NULL))
		{

			// File does not exist ?
			
			if (GetLastError()!=ERROR_HTTP_HEADER_NOT_FOUND)
			{
				outputInfo("*** cannot query file size (%d) (%s || %s)\n",GetLastError(),serverName,serverFile);
				AfxThrowFileException(0,0,NULL);
				return false;
			}
			else
			{
				// ce message apparait dans le cas des stats : pas de header ?
				if (filename)
						outputInfo("*** cannot query file size ERROR_HTTP_HEADER_NOT_FOUND (%s || %s)\n",serverName,serverFile);
			}
		}	

		if (filename)
			myfile = new CFile(filenamesys, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		
		int nbread=0;
		int numbytes;

	
		int pourcent=0;

		while (numbytes = remoteFile->Read(httpbuff, HTTPBUFLEN))
		{
			nbread+=numbytes;

			if (myfile)
				myfile->Write(httpbuff, numbytes);
			
			
			if (dwLength)
				 pourcent = (nbread*100)/dwLength;
			else
				pourcent = (pourcent+5)%100;

			if (myfile)
			{
				showProgress(url,pourcent); //("downloading: %s (%3d%%)",getfile(url),pourcent);
				
				if (r_sim65816.should_emulator_terminate())
				{
					outputInfo("Download aborted (%s)\n",url);
					AfxThrowFileException(0,0,NULL);
				}
			}
		}		

		if (remoteFile)
		{
			remoteFile->Close();
			delete remoteFile;
			remoteFile=NULL;
		}

		if (pServer)
		{
			pServer->Close();
			delete pServer;
			pServer=NULL;
		}

		if (myfile)
		{
			myfile->Close();
			delete myfile;
			myfile=NULL;
		}

     }


     CATCH_ALL(error)
     {
		 int err = GetLastError();
		 
		 showStatus("Cannot download %s\n",getfile(url));

		 if (myfile)		{ myfile->Close(); delete myfile; }
		 if (remoteFile && remotefileopen)	{ remoteFile->Close(); delete remoteFile; }
		 if (pServer)		{ pServer->Close(); delete pServer; }
		 
		 DeleteFile(filenamesys);
		 
		 return false;
     }
     END_CATCH_ALL;
 
     return true;
}



bool CDownload::retrievePersistentZippedFile(const char* url,int _order,MyString& _path, MyString& _short)
{

	MyString pdir = getPersistentDir(url);
	CString dir(pdir);

	// enum
	CString search;
	search.Format(_T("%s\\%d_*.*"),dir.GetString(),_order);
	
	WIN32_FIND_DATA FindFileData;
  HANDLE hFind;

  hFind = FindFirstFile(search, &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE) 
	return false;

	MyString found(FindFileData.cFileName);
  	_path.Format("%s\\%s",pdir,found.c_str());
	_short = found.Mid(2);
	return true;

}


void	CDownload::createDirectory(const char *_dir)
{
	TOSYSTEMCHAR(_dir,dirsys);
	CreateDirectory(dirsys,NULL);
}

extern const char * activegsdownloaddir;
extern const char * activegspersistentdir;

void CDownload::initPersistentSystemPath(MyString& path)
{

	TCHAR pfraw[MAX_PATH];
	SHGetSpecialFolderPath(NULL,pfraw,CSIDL_PERSONAL,TRUE);
	path = pfraw;
}


void	CDownload::deleteFile(const char* _dir)
{
#ifndef WINCE
	remove(_dir);
#endif
}

void	CDownload::deleteDirectory(const char* _dir, int keepDir)
{
}
