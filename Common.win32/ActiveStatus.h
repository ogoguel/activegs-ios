#pragma once

//#include "activegsresrc.h"
#include "activecommonres.h"
// CActiveStatus dialog

class CActiveStatus : public CDialog
{
	DECLARE_DYNAMIC(CActiveStatus)

public:
	CActiveStatus(CWnd* pParent = NULL);   // standard constructor
	virtual ~CActiveStatus();
	UINT_PTR TimerID;

// Dialog Data
	enum { IDD = IDD_STATUS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};
