
// ChildView.cpp : implementation of the CChildView class
//

#include "../Common.win32/stdafx.h"
#include "ChildView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	pEmulatorCtrl = NULL;
	parent = NULL;
	//pConfig = NULL;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
#ifndef UNDER_CE
	ON_WM_MOUSEACTIVATE()
#endif
	ON_WM_SIZE()
//	ON_WM_ERASEBACKGND()
END_MESSAGE_MAP()

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	OutputDebugString("PreCreateWindow");

	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
/*	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
*/
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_SIZEALL /*IDC_ARROW*/), NULL /*reinterpret_cast<HBRUSH>(COLOR_WINDOW+1)*/, NULL);
	return TRUE;
}

int CChildView::CreateEmulatorWindow(CWnd* _w)
{
	/*
	strMyClass = AfxRegisterWndClass(
      CS_VREDRAW | CS_HREDRAW,
      ::LoadCursor(NULL, IDC_ARROW),
      (HBRUSH) ::GetStockObject(WHITE_BRUSH),
      ::LoadIcon(NULL, IDI_APPLICATION));
*/

	outputInfo("CreateEmulatorWindow w:%X this:%X pEmulator:%X theEmulator:%X\n",_w,this,pEmulatorCtrl,CEmulator::theEmulator);
	CRect rect;
	parent = _w;
	_w->GetClientRect(&rect);
	outputInfo("Rect width:%d, height:%d\n",rect.Width(),rect.Height());
	
	  BOOL (__thiscall CWnd::* p )(LPCTSTR,LPCTSTR,DWORD,const RECT &,CWnd *,UINT,CCreateContext *) = &CChildView::Create;
	outputInfo("p=%X",p);

	int r = Create(NULL, _T("activegs"),  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN/*AFX_WS_DEFAULT_VIEW*/,
	rect,_w
		//	CRect(rect0, 0, 704, 460),
//	CRect(0, 0, 1000, 600),
, AFX_IDW_PANE_FIRST, NULL);
	SetFocus();
	return r;
}

extern int	g_window_width;
extern	int g_window_height;

void CChildView::resize()
{

	extern void window_needs_full_refresh();

	if (!parent) return ;
  RECT rcClient, rcWindow;
  RECT localClient, localWindow;
  POINT ptDiff;
  parent->GetClientRect( &rcClient);
  parent->GetWindowRect( &rcWindow);
GetClientRect( &localClient);
GetWindowRect( &localWindow); 

#ifdef VIDEO_SINGLEVLINE
	localClient.bottom *=2;
#endif
  ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
  ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
  int nWidth = rcClient.right - rcClient.left;
  int nHeight = rcClient.bottom - rcClient.top;
  MoveWindow(0/*rcWindow.left*/, 0/*rcWindow.top*/, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);

  window_needs_full_refresh();

}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	outputInfo("OnCreate CChildView:%X CurrentEmulator:%X\n",this,CEmulator::theEmulator);
	
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

#ifdef UNDER_CE
	DEVMODE mode;
	memset(&mode,0,sizeof(DEVMODE));
	mode.dmSize = sizeof(DEVMODE);

	mode.dmFields=DM_DISPLAYORIENTATION;
	mode.dmDisplayOrientation = DMDO_90;

	LONG Err= ChangeDisplaySettingsEx(NULL,&mode,NULL,0,NULL);
	if (Err!=DISP_CHANGE_SUCCESSFUL)
		outputInfo("failed to change display\n");
	#endif

	if (CEmulator::theEmulator)
	{
		outputInfo("Emulator was running: kill it (%X)\n",CEmulator::theEmulator);
		CEmulator::theEmulator->terminateEmulator();
		delete CEmulator::theEmulator;
	}
	resize();

	pEmulatorCtrl =  new CEmulatorWin32((CEmulator**)&this->pEmulatorCtrl);
	//id = pEmulatorCtrl->id;
	//outputInfo("new emulator (%X)(%X)(%d)\n",this,pEmulatorCtrl,CEmulator::theEmulator->id);
	hInst = lpCreateStruct->hInstance;
	CEmulatorConfig* pConfig = &config;
	
