// TabOption.cpp : fichier d'implémentation
//

#include "../Common.win32/stdafx.h"
#include "activecommon.h"
#include "tabxml.h"



// Boîte de dialogue CTabOption

IMPLEMENT_DYNAMIC(CTabXML, CDialog)
CTabXML::CTabXML(CWnd* pParent /*=NULL*/)
	: CTabActiveInfo(CTabXML::IDD, pParent)
{
}

CTabXML::~CTabXML()
{
}

void CTabXML::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MULTIPLENAME, urlName);
	DDX_Control(pDX, IDC_MULTIPLEPUBLISHER, urlPublisher);

}


BEGIN_MESSAGE_MAP(CTabXML, CDialog)
	ON_BN_CLICKED(IDC_MULTIPLESWAP, OnBnClickedSwap)
END_MESSAGE_MAP()


/*
BOOL CTabOption::OnInitDialog()
{
	CDialog::OnInitDialog();
	initOptionDialog();
	return TRUE;
}
*/

void CTabXML::OnBnClickedSwap()
{
	CEmulator* emu = CEmulator::theEmulator ;
	if (!emu) return ;
	emu->smartSwap();
}


void CTabXML::refresh()
{
	CEmulator* emu = CEmulator::theEmulator;
	if (!emu) return ;
	
	/*
	const char * name = "*Custom Config*";
	const char * publisher = "";
	const char * year = "";
	const char * nameurl = NULL;
	const char * publisherurl = NULL;
	MyString swapdisk = "*not available*";
	int			swapdiskenable = 0;

	if (emu->config->pure)
	{
		name = emu->config->name.c_str();
		year = emu->config->year.c_str();
		publisher = emu->config->publisher.c_str();
		nameurl = emu->config->nameurl.c_str();
		publisherurl = emu->config->publisherurl.c_str();
		int smart = emu->getSmartSwap();
		if (smart)
		{
				CSlotInfo ref = emu->getLocalIMGInfo(smart/10,smart%10);
			if (ref.status == DELAYEDMOUNT)
				swapdisk.Format("*%s*",ref.shortname.c_str());
			else
				swapdisk = ref.shortname.c_str();
			swapdiskenable=1;
		}
	}
	CWnd* pText = GetDlgItem(IDC_MULTIPLENAME);
	if (nameurl && nameurl[0])
	{
		urlName.SetURL(nameurl);
		urlName.SetWindowText(name);
		if (pText)
			pText->SetWindowText(NULL);
	}
	else
	{
	if (pText)
			pText->SetWindowText(name);
	}

	if( publisherurl && publisherurl[0])
		urlPublisher.SetURL(publisherurl);
	urlPublisher.SetWindowText(publisher);
	

	pText = GetDlgItem(IDC_MULTIPLEYEAR);
	if (pText)
		pText->SetWindowText(year);
	
	pText = GetDlgItem(IDC_MULTIPLEPUBLISHER);
	if (pText)
		pText->SetWindowText(publisher);
	
	
	pText = GetDlgItem(IDC_MULTIPLECURRENT);
	if (pText)
		pText->SetWindowText(swapdisk.c_str());
	
	CWnd *swapbutton = GetDlgItem(IDC_MULTIPLESWAP);
	if (swapbutton)
		::EnableWindow(swapbutton->m_hWnd,swapdiskenable);
		*/

}