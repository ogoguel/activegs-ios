#include "../common.win32/stdafx.h"
#include <Psapi.h>
#include <mmsystem.h>
#include "activecommon.h"
#include "../kegs/src/sim65816.h"
#include "../kegs/src/driver.h"
#include "../kegs/src/adb.h"
#include "../common.win32/activegs.rc.h"


extern const char* currentBuild;

void x_display_alert(int _abord, const char* _title, const char* _msg)
{
	
	CString strsys(_msg);
	CString titlesys(_title);

	MessageBox(NULL,strsys,titlesys,MB_OK);
}


CEmulatorWin32::CEmulatorWin32(CEmulator** _parent) : CEmulator(_parent)
{

	m_hWnd = NULL;
	hThread = NULL;
#ifndef UNDER_CE
		hAlphaCursor = NULL;
	activeInfo = NULL;
	wStatus=NULL;
	wLoading=NULL;
	theConsole=NULL;
	hhkLowLevelKybd=NULL;
#endif

}
	
extern void my_exit(int);

CEmulatorWin32::~CEmulatorWin32()
{
	outputInfo("~CEmulatorWin32 (%X)\n",this);
	destroy();
#ifndef UNDER_CE
	disableSystemKeys();
#endif
	
}
extern	void kegsshut();

DWORD WINAPI CEmulatorWin32::myThread(LPVOID lpParameter)
{
	CEmulatorWin32* emu = (CEmulatorWin32*)lpParameter;
	kegsinit(emu->m_hWnd);
	emu->theThread();
	kegsshut();
	CloseHandle(emu->hThread);
	emu->hThread=NULL;
	return 0;
}






int CEmulatorWin32::initWindow(HWND _hWnd, HINSTANCE _hInstance)
{

	m_hWnd = _hWnd;
	hInst = _hInstance;

#ifndef UNDER_CE
	theConsole=new CConsole();
	CWnd *pw =CWnd::FromHandle(m_hWnd);
	wStatus = new CActiveStatus(pw);
	wStatus->Create(CActiveStatus::IDD,pw);
	wLoading = new CActiveStatus(pw);
	wLoading->Create(IDD_LOADING,pw);

	wLoadingBitmap[0].LoadBitmap(IDB_DISK51);
	wLoadingBitmap[1].LoadBitmap(IDB_DISK52);
	wLoadingBitmap[2].LoadBitmap(IDB_DISK61);
	wLoadingBitmap[3].LoadBitmap(IDB_DISK62);

	enableSystemKeys();
#endif

	resetLocalIMG();
	return 1;
	
}

void async_snd_shutdown();
void async_sound_init();

void CEmulatorWin32::x_WaitForEndThread()
{
	// enlËve le mode pause
	//g_pause=0;

	DWORD r;
	MSG msg;
	while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE )||bInThread ) 
	{
		//	Sleep(20);
		int bStillRunning = hThread && (r=WaitForSingleObject(hThread,0)==WAIT_TIMEOUT);
		if (!bStillRunning)
		{
			outputInfo("exiting thread %X %X\n",hThread,r);
			break;
		}
	}

}
/*
void CEmulatorWin32::activateEmulator()
{
	if (theEmulator != this) 
		return ;
	launchEmulator(	this->hInst,this->m_hWnd );
}
*/

void CEmulatorWin32::x_LaunchThread( )
{
	if (theEmulator != this) return ;

	outputInfo("launching win32 thread\n");
	
	DWORD threadID;

#ifndef UNDER_CE
	theConsole->initConsole(this->hInst,this->m_hWnd);
	hAlphaCursor = CreateAlphaCursor(hInst);
#endif

	// initialize le son async
	//async_sound_init();

	hThread= CreateThread(NULL,0,myThread,this,0,&threadID );
	if (!hThread)
		outputInfo("thread initialisation failed!\n");
/*
	else
		bRun = true;
*/	
}



LRESULT CEmulatorWin32::eventHandler(UINT umsg, WPARAM wParam, LPARAM lParam)
{
	return win_event_handler(m_hWnd,umsg,wParam,lParam);
}

void win_event_key_async(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags);

void	CEmulatorWin32::keyHandler(UINT raw_vk,BOOL down, UINT flags)
{
	win_event_key_async(m_hWnd,raw_vk,  down, 0/*repeat*/, flags);
}

