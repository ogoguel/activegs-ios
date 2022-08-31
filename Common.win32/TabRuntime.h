#pragma once
#include "sliderzip.h"
#include "../Kegs/Src/StdString.h"


// Boîte de dialogue CTabRuntime

class CTabRuntime : public CTabActiveInfo
{
	DECLARE_DYNAMIC(CTabRuntime)
	void getStateFilename(MyString& filename);
public:
	CTabRuntime(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CTabRuntime();
	bool	enableSpeedChange;

// Données de boîte de dialogue
	enum { IDD = IDD_TABRUNTIME };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeSpeedmode();
	CSliderZip m_SliderZip;
	afx_msg void OnCbnDropdownSpeedmode();
	afx_msg void OnCbnCloseupSpeedmode();
	BOOL OnNeedText( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	
	virtual void refresh();
	void setPauseText();
		afx_msg void OnCbnSelchangeJoystick();
	afx_msg void OnBnClickedConsolebutton();

	afx_msg void OnBnClickedButton1();

	afx_msg void OnBnClickedConsolebutton2();
	afx_msg void OnBnClickedSavestate();
	afx_msg void OnBnClickedRestorestate();
	afx_msg void OnCbnSelchangeVideofx();
	afx_msg void OnCbnSelchangeColormode();
	afx_msg void OnBnClickedConsolebutton3();
};
