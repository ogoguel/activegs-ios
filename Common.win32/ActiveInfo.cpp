// ActiveInfo.cpp : fichier d'implémentation
//

#include "../common.win32/stdafx.h"
#include "activeinfo.h"
#include "tabgeneral.h"
#include "taboption.h"
#include "tabruntime.h"
#include "tabxml.h"
#include "diskediturl.h"
#include "activecommon.h"
#include "../common/svnversion.h"
#include "../kegs/src/moremem.h"
#include "../kegs/src/sim65816.h"

// Boîte de dialogue CActiveInfo

IMPLEMENT_DYNAMIC(CActiveInfo, CDialog)
CActiveInfo::CActiveInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CActiveInfo::IDD, pParent)
{
	pTabDialog = NULL;
	uTimer=0;
	
}

CActiveInfo::~CActiveInfo()
{
	
}

void CActiveInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_ACTIVETAB, m_InfoTab);
//	DDX_Control(pDX, IDC_URLOG, urlOG);
}


BEGIN_MESSAGE_MAP(CActiveInfo, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_ACTIVETAB, OnTcnSelchangeActivetab)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CActiveInfo::freeTabDialog()
{
	if (pTabDialog)
	{
		if (uTimer)	KillTimer(uTimer);
		uTimer=0;
		pTabDialog->DestroyWindow();
		delete pTabDialog;
		pTabDialog=NULL;
		
	}
}


// Gestionnaires de messages CActiveInfo

BOOL CActiveInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	int i=0;
	CTabCtrl* infoTab = (CTabCtrl*)GetDlgItem(IDC_ACTIVETAB);
	if (infoTab)
	{
		//infoTab->InsertItem(i++,"Image")	;
		infoTab->InsertItem(i++,"Disk Conf.");
		infoTab->InsertItem(i++,"Runtime Info");
		infoTab->InsertItem(i++,"Options");
		infoTab->InsertItem(i++,"Variables");
		infoTab->InsertItem(i++,"Help");
		infoTab->InsertItem(i++,"Credits");
		OnTcnSelchangeActivetab(NULL,NULL);
	}
	else
		outputInfo("getdlgitem failed: %d",GetLastError());
	/*
	m_InfoTab.InsertItem(1,"Runtime Info");
	m_InfoTab.InsertItem(2,"Console");
	*/
	
	// change le titre
	SetWindowText(ACTIVEGSVERSIONSTR);
	//char s[1024];
	//GetWindowText(hWnd,s,1024);
	CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}

extern int g_zipgs_reg_c05a;