int CEmulatorWin32::hasFocus()
{
	HWND h = ::GetFocus();
	if (h == m_hWnd) return 1;
	else return 0;
}

#ifndef UNDER_CE
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{

   BOOL fEatKeystroke = FALSE;
   CEmulatorWin32* emu = (CEmulatorWin32*)CEmulator::theEmulator;

   // Manage the key only if the emulator has the focus
   if (emu && emu->hasFocus() && nCode == HC_ACTION) 
   {
	  PKBDLLHOOKSTRUCT p;
		p = (PKBDLLHOOKSTRUCT) lParam;

		int down=0;
		switch (wParam) 
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: 	
			down=1;	
		case WM_SYSKEYUP:
		case WM_KEYUP: 
			
		int syskeys = ( p->vkCode == VK_TAB && p->flags & LLKHF_ALTDOWN ) ||
                      ( p->vkCode == VK_ESCAPE &&  p->flags & LLKHF_ALTDOWN ) ||
					  ( p->vkCode == VK_ESCAPE && (GetKeyState(VK_CONTROL) & 0x8000)) ||
                        p->vkCode == VK_LWIN ||
                        p->vkCode == VK_RWIN ||
						p->vkCode==VK_CANCEL || 
						p->vkCode==VK_SCROLL ||
						p->vkCode==VK_TAB ||
						p->vkCode==VK_LMENU ||
                        p->vkCode == VK_APPS;

			if (syskeys	)
			{
				fEatKeystroke = 1;
			//	outputInfo("LLSYKEY: %X\n",p->vkCode);
				ASSERT(emu);
				emu->keyHandler(p->vkCode,down,((p->flags&0x1)<<8));
			}
			else
			{
			//	outputInfo("LLKEY: %X\n",p->vkCode);
			}

		break;
       	}
   }
   else
   {
	//   outputInfo("NO LL: %X %X %X",GetActiveWindow(),::GetFocus(),emu?emu->m_hWnd:0 );
   }

   return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, 
          lParam));
}

void CEmulatorWin32::enableSystemKeys()
{
	hhkLowLevelKybd  = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, AfxGetInstanceHandle(), 0);
}

void CEmulatorWin32::disableSystemKeys()
{
	if (hhkLowLevelKybd)
		UnhookWindowsHookEx(hhkLowLevelKybd);
}
#endif


extern const char * activegsdownloaddir ;

extern int g_config_control_panel;

LRESULT CEmulatorWin32::windowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	{
		if (message==WM_USER)
		{
			if (wParam==LOADING_ON)
			{
				showLoading(TRUE,lParam);
				return TRUE;
			}
			else
			if (wParam==LOADING_OFF)
			{
				showLoading(FALSE,lParam);
				return TRUE;
			}
			else
			if (wParam==REFRESH_STATUS)
			{
				refreshStatus(this->statusMsg.c_str());		
				return TRUE;
			}
			else
				return eventHandler(message,wParam,lParam);

		}
		else
#ifndef UNDER_CE
		if (message==WM_SETCURSOR)
		{
			if (hasFocus() && !g_config_control_panel)	
			{
				SetCursor(hAlphaCursor); 
				return TRUE;
			}
			else
			{
				//outputInfo("nofocus\n");
				return FALSE;
			}
		}
		else
#endif
		if (message==WM_RBUTTONDOWN)
		{
			ShowActiveInfo();
		}
		else
			return eventHandler(message,wParam,lParam);
	}

	return DefWindowProc(m_hWnd,message, wParam, lParam);
}

void CEmulatorWin32::destroy()
{
	
	
	outputInfo("onDestroy\n");

#ifndef UNDER_CE
	if (hAlphaCursor)
	{
		DestroyIcon(hAlphaCursor);
		hAlphaCursor = NULL;
	}
	if (activeInfo)
	{
		activeInfo->DestroyWindow();
		delete activeInfo;
		activeInfo=NULL;
	}

	if (wStatus 	)
	{
	//	if (IsWindow(wStatus->m_hWnd))
			wStatus->DestroyWindow();
		delete wStatus;
		wStatus = NULL;
	
	}
	

	if (wLoading )
	{
		
		// if (IsWindow(wLoading->m_hWnd))
			wLoading->DestroyWindow();
		delete wLoading;
		wLoading=NULL;
	}
	

	for(int i=0;i<4;i++)
			wLoadingBitmap[i].DeleteObject();

	if (theConsole)
	{
		theConsole->shutConsole(this->m_hWnd);
		delete theConsole;
		theConsole=NULL;
	}
#endif
}


