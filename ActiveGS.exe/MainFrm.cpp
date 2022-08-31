// MainFrm.cpp : implémentation de la classe CMainFrame
//

#include "../Common.win32/stdafx.h"
#include "ActiveGSApp.h"

#include "../common.win32/activecommon.h"
#include "../common/CEmulatorCtrl.h"
#include "mainfrm.h"
#include "../common/ki.h"

#include "../common/svnversion.h"
#include "../kegs/src/defcomm.h"
#include "../kegs/src/savestate.h"
#include "../kegs/src/driver.h"
#include "cgfiltyp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_SIZING()
END_MESSAGE_MAP()

void associateActiceGSConfig(const char* _ext, const char* _desc, int _icon)
{
		
	CGCFileTypeAccess TheFTA;

	// get full file path to program executable file

	TCHAR	szProgPath[MAX_PATH * 2];
	::GetModuleFileName(NULL, szProgPath, sizeof(szProgPath)/sizeof(TCHAR));

	CString csTempText;

	TheFTA.SetExtension(_ext);

	// just pass file path in quotes on command line

	csTempText  = szProgPath;
	csTempText += " \"%1\"";
	TheFTA.SetShellOpenCommand(csTempText);
	TheFTA.SetDocumentShellOpenCommand(csTempText);

	CString classname("ActiveGS.");
	classname += _ext;
	TheFTA.SetDocumentClassName(classname);

	TheFTA.SetDocumentDescription(_desc);

	// use first icon in program

	csTempText  = szProgPath;
	csTempText += ",";
	csTempText += (char)(_icon+'0');
	TheFTA.SetDocumentDefaultIcon(csTempText);

	// set the necessary registry entries	

	TheFTA.RegSetAllInfo();

}
CMainFrame::CMainFrame(int _argc, char** _argv)
{
	outputInfo("CMainFrame() (inst:%0X)\n",this);

	// Répertoire par default = pour une appli, le répertoire courant
	char windir[1024];
	GetCurrentDirectory(1024,windir);

#ifndef ACTIVEGSKARATEKA
	associateActiceGSConfig("activegsxml","ActiveGS Configuration file",0);
	associateActiceGSConfig("nib","Apple II 5.25 Disk Image",1);
	associateActiceGSConfig("dsk","Apple II 5.25 Disk Image",1);
	associateActiceGSConfig("2mg","Apple II 3.5 Image",2);
	associateActiceGSConfig("po","Apple II 3.5 Disk Image",2);
	associateActiceGSConfig("do","Apple  II 5.25 Disk Image",1);
#endif
	

	void (*driver)() = NULL;
#ifdef VIRTUALAPPLE
	extern void jniSetup(const char*);
	jniSetup(NULL);
#else
	#ifdef ACTIVEGSKARATEKA
		extern void karateka_driver();
		driver = karateka_driver;
	#else
		extern void activegs_driver();
		driver = activegs_driver;
	#endif
		g_driver.init( driver );
#endif

	CDownload::initPersistentPath();

	option.initOptions();


	m_wndView.config.setBaseURL(windir); 

#ifdef VIRTUALAPPLE
	m_wndView.config.xmlconfig =g_driver.activegsxml;
#else
	#ifdef ACTIVEGSPOP
		m_wndView.config.xmlconfig ="http://asset/pop.activegsxml";
	#elif ACTIVEGSKARATEKA
		m_wndView.config.xmlconfig ="http://asset/karateka.activegsxml";
	#else
		m_wndView.config.xmlconfig ="default.activegsxml";
		m_wndView.config.processCommandLine(_argc,_argv);
	#endif
#endif

	RECT r;
    r.left = 100;
    r.top = 100;
	r.right = r.left+X_A2_WINDOW_WIDTH;

#ifdef VIDEO_SINGLEHLINE
    float w = X_A2_WINDOW_WIDTH*2;
#else
    float w = X_A2_WINDOW_WIDTH;
#endif

	r.right = r.left + w * 2;


#ifdef VIDEO_SINGLEVLINE
    float h = X_A2_WINDOW_HEIGHT*2;
#else
	float h = X_A2_WINDOW_HEIGHT;
#endif

	r.bottom = r.top + h * 2;

	const char* ver = ACTIVEGSVERSIONSTR;
	
	Create(NULL, ver, WS_THICKFRAME | WS_SYSMENU,r, NULL,NULL, 0, NULL );



}

CMainFrame::~CMainFrame()
{

}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	outputInfo("OnCreate (inst:%0X)\n",lpCreateStruct->hInstance);
	
	m_wndView.CreateEmulatorWindow(this);
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;	
	return TRUE;
}

// diagnostics pour CMainFrame

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

// gestionnaires de messages pour CMainFrame

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// passe le focus à la fenêtre d'affichage
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	/*
	// laisser la priorité à la vue pour cette commande
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	*/
	// sinon, la gestion par défaut est utilisée
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
void CMainFrame::OnDestroy()
{
	outputInfo("OnDestroy() (inst:%0X)\n",this);
	CFrameWnd::OnDestroy();

}


extern void window_needs_full_refresh();

LRESULT CMainFrame::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{

	if (message==WM_SIZE)
	{
			m_wndView.resize(); 
			return 1;
	}
	else
	if (message==WM_ERASEBKGND)
	{
		// prevent mainframe to refresh its background
		return 1;
	}
	else
		return CFrameWnd::DefWindowProc(message, wParam, lParam);
}



void CMainFrame::OnSizing(UINT nSide,LPRECT lpRect)
{
	RECT rcClient, rcWindow;
    GetClientRect( &rcClient);
	GetWindowRect( &rcWindow);
	int borderw = (rcWindow.right - rcClient.right) + (rcClient.left - rcWindow.left );
	int borderh = (rcWindow.bottom - rcClient.bottom) + (rcClient.top - rcWindow.top );

	// détermine le sens du resize (en x ou en y)

	int deltax = (rcWindow.right-lpRect->right) + (rcWindow.left-lpRect->left);
	if (deltax<0) deltax = -deltax;
	int deltay = (rcWindow.bottom-lpRect->bottom) + (rcWindow.top-lpRect->top);
	if (deltay<0) deltay = -deltay;

#ifdef VIDEO_SINGLEVLINE
    int bottom =  X_A2_WINDOW_HEIGHT*2;
#else
     int bottom = X_A2_WINDOW_HEIGHT;
#endif
#ifdef VIDEO_SINGLEHLINE
    int right = X_A2_WINDOW_WIDTH*2;
#else
    int right = X_A2_WINDOW_WIDTH;
#endif
const float ratio = (float)right/(float)bottom;

	int w = (lpRect->right - lpRect->left) - borderw ;
	int h = (lpRect->bottom - lpRect->top) - borderh ;

	if (deltax || deltay)
	{
		if (nSide !=6)
			lpRect->bottom = lpRect->top + (int)(w/ratio) + borderh;
		else
			lpRect->right = lpRect->left + (int)(h*ratio) + borderw;
	}
  return CFrameWnd::OnSizing(nSide,lpRect);
}
