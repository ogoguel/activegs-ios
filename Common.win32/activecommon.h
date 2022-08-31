#pragma once


#include "stdafx.h"
#include "../Common/CEmulatorCtrl.h"

#ifndef UNDER_CE
#include "activeinfo.h"
#include "activecommonres.h"       // main symbols
#include "activestatus.h"
#endif

//#define	FTAURL	"http://www.freetoolsassociation.com/stat/stat.php?ver=2,0,0,11"


bool retrieveProgramFilesPath(MyString& path);


#define HIST_DEFAULT_LIMIT	262144

HCURSOR CreateAlphaCursor(HINSTANCE hInst);

struct CConsole
{
	CConsole* theConsole;
	HMODULE mod;

	HWND hWnd;
	HWND hWndParent;
	int	visible;
	char ConsoleHistory[HIST_DEFAULT_LIMIT];
	int HistorySize;
	int BufferSize;
	int kNbVisibleLinesInEditCtrl;
	int enable;
	char	inputBuffer[256];
	WNDPROC OldProc;
	HFONT	hFont;


	CConsole();
	~CConsole();

	void	showConsole();
	void	hideConsole();
	void	initConsole(HINSTANCE hInst,HWND hWnd);
	void	shutConsole(HWND);
	void	toggleConsole();
	void	addLine(const char* buf);
	void	enableConsole();
	void	disableConsole();
	int		read(char* buf);
	void	refreshLine();

};

const int DEFAULTCONFIGPARAM = -1000;



class CEmulatorWin32 : public CEmulator
{
public:

#ifndef UNDER_CE
	CConsole* theConsole;
	CActiveInfo* activeInfo;	
	CActiveStatus*	wStatus;
	CActiveStatus*	wLoading;
	CBitmap			wLoadingBitmap[4];
	HCURSOR hAlphaCursor;
#endif
	HANDLE	hThread;
	HWND	m_hWnd;


	HMODULE	mod;

	HHOOK		hhkLowLevelKybd;
	void		enableSystemKeys();
	void		disableSystemKeys();
	int			hasFocus();
	
	static	DWORD WINAPI myThread(LPVOID lpParameter);
	
	HINSTANCE		hInst;

	CEmulatorWin32(CEmulator** _parent);
	int	initWindow(HWND _hWnd,HINSTANCE _hInst);
	 ~CEmulatorWin32();

	LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void	destroy();
	LRESULT	eventHandler(UINT umsg, WPARAM wParam, LPARAM lParam);
	void	keyHandler(UINT raw_vk,BOOL down, UINT flags);

	void x_LaunchThread();
	void x_WaitForEndThread();

	virtual	void	refreshStatus(const char* _msg);
	virtual	void showLoading( int _loading,int _disk);
	void ShowActiveInfo();
	virtual void showStatus(const char* _msg);
	virtual void loadingInProgress(int _motorOn,int _slot, int _drive);
	virtual	void disableConsole();
	virtual	int	activeRead(char *buf);
	virtual	void	getEngineString(MyString& _engine);

	
};