void CActiveInfo::OnTcnSelchangeActivetab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTabCtrl* infoTab = (CTabCtrl*)GetDlgItem(IDC_ACTIVETAB);
	//static	const int sel2id[]={ IDD_TABGENERAL,IDD_TABOPTION,IDD_TABRUNTIME1,IDD_TABDISKCONF,IDD_TABPARAM,IDD_TABHELP,0};
	static	const int sel2id[]={	
							//		IDD_TABXML,	
									IDD_TABDISKCONF,	
									IDD_TABRUNTIME1,
									IDD_TABOPTION,
									IDD_TABPARAM,
									IDD_TABHELP,
									IDD_TABGENERAL,
									0};
	int sel = infoTab->GetCurSel();
	int id = sel2id[sel];
	
	freeTabDialog();

	
	if (!id) return;

	CRect m_ClientRect,WindowRect;

	// Determine the size of the area for the contents
	infoTab->GetClientRect(&m_ClientRect);
	infoTab->AdjustRect(FALSE, &m_ClientRect);
	
	// Determine the offset within the view's client area

	infoTab->GetWindowRect(&WindowRect);
	infoTab->ScreenToClient(WindowRect);
	m_ClientRect.OffsetRect(WindowRect.left, WindowRect.top);

	
	switch(id)
	{
	case	IDD_TABGENERAL:
		pTabDialog = new CTabGeneral(infoTab);
		break;
	case	IDD_TABOPTION:
		pTabDialog = new CTabOption(infoTab);
		break;
	case	IDD_TABRUNTIME1:
		pTabDialog = new CTabRuntime(infoTab);
		break;
	case	IDD_TABDISKCONF:
		pTabDialog = new CDiskEditURL(infoTab);
		break;
	case	IDD_TABXML:
		pTabDialog = new CTabXML(infoTab);
		break;
	default:
		pTabDialog = new CTabActiveInfo(id,infoTab);
	}
			
	pTabDialog->Create(id,infoTab);

	switch(id)
	{
	case IDD_TABDISKCONF:
		{
		
		CComboBox *pCombo = (CComboBox*)pTabDialog->GetDlgItem(IDC_COMBOSLOT);
		ASSERT(pCombo);
		CDiskEditURL* pDialogDisk = (CDiskEditURL*)pTabDialog;
		MyString s;
		int id;
		int err;
		for(int slot=5;slot<=7;slot++)
		{
			int maxdrive=2;
			if (slot==7) maxdrive=MAXSLOT7DRIVES;
			for(int disk=1;disk<=maxdrive;disk++)
			{
				s.Format("S%dD%d",slot,disk);
				id = slot*10+disk ;
				err = pCombo->InsertString(-1,s.c_str());	
				pCombo->SetItemData(err,id);
			}
		}

		int d = pDialogDisk->selectedItem ;
		if (d==-1) d=0;
		pCombo->SetCurSel(d);
		pDialogDisk->OnCbnSelchangeComboslot();
		
		CListCtrl *pList = (CListCtrl*)pTabDialog->GetDlgItem(IDC_SLOTLIST);
		pList->InsertColumn(0,"Slot");
		pList->SetColumnWidth(0,40);
		/*
		pList->InsertColumn(1,"(E)");
		pList->SetColumnWidth(1,10);
		*/
		pList->InsertColumn(1,"Filename");
		pList->SetColumnWidth(1,100);
		pList->InsertColumn(2,"Size");
		pList->SetColumnWidth(2,50);
		pList->InsertColumn(3,"Prefix");
		pList->SetColumnWidth(3,120);
		/*
		pList->InsertColumn(5,"Path");
		pList->SetColumnWidth(5,1000);
		*/
	//	updateRuntimeImages();
		}
		break;
	case IDD_TABRUNTIME1:
		{
		CSliderCtrl *pSlider = (CSliderCtrl*)pTabDialog->GetDlgItem(IDC_ZIPSLIDER);
		pSlider->SetRange(0,15);
		pSlider->SetPos(15-(g_moremem.g_zipgs_reg_c05a>>4)&0x0F);
		updateRuntimeSpeed();	
		uTimer = SetTimer(1, 1000, 0);

		break;
		}
	case IDD_TABPARAM:
		{
		CListCtrl *pList = (CListCtrl*)pTabDialog->GetDlgItem(IDC_PARAMLIST);
		pList->InsertColumn(0,"Name");
		pList->SetColumnWidth(0,80);
		pList->InsertColumn(1,"Value");
		pList->SetColumnWidth(1,160);
		pList->InsertColumn(2,"Hex");
		pList->SetColumnWidth(2,80);
		updateParams();
		break;
		}
	case IDD_TABOPTION:
		{
			
		}
		break;
	case	IDD_TABXML:
		break;	
	
	case IDD_TABGENERAL:
		{
		
			CWnd *pText = (CWnd*)pTabDialog->GetDlgItem(IDC_BUILD);
			if (pText)
			{
				const char * builddate = "Built on " ACTIVEGSDATE ;
					pText->SetWindowText(builddate);
			}
			pText = (CWnd*)pTabDialog->GetDlgItem(IDC_VERSION);
			if (pText)
			{
					const char * builddate = ACTIVEGSVERSIONSTRFULL;
					pText->SetWindowText(builddate);
			}
	
			
		}
		break;
	default:
		break;
	}

	
	refresh();
	pTabDialog->MoveWindow(m_ClientRect);
	pTabDialog->ShowWindow(SW_SHOW);

	if (pResult)
		*pResult = 0;
	
}