//	g_download.initPersistentPath();
	
	pEmulatorCtrl->initWindow(m_hWnd,hInst);
	pEmulatorCtrl->setConfig(pConfig); 
	pEmulatorCtrl->launchEmulator();

	return 0;
}

void CChildView::OnPaint() 
{
	
	if ( pEmulatorCtrl && pEmulatorCtrl == CEmulator::theEmulator )
	{

		CWnd::OnPaint();
	}
	else
	{
		PAINTSTRUCT ps; 
		CDC* cdc = BeginPaint(&ps); 
		RECT r;
		GetClientRect(&r);
		CBrush brush;
		brush.CreateSolidBrush(RGB(36,34,244));
		cdc->FillRect(&r,&brush);

#ifdef UNDER_CE
		cdc->DrawText( _T("PAUSED"), -1, &r, DT_CENTER|DT_SINGLELINE|DT_VCENTER);
#else
		cdc->SetTextColor(RGB(255,255,255));
		cdc->SetBkColor(RGB(36,34,244));

		int l = (r.bottom - r.top)/2 - 20;

		const char* line1="The emulator has been closed as another instance is running.";
		cdc->TextOut(100 ,l, line1,strlen(line1));
		const char line2[]="Refresh this window to re-activate the emulator.";
		cdc->TextOut(100 ,l+20, line2,strlen(line2));
#endif	
		EndPaint(&ps);
	}
}

void CChildView::OnDestroy()
{
	outputInfo("OnDestroy CChildView:%X theEmulator:%X\n",this,pEmulatorCtrl);

	if (pEmulatorCtrl)
	{
	//	ASSERT(CEmulator::theEmulator == pEmulatorCtrl);
		if (CEmulator::theEmulator == pEmulatorCtrl)
		{
			pEmulatorCtrl->terminateEmulator();
			delete pEmulatorCtrl;
			pEmulatorCtrl = NULL;
		}
		else
			outputInfo("ERR:OnDestroy but pEMulatorCtrl not owned by us ... CChildView:%X theEmulator:%X\n",this,pEmulatorCtrl);

	}
}

LRESULT CChildView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{


#if 0
	switch(message)
	{
		
		case WM_KEYUP:
			outputInfo("DefWindowProc: WM_KEYUP: %X\n",wParam);
			break;
		case WM_KEYDOWN:
			outputInfo("DefWindowProc: WM_KEYDOWN: %X\n",wParam);
			break;
		default:
			break;
	}
#endif
	if (CEmulator::theEmulator && pEmulatorCtrl == CEmulator::theEmulator)
		return pEmulatorCtrl->windowProc(message,wParam,lParam);
	else
		return CWnd::DefWindowProc(message, wParam, lParam);
	
}

 BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:

			if (pEmulatorCtrl && pEmulatorCtrl == CEmulator::theEmulator)
					pEmulatorCtrl->windowProc(pMsg->message,pMsg->wParam,pMsg->lParam);
			return 1;	
		/*
		case WM_LBUTTONDOWN:
			outputInfo("WM_LBUTTONDOWN\n");
			break;
		case WM_LBUTTONUP:
			outputInfo("WM_LBUTTONUP\n");
			break;
			*/
		default:
			break;
	}

	return 0; // to dispatch messag
}

 void CChildView::OnSetFocus(CWnd* pOldWnd )
 {
	// outputInfo("OnSetFocus\n");
	 CWnd::OnSetFocus(pOldWnd);
 }

 CWnd* CChildView::SetFocus()
 {
	// outputInfo("SetFocus\n");
	 CWnd* w = CWnd::SetFocus();
	return w;
 }

#ifndef UNDER_CE
 int CChildView::OnMouseActivate(CWnd* pDesktopWnd,UINT nHitTest,UINT message )
 {
//	 outputInfo("child OnMouseActivate\n");

 // Was used in the .OCX version
//	OnActivateInPlace(TRUE, NULL);  // OnActivateInPlace() is an undocumented function
	 return CWnd::OnMouseActivate(pDesktopWnd,nHitTest, message); 
//	 return MA_ACTIVATEANDEAT   ;
 }
#endif

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
 {
		return 1;
 }

void CChildView::OnSettingChange(UINT uFlags,LPCTSTR lpszSection)
 {
	 outputInfo("OnSettingChange");
 }

