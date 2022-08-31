#pragma once


// Boîte de dialogue CTabOption

class CTabOption : public CTabActiveInfo
{
	DECLARE_DYNAMIC(CTabOption)

public:
	CTabOption(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CTabOption();

// Données de boîte de dialogue
	enum { IDD = IDD_TABOPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV
	void	initOptionDialog();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedNoiecache();
	afx_msg void OnBnClickedHaltbadread();
	afx_msg void OnBnClickedHaltbadaccess();
	afx_msg void OnBnClickedHaltevent();
	afx_msg void OnBnClickedDefaultoptions();
	afx_msg void OnBnClickedHaltbadread2();
	afx_msg void OnBnClickedDisablerewind();
};