/*
void CActiveInfo::updateRuntimeImages()
{
	CListCtrl *pList = (CListCtrl*)pTabDialog->GetDlgItem(IDC_SLOTLIST);
	if (!pList) return ;

	pList->DeleteAllItems();

	int nbi=0;
	for(int slot=5;slot<=7;slot++)
		for(int disk=1;disk<=2;disk++)
			for(int active=0;active<ACTIVEGSMAXIMAGE;active++)
			{
			 CSlotInfo info;
			 CEmulator* emu=CEmulator::theEmulator;
			int ret = emu->getLocalMultipleIMGInfo(slot,disk,info,active);
			if (!ret) continue;
			if (info.status==UNDEFINED || info.status ==NOTHING) continue;

			MyString str;
			str.Format("S%dD%d",slot,disk);
			if (ret==1)
					str +="*";
			pList->InsertItem(nbi,str.c_str());
		//	pList->SetItemData(nbi,(slot-5);
		//	pList->SetItemText(nbi,1,"e");

			
			MyString shortname (info.shortname.c_str());
			
			MyString sizeStr;
			if (info.size>1*1024*1024)
				sizeStr.Format("%4dMb",info.size/(1024*1024));
			else
				sizeStr.Format("%4dKo",info.size/1024);
				
			MyString prefix;
			switch(info.status)
			{
				case MOUNTED:
					prefix = info.prefix.c_str();
					break;
				case EJECTED:
					if (!info.filename.IsEmpty())
						prefix = "*EJECTED*";
					break;
				case READY2MOUNT:
					prefix = "*READY2MOUNT*";
					shortname = getfile(info.url);
					break;
				case DELAYEDMOUNT:
					prefix = "*DELAYEDMOUNT*";
					shortname = getfile(info.url);
					break;
				case FAILED:
					prefix	="*FAILED*";
					break;
			}
			pList->SetItemText(nbi,1,shortname.c_str());
			pList->SetItemText(nbi,2,sizeStr.c_str());		
			pList->SetItemText(nbi,3,prefix.c_str());

		//	pList->SetItemText(nbi,5,info.name);

			nbi++;
			
		}
	
}
*/



//extern char targetSpeed[256];
extern char estimatedSpeed[256];
//extern int g_speed_fast;

void CActiveInfo::updateRuntimeSpeed()
{

	CWnd* speedTab = (CWnd*)pTabDialog->GetDlgItem(IDC_SPEED);	
	if (!speedTab) return ;
	
	CString speed;	// pour AppendFormat
	speed.Format("%s MHz ",estimatedSpeed);
	
	if (g_sim65816.g_speed_fast)
	{
//		speed.AppendFormat("(%s MHz)",targetSpeed);
	}
	else
		speed.Append("(1.0 MHz*)");

	speedTab->SetWindowText(speed);

	CComboBox *pCombo = (CComboBox*)pTabDialog->GetDlgItem(IDC_SPEEDMODE);
	if (pCombo && ( ((CTabRuntime*)pTabDialog)->enableSpeedChange))
	{
		CEmulator* emu = CEmulator::theEmulator ;
		int	sel = pCombo->GetCurSel();
		int speed = pCombo->GetItemData(sel);
		int sp = g_sim65816.get_limit_speed();
		if (sp != speed)
		{
			namevalue& nv = option.find(OPTION_SPEED);
			int ispeed = nv.getOrder(g_sim65816.get_limit_speed());
			pCombo->SetCurSel(ispeed);
			printf("resetting speed to %s (%d)\n",nv.getDescription(sp),sp);
		}
	}

	
	CSliderCtrl *pSlider = (CSliderCtrl*)pTabDialog->GetDlgItem(IDC_ZIPSLIDER);
	if (pSlider)
		pSlider->SetPos(15-(g_moremem.g_zipgs_reg_c05a>>4)&0x0F);
	
}

void CActiveInfo::OnTimer(UINT_PTR nIDEvent)
{
	updateRuntimeSpeed();
	CDialog::OnTimer(nIDEvent);
}

void CActiveInfo::updateParams()
{
	CListCtrl *pList = (CListCtrl*)pTabDialog->GetDlgItem(IDC_PARAMLIST);
	if (!pList) return ; 
	pList->DeleteAllItems();
	CEmulator* emu=CEmulator::theEmulator;
	int pos = 0;
	for(int i=0;i<emu->nbparams;i++)
	{
		pList->InsertItem(pos,emu->params[i].data.c_str());
		pList->SetItemText(pos,1,emu->params[i].value.c_str());
		MyString hex;
		hex.Format("(%X)",emu->params[i].hex);
		pList->SetItemText(pos,2,hex.c_str());
		pos++;
	}
	for(int i=0;i<NB_OPTIONS;i++)
	{
		const char* name = option.options[i].def->name;
		pList->InsertItem(pos,name);
		MyString desc; option.getStrValue(desc,(option_id)i);
		pList->SetItemText(pos,1,desc.c_str());
		if (option.options[i].def->convertTableInt)
		{
			MyString hex;
			hex.Format("(%d)",option.options[i].intvalue);
			pList->SetItemText(pos,2,hex.c_str());
		
		}
		pos++;
	}
}

void CActiveInfo::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO : ajoutez ici le code de votre gestionnaire de messages
	freeTabDialog();
}

void CActiveInfo::refresh()
{
	if (pTabDialog)
		pTabDialog->refresh();
	
}



