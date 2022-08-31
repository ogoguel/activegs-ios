// ActiveStatus.cpp : implementation file
//

#include "../common.win32/stdafx.h"
//#include "ActiveGS.h"
#include "ActiveStatus.h"
//#include "ActiveGSCtrl.h"

// CActiveStatus dialog

IMPLEMENT_DYNAMIC(CActiveStatus, CDialog)
CActiveStatus::CActiveStatus(CWnd* pParent /*=NULL*/)
	: CDialog(CActiveStatus::IDD, pParent)
{
	TimerID = NULL;
}

CActiveStatus::~CActiveStatus()
{
	
}

void CActiveStatus::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CActiveStatus, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CActiveStatus message handlers

void CActiveStatus::OnTimer(UINT_PTR nIDEvent)
{
	ShowWindow(SW_HIDE);
	CDialog::OnTimer(nIDEvent);
	TimerID = NULL;
}

void CActiveStatus::OnDestroy()
{
	if (TimerID)
		KillTimer(TimerID);
	TimerID = NULL;
}