void CEmulatorWin32::ShowActiveInfo()
{
	if(!bInThread) 
	{
		MessageBox(m_hWnd,_T("Close other instances first & reload page"),_T("ActiveGS"),MB_OK);
		return ;
	}	
#ifndef UNDER_CE
	if (!activeInfo)
	{
		CWnd *pw =CWnd::FromHandle(m_hWnd);
		activeInfo = new CActiveInfo(pw);	// this?
		activeInfo->Create(CActiveInfo::IDD,pw);	// this ?
	}

	g_adb.g_warp_pointer=WARP_NONE;
	g_adb.g_hide_pointer = 0;
	x_hide_pointer(false);

	activeInfo->ShowWindow(SW_SHOW);
#endif
}

void CEmulatorWin32::refreshStatus(const char* _msg)
{
#ifndef UNDER_CE
	if (wStatus)
	{
		RECT my;
		wStatus->GetWindowRect(&my);
		wStatus->ScreenToClient(&my);
		my.left += 8;
		my.right += 8;
		my.bottom += 8;
		my.top +=8;

		wStatus->MoveWindow(&my);
		wStatus->SendDlgItemMessage(IDC_STATUS,WM_SETTEXT,0,(LPARAM)(LPCSTR)_msg);
		wStatus->ShowWindow(SW_SHOWNOACTIVATE);
		OutputDebugString(_msg);
		if (wStatus->TimerID)
			wStatus->KillTimer(wStatus->TimerID);
		wStatus->TimerID = wStatus->SetTimer(0,2000,NULL);

	}
#endif
}


void CEmulatorWin32::showLoading( int _loading,int _disk)
{
#ifndef UNDER_CE
	if (wLoading)
	{
		if (_loading)
		{		
			RECT my;
			wLoading->GetWindowRect(&my);
			wLoading->ScreenToClient(&my);
			
			
			CWnd* w = CWnd::FromHandle(this->m_hWnd);
			RECT wrect;
			w->GetClientRect(&wrect);


			my.left += wrect.right-56;
			my.right += wrect.right-56;
			my.bottom += 8;
			my.top +=8;
			wLoading->MoveWindow(&my);

			CWnd * hWndViewer = wLoading->GetDlgItem(IDC_IMAGECTRLDISK); 
			hWndViewer->SendMessage( STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) wLoadingBitmap[_disk].m_hObject ); 

			wLoading->ShowWindow(SW_SHOWNOACTIVATE);
			if (wLoading->TimerID)
			{
				wLoading->KillTimer(wLoading->TimerID);
				wLoading->TimerID = NULL;
			}
		}
		else
		{
			if (!wLoading->TimerID)
				wLoading->TimerID = wLoading->SetTimer(2,2000,NULL);
		}
		
	}
#endif
}

#ifndef UNDER_CE
void DeleteCursor(HCURSOR theCursor)
{
	DestroyIcon(theCursor);
}

typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIEXYZTRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
} MYBITMAPV5HEADER;



