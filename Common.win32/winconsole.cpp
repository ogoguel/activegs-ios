#include "../common.win32/stdafx.h"
#include <Richedit.h>
#include "consoleresrc.h"
#include "activecommon.h"
#include "../common/ki.h"
#include "../common/svnversion.h"
#include "../kegs/src/sim65816.h"

int outputInfoInternal(const char* _str)
{
	CEmulatorWin32* p = (CEmulatorWin32*)CEmulator::theEmulator;
	if (p)
	{
		CConsole* c = p->theConsole;
		if (c && IsWindow(c->hWnd))
			c->addLine(_str);
	}
	
	OutputDebugString(_str);
	return 0;
}

int outputInfo(const MYCHAR* format,...)
{
	
	va_list argptr;
	va_start( argptr, format );  
	CString st;
	st.FormatV(format,argptr);
	va_end(argptr);
	
	outputInfoInternal(st);
	return 0;
}

int fOutputInfo(FILE* _file,const char* format,...)
{
	va_list argptr;
	va_start( argptr, format );  
	CString st;
	st.FormatV(format,argptr);
	va_end(argptr);

	if ((_file!=stdout) && (_file!=stderr))
		fprintf(_file,st);
	else
		outputInfoInternal(st);
	return 0;

}

void	CConsole::showConsole()
{
	if (!IsWindow(hWnd)) return ;
	ShowWindow(hWnd,SW_SHOW);
	visible++;
}

void	CConsole::hideConsole()
{
	if (!IsWindow(hWnd)) return ;
	ShowWindow(hWnd,SW_HIDE);
	visible--;
	/*
	if (enable)
		// run the emulator
		strcpy(inputBuffer,"g");
	*/
}



BOOL CALLBACK EditProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
HWND parent = GetParent(hwnd);
	CConsole* theConsole = (CConsole*)GetWindowLongPtr(parent,GWLP_USERDATA);
		
	switch(Message)
	{
		case WM_CHAR:
			if (wParam==VK_RETURN)
			{
				SendMessage( hwnd, WM_GETTEXT, (WPARAM)255, (LPARAM)(LPCSTR)(theConsole->inputBuffer));
				outputInfo(">%s\n",theConsole->inputBuffer);
				SendMessage( hwnd, WM_SETTEXT, (WPARAM)NULL, (LPARAM)(LPCSTR)NULL);
				UpdateWindow(hwnd);
				return 0;
			}
			break;
	}
	return CallWindowProc(theConsole->OldProc,hwnd,Message,wParam,lParam);
}

INT_PTR CALLBACK ConsoleProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	
	CConsole* theConsole = (CConsole*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
    switch(Message)
    {
		case WM_USER:
			if (theConsole)
				theConsole->refreshLine();
			break;
        case WM_DESTROY:
			OutputDebugString("WM_DESTROY Console\n");
			
			// détruit la font
			if (theConsole)
			{
				if ( theConsole->hFont)
				{
					DeleteObject(theConsole->hFont);
					theConsole->hFont=NULL;
				}
				theConsole->hWnd=NULL;
			}
			break;
		case WM_CLOSE:
			outputInfo("WM_CLOSE\n");
			theConsole->hideConsole();
			break;
		case WM_COMMAND:
			if (theConsole)
			switch (LOWORD(wParam)) 
			{
			case IDC_GO:
				theConsole->inputBuffer[0]='g';
				theConsole->inputBuffer[1]=0;
				break;
			case IDC_REGS:
				theConsole->inputBuffer[0]=0x05;
				theConsole->inputBuffer[1]=0;
				break;
			case IDC_STEP:
				theConsole->inputBuffer[0]='s';
				theConsole->inputBuffer[1]=0;
				break;
			case IDC_BUTTONBRK:
				{
				extern int g_config_control_panel;
				if (!g_config_control_panel) 
					set_halt_act(HALT_WANTTOBRK);
				else
					outputInfo("Emulator not ready (likely in pause)...\n");
				}
				break;
			}
			break;
		case WM_CREATE:
			outputInfo("WM_CREATE\n");
			break;
		case WM_INITDIALOG:
			{
			// récupère le bon ptr
			theConsole = (CConsole*)lParam;
			// associe la classe à la fenetre
			SetLastError(0);
			if (!	SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)theConsole) && GetLastError())
				{
					LONG err = GetLastError();
					char str[255];
					sprintf(str,"SetWindowLong(initdialog) Failed (%X %d)\n",hwnd,err);
					OutputDebugString(str);
				}
				LOGFONT lf;

				// Creates a non proportionnal font for both EDIT1 & EDIT2
				
				memset( &lf, 0, sizeof(LOGFONT) );		// zero out structure
				lf.lfHeight = 14;						// request a 12-pixel-height font
				lf.lfWeight = FW_NORMAL;
				lf.lfCharSet = ANSI_CHARSET;
				lf.lfPitchAndFamily = FIXED_PITCH & FF_DONTCARE;
				strcpy( lf.lfFaceName, "Courier New" );		// request a face name "Arial"
				
				if (!(theConsole->hFont = CreateFontIndirect( &lf )) )
						OutputDebugString("font failed\n");

				// Hook a new windw procedure for EDIT1

				HWND edit2 = GetDlgItem(hwnd,IDC_EDIT1);
				ASSERT(edit2!=NULL);
				if (!SetWindowLongPtr(edit2,GWLP_USERDATA,(LONG_PTR)theConsole))
				{
					LONG err = GetLastError();
					char str[255];
					sprintf(str,"SetWindowLongPtr(edit2) Failed (%X %d)\n",edit2,err);
					OutputDebugString(str);
				}

				theConsole->OldProc = (WNDPROC)GetWindowLongPtr(edit2,GWLP_WNDPROC);
				if (!SetWindowLongPtr(edit2,GWLP_WNDPROC,(LONG_PTR)EditProc))
				{
					LONG err = GetLastError();
					char str[255];
					sprintf(str,"SetWindowLong(proc) Failed (%X %d)\n",edit2,err);
					OutputDebugString(str);
				}
				SendDlgItemMessage(hwnd,IDC_EDIT2,WM_SETFONT,(WPARAM)theConsole->hFont,TRUE);
				SendDlgItemMessage(hwnd,IDC_EDIT1,WM_SETFONT,(WPARAM)theConsole->hFont,TRUE);

				

				break;
			}
		
        default:
            return FALSE;
    }
    return TRUE;
}


	


void CConsole::addLine(const char* buf)
{
	int lb,i;
	
	lb = (int)strlen(buf);

	// ignore un message qui ne tient pas dans le buffer
	if (lb+16>HIST_DEFAULT_LIMIT)
		return ;	


	if (HistorySize+lb > (BufferSize-8) )
		HistorySize = 0;
	
	strcpy(ConsoleHistory+HistorySize, buf);
	HistorySize += lb;

	for(i=0;i<lb;i++)
		if (buf[i]=='\n') break;
	if (i==lb){
		// no line break = no nead to refresh
		return ;
	}

	PostMessage(hWnd,WM_USER,NULL,NULL);
}

void CConsole::refreshLine()
{
LONG lcount,CurLine,NbLinesToScroll;
	if (!IsWindow(hWnd)) return ;

	SendDlgItemMessage(hWnd,IDC_EDIT2,WM_SETTEXT,0,(LPARAM)(LPCSTR)ConsoleHistory);

	lcount = (LONG)SendDlgItemMessage(hWnd,IDC_EDIT2,EM_GETLINECOUNT,0,0);
	if( lcount > 1 )
	{
		CurLine = (int)SendDlgItemMessage(hWnd,IDC_EDIT2, EM_GETFIRSTVISIBLELINE, 0, 0 );
		if( CurLine > lcount )
			CurLine = lcount;
		NbLinesToScroll = lcount - CurLine - kNbVisibleLinesInEditCtrl;

		if( NbLinesToScroll < 0 )
			NbLinesToScroll = 0;

		SendDlgItemMessage(hWnd,IDC_EDIT2, EM_LINESCROLL, 0, NbLinesToScroll );
	}
}

void CConsole::shutConsole(HWND _hWndParent)
{

//	if (_hWndParent!=hWndParent) return ;
	
	OutputDebugString("shutConsole\n");
		
	if (IsWindow(hWnd))
	{
		DestroyWindow(hWnd);
		hWnd=NULL;
	}


}


CConsole::CConsole()
{
	
	mod = LoadLibrary("riched20.dll");
//	CEmulator::theEmulator->theConsole=this;
	hWnd  = NULL;
	enable = 0;
}

CConsole::~CConsole()
{
	FreeLibrary(mod);
//	CEmulator::theEmulator->theConsole=NULL;

}

void CConsole::initConsole(HINSTANCE _hInst,HWND _hWndParent)
{

	if (hWnd)
	{
		outputInfo("Console already running\n");
		return ; // Already running
	}

	hWndParent = _hWndParent;

	//theConsole=this;
	HistorySize = 0;
	visible=0;
	kNbVisibleLinesInEditCtrl = 23;
	BufferSize = HIST_DEFAULT_LIMIT;
	inputBuffer[0]=0;


		// console 
//	__asm int 3;
	OutputDebugString("createConsole\n");
	
	hWnd  = CreateDialogParam(_hInst, MAKEINTRESOURCE(IDD_CONSOLE), _hWndParent, ConsoleProc,(LPARAM)this);
	if (!hWnd)
	{
		outputInfo("createConsole failed... %d (%X)\n",GetLastError(),_hInst);
	//	theConsole=NULL;
		return ;
	}

	SendDlgItemMessage(hWnd,IDC_EDIT2, EM_EXLIMITTEXT, (WPARAM)HIST_DEFAULT_LIMIT, 0);
	

	return ;
}


void CConsole::toggleConsole()
{
	outputInfo("toggleConsole\n");
	
	if (!visible)
		showConsole();
	else
		hideConsole();
	
}



int	CConsole::read(char* buf)
{
	int l;

	if (!enable) enableConsole();

	while(!inputBuffer[0] && IsWindow(hWnd))
	{
		Sleep(100);
		if (r_sim65816.should_emulator_terminate())
				break;
	}

	strcpy(buf,inputBuffer);
	l = strlen(buf);
	if (l)
		buf[l++]=0x0A;
	buf[l]=0;
	inputBuffer[0]=0;
	return l;
}

void CConsole::enableConsole()
{
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT1),TRUE);
	enable=1;
	inputBuffer[0]=0;	// clear previous commands
	if (!visible)
		showConsole();
}

void CConsole::disableConsole()
{
	
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT1),FALSE);
	enable=0;
}


