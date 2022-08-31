
// ChildView.h : interface of the CChildView class
//


#pragma once
#include "../common.win32/activecommon.h"
#include "../common/ki.h"


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();
	
// Attributes
public:
	CEmulatorWin32*		pEmulatorCtrl;
	int				id;
	HINSTANCE		hInst;
	CEmulatorConfig	config;
	CWnd*	parent;
// Operations
public:
	int			CreateEmulatorWindow(CWnd* _w);
// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd );
	afx_msg void OnDestroy();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd,UINT nHitTest,UINT message );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSettingChange(UINT uFlags,LPCTSTR lpszSection);

	virtual CWnd*SetFocus();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void	resize();

	// Generated message map functions
//protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