HCURSOR CreateAlphaCursor(HINSTANCE hInst)
{
HDC hMemDC;
DWORD dwWidth, dwHeight;
MYBITMAPV5HEADER bi;
HBITMAP hBitmap, hOldBitmap;
void *lpBits;
DWORD x,y;
HCURSOR hAlphaCursor = NULL;

HDC hdc;
HDC src;
HBITMAP hMonoBitmap;
DWORD *lpdwPixel;
ICONINFO ii;
HGDIOBJ hOldSrc;

	HBITMAP hBm = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ALPHACURSOR));
	if (!hBm)
	{
		outputInfo("Load AlphaCursor failed (inst:%0X)\n",hInst);
		return NULL;
	}

	BITMAP bm;
	GetObject(hBm, sizeof(bm), &bm);
	/*
	alpha.GetBitmap(&bm);
	*/

	dwWidth = bm.bmWidth; // width of cursor
	dwHeight = bm.bmHeight; // height of cursor

	ZeroMemory(&bi,sizeof(MYBITMAPV5HEADER));
	bi.bV5Size = sizeof(MYBITMAPV5HEADER);
	bi.bV5Width = dwWidth;
	bi.bV5Height = dwHeight;
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	// The following mask specification specifies a supported 32 BPP
	// alpha format for Windows XP.
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;
	bi.bV5AlphaMask = 0xFF000000;


	hdc = GetDC(NULL);

	// Create the DIB section with an alpha channel.
	hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS,
	(void **)&lpBits, NULL, (DWORD)0);

	hMemDC = CreateCompatibleDC(hdc);
	src  = CreateCompatibleDC(hdc);
	ReleaseDC(NULL,hdc);

	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	hOldSrc = SelectObject( src, hBm );
	BitBlt(hMemDC,0,0,dwWidth,dwHeight,src,0,0,SRCCOPY);
	SelectObject(src,hOldSrc);
	DeleteDC(src);
	SelectObject(hMemDC,hOldBitmap);
	DeleteDC(hMemDC);

	// Create an empty mask bitmap
	hMonoBitmap = CreateBitmap(dwWidth,dwHeight,1,1,NULL);

	lpdwPixel = (DWORD *)lpBits;
	for (x=0;x<dwWidth;x++)
		for (y=0;y<dwHeight;y++)
		{
		// Clear the alpha bits
		*lpdwPixel &= 0x00FFFFFF;
		// Set the alpha bits to 0x9F (semi-transparent)
		if (*lpdwPixel!=0x00FFFFFF)
			*lpdwPixel |= 0x9F000000;

			
		lpdwPixel++;
		}


	
	ii.fIcon = FALSE; // Change fIcon to TRUE to create an alpha icon
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmMask = hMonoBitmap;
	ii.hbmColor = hBitmap;

	// Create the alpha cursor with the alpha DIB section
	hAlphaCursor = CreateIconIndirect(&ii);

	DeleteObject(hBitmap);
	DeleteObject(hMonoBitmap);

	return hAlphaCursor;
}
#endif

void CEmulatorWin32::showStatus(const char* _msg)
{
	if (IsWindow(m_hWnd))
	{
		statusMsg = _msg;
		SendMessage(m_hWnd,WM_USER,REFRESH_STATUS,0);
	}
}


void CEmulatorWin32::loadingInProgress(int _motorOn,int _slot, int _drive)
{
	int msg;
	if (!_motorOn)
		msg =LOADING_OFF;
	else
		msg=LOADING_ON;
				
	if (IsWindow(m_hWnd))
	{
		SendMessage(m_hWnd,WM_USER,msg,((_slot-5)&1)*2+((_drive-1)&1));
	}
}


void CEmulatorWin32::disableConsole(void)
{
#ifndef UNDER_CE
	if (!theConsole) return ;
	theConsole->disableConsole();
#endif
}

int	CEmulatorWin32::activeRead(char *buf)
{
#ifndef UNDER_CE
	CConsole* c = theConsole;
	if (c)
		return c->read(buf);
#endif
	buf[0]=0;
	return 0;
	
}
void	CEmulatorWin32::getEngineString(MyString& _engine)
{

	_engine ="WIN32";
#ifdef USE_RASTER
	_engine+="(vbl)";
#endif
};


void x_refresh_panel(int _enum)
{
#ifndef UNDER_CE
	CEmulatorWin32* p = (CEmulatorWin32*)CEmulator::theEmulator;
	if (!p) return;
	if (!p->activeInfo) return ;
	if (_enum & PANEL_PARAMS)
		p->activeInfo->updateParams();
	if (_enum & (PANEL_DISK | PANEL_RUNTIME))
		p->activeInfo->refresh();
#endif
}


const int soundWin32Ressources[]=
{
	NULL,
	IDR_SPINUPSEARCH1,
	IDR_SPINUPSEARCH2
};

void x_play_sound(enum_sound hSound)
{
	HINSTANCE h = ((CEmulatorWin32*)CEmulator::theEmulator)->hInst;
	BOOL err = PlaySound(MAKEINTRESOURCE(soundWin32Ressources[hSound]), h, SND_RESOURCE | SND_ASYNC); 
}

int x_is_sound_playing(enum_sound _hSound)
{
	return 0; 
};