// SliderZip.cpp : fichier d'implémentation
//

#include "../common.win32/stdafx.h"
#include "activecommon.h"
#include "SliderZip.h"
#include "../kegs/src/moremem.h"

// Boîte de dialogue CSliderZip

IMPLEMENT_DYNAMIC(CSliderZip, CSliderCtrl)
CSliderZip::CSliderZip(CWnd* pParent /*=NULL*/)
	: CSliderCtrl()
{
}

CSliderZip::~CSliderZip()
{
}

void CSliderZip::DoDataExchange(CDataExchange* pDX)
{
	CSliderCtrl::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSliderZip, CSliderCtrl)
ON_NOTIFY_REFLECT(NM_RELEASEDCAPTURE, OnNMReleasedcapture)
END_MESSAGE_MAP()

// Gestionnaires de messages CSliderZip

void CSliderZip::OnNMReleasedcapture(NMHDR *pNMHDR, LRESULT *pResult)
{
	g_moremem.g_zipgs_reg_c05a = ((15-(GetPos() & 0x0F))<< 4 )|(g_moremem.g_zipgs_reg_c05a&0x0F);
	*pResult = 0;
}

