#pragma once


// Boîte de dialogue CSliderZip

class CSliderZip : public CSliderCtrl
{
	DECLARE_DYNAMIC(CSliderZip)

public:
	CSliderZip(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CSliderZip();

// Données de boîte de dialogue
	enum { IDD = IDD_TABRUNTIME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnNMReleasedcapture(NMHDR *pNMHDR, LRESULT *pResult);

protected:

};
