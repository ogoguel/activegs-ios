// MainFrm.h : interface de la classe CMainFrame
//


#pragma once

#include "../Common.win32/ChildView.h"
//#include "../common32/activecommon.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame(int _argc, char ** _argv);
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributs
public:
	CChildView    m_wndView;

// Opérations
public:

// Substitutions
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implémentation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // membres incorporés de la barre de contrôle
	/*
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	*/
// Fonctions générées de la table des messages
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnSizing(UINT nSide,LPRECT lpRect);
public:
	afx_msg void OnDestroy();
};


