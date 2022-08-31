#pragma once


// CDiskEditURL dialog

class CDiskEditURL : public CTabActiveInfo
{
	DECLARE_DYNAMIC(CDiskEditURL)

public:
	CDiskEditURL(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDiskEditURL();
	int		selectedItem;
// Dialog Data
	enum { IDD = IDD_TABDISKCONF };
	virtual void  refresh();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void  OnDropFiles();

	BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMountbutton();
	afx_msg void OnBnClickedEject();
	afx_msg void OnBnClickedReload();
	afx_msg void OnLvnItemActivateSlotlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemclickSlotlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedReboot5();
	afx_msg void OnBnClickedReboot6();
	afx_msg void OnBnClickedReboot7();
//	afx_msg void OnBnClickedMountbutton();
	afx_msg void OnCbnSelchangeComboslot();
	afx_msg void OnBnClickedFileselect();
	afx_msg void OnBnClickedSwap();
	int getCurrentSlotAndDrive(int& slot, int &drive);
};
