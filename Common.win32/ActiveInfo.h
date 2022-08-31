#pragma once
#include "afxcmn.h"
#include "hyperlink.h"
#include "activecommonres.h"

class CTabActiveInfo : public CDialog
{
public:
	CTabActiveInfo(UINT IDD, CWnd* pParent /*=NULL*/)
		: CDialog(IDD, pParent) {}


	virtual	void refresh() {} ;
};

// Boîte de dialogue CActiveInfo

class CActiveInfo : public CDialog
{
	DECLARE_DYNAMIC(CActiveInfo)

public:
	CActiveInfo(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CActiveInfo();

// Données de boîte de dialogue
	enum { IDD = IDD_ACTIVEINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
//	CActiveTab m_InfoTab;
	CTabActiveInfo*	pTabDialog;
	UINT	uTimer;
	virtual BOOL OnInitDialog();
	afx_msg void OnTcnSelchangeActivetab(NMHDR *pNMHDR, LRESULT *pResult);
	void	updateRuntimeSpeed();
//	void	updateRuntimeImages();
	void	updateParams();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void freeTabDialog();
	afx_msg void OnDestroy();

	void	refresh();
};
