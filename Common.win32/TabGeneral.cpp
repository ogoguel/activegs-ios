// TabGeneral.cpp : fichier d'implémentation
//

#include "../Common.win32/stdafx.h"
#include "activecommon.h"
#include "tabgeneral.h"


// Boîte de dialogue CTabGeneral

IMPLEMENT_DYNAMIC(CTabGeneral, CDialog)
CTabGeneral::CTabGeneral(CWnd* pParent /*=NULL*/)
	: CTabActiveInfo(CTabGeneral::IDD, pParent)
{
	HCURSOR	hHand = NULL;
}

CTabGeneral::~CTabGeneral()
{
	if (hHand)
		DeleteObject(hHand);
}

void CTabGeneral::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URLOG, urlOG);
	DDX_Control(pDX, IDC_URLKD, m_urlKD);
//	DDX_Control(pDX, IDC_URLTD, urlTD);
	DDX_Control(pDX, IDC_URLFTA, urlFTA);
}


BEGIN_MESSAGE_MAP(CTabGeneral, CDialog)
	ON_STN_CLICKED(IDC_URLKD2, &CTabGeneral::OnStnClickedUrlkd2)
END_MESSAGE_MAP()


// Gestionnaires de messages CTabGeneral

BOOL CTabGeneral::OnInitDialog()
{
	CDialog::OnInitDialog();

	CEmulatorWin32* pEmu = (CEmulatorWin32*)CEmulator::theEmulator;
	if (pEmu)
		hHand = LoadCursor(pEmu->hInst,MAKEINTRESOURCE(IDC_CURSOR1));
	

	m_urlKD.SetURL("mailto:kadickey@alumni.princeton.edu");
	m_urlKD.SetLinkCursor(hHand);
	urlOG.SetURL("mailto:o@goguel.com");
	urlOG.SetLinkCursor(hHand);

	//	urlTD.SetURL("mailto:tdracz@artpol.com.pl");
	urlFTA.SetURL("http://activegs.freetoolsassociation.com");
	urlFTA.SetLinkCursor(hHand);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}



void CTabGeneral::OnStnClickedUrlkd2()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}
