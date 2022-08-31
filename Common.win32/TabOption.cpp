// TabOption.cpp : fichier d'implémentation
//

#include "../Common.win32/stdafx.h"
#include "activecommon.h"
#include "taboption.h"



// Boîte de dialogue CTabOption

IMPLEMENT_DYNAMIC(CTabOption, CDialog)
CTabOption::CTabOption(CWnd* pParent /*=NULL*/)
	: CTabActiveInfo(CTabOption::IDD, pParent)
{
}

CTabOption::~CTabOption()
{
}

void CTabOption::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTabOption, CDialog)
	ON_BN_CLICKED(IDC_NOIECACHE, OnBnClickedNoiecache)
	ON_BN_CLICKED(IDC_HALTBADREAD, OnBnClickedHaltbadread)
	ON_BN_CLICKED(IDC_HALTBADACCESS, OnBnClickedHaltbadaccess)
	ON_BN_CLICKED(IDC_HALTEVENT, OnBnClickedHaltevent)
	ON_BN_CLICKED(IDC_DEFAULTOPTIONS, OnBnClickedDefaultoptions)
	ON_BN_CLICKED(IDC_DISABLEREWIND, &CTabOption::OnBnClickedDisablerewind)
END_MESSAGE_MAP()



BOOL CTabOption::OnInitDialog()
{
	CDialog::OnInitDialog();
	initOptionDialog();
	return TRUE;
}

void CTabOption::initOptionDialog()
{
	
	CButton *pButton = (CButton*)GetDlgItem(IDC_NOIECACHE);
	pButton->SetCheck(1); //pEmu->option.getIntValue(OPTION_PERSISTENTDIR));
	pButton->EnableWindow(0);
	pButton = (CButton*)GetDlgItem(IDC_HALTBADREAD);
	
	pButton->SetCheck(option.getIntValue(OPTION_HALTONBADREAD));
	
	pButton = (CButton*)GetDlgItem(IDC_HALTBADACCESS);
	pButton->SetCheck(option.getIntValue(OPTION_HALTONBADACC));

	pButton = (CButton*)GetDlgItem(IDC_HALTEVENT);
	pButton->SetCheck(option.getIntValue(OPTION_HALTONHALTS));
	
	pButton = (CButton*)GetDlgItem(IDC_ENABLEREWIND);
	pButton->SetCheck(option.getIntValue(OPTION_ENABLEREWIND));

	CWnd *pWnd = (CWnd*)GetDlgItem(IDC_PERSISTENT);
	
	pWnd->SetWindowText(CDownload::getPersistentPath());
}

void CTabOption::OnBnClickedNoiecache()
{
}


void CTabOption::OnBnClickedHaltbadread()
{
	// TODO : ajoutez ici le code de votre gestionnaire de notification de contrôle
	CButton *pButton = (CButton*)GetDlgItem(IDC_HALTBADREAD);
	if (!pButton) return ;
	int ck = pButton->GetCheck();
	option.setIntValue(OPTION_HALTONBADREAD,ck);
	option.saveOptions(1);
	
}

void CTabOption::OnBnClickedHaltbadaccess()
{
	// TODO : ajoutez ici le code de votre gestionnaire de notification de contrôle
	CButton *pButton = (CButton*)GetDlgItem(IDC_HALTBADACCESS);
	if (!pButton) return ;
	int ck = pButton->GetCheck();
	option.setIntValue(OPTION_HALTONBADACC,ck);
	option.saveOptions(1);
	
}

void CTabOption::OnBnClickedHaltevent()
{
	// TODO : ajoutez ici le code de votre gestionnaire de notification de contrôle
	CButton *pButton = (CButton*)GetDlgItem(IDC_HALTEVENT);
	if (!pButton) return ;
	int ck = pButton->GetCheck();
	option.setIntValue(OPTION_HALTONHALTS,ck);
	option.saveOptions(1);
	
}

void CTabOption::OnBnClickedDefaultoptions()
{
	option.setDefaultOptions();
	option.saveOptions(1);
	initOptionDialog();
}


void CTabOption::OnBnClickedDisablerewind()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_ENABLEREWIND);
	if (!pButton) return ;
	int ck = pButton->GetCheck();
	option.setIntValue(OPTION_ENABLEREWIND,ck);
	option.saveOptions(1);
}
