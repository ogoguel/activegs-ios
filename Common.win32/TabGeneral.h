#pragma once
#include "hyperlink.h"


// Boîte de dialogue CTabGeneral

class CTabGeneral : public CTabActiveInfo
{
	DECLARE_DYNAMIC(CTabGeneral)

public:
	CTabGeneral(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CTabGeneral();
	HCURSOR	hHand;
// Données de boîte de dialogue
	enum { IDD = IDD_TABGENERAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CHyperLink urlOG;
	virtual BOOL OnInitDialog();
	CHyperLink m_urlKD;
	CHyperLink urlTD;
	CHyperLink urlFTA;
	afx_msg void OnStnClickedUrlkd2();
};
