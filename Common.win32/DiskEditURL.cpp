// DiskEditURL.cpp : implementation file
//

#include "../Common.win32/stdafx.h"
//#include "ActiveGSApp.h"
#include "activecommon.h"
#include "DiskEditURL.h"
#include <Windowsx.h>

// CDiskEditURL dialog

IMPLEMENT_DYNAMIC(CDiskEditURL, CDialog)

CDiskEditURL::CDiskEditURL(CWnd* pParent /*=NULL*/)
	: CTabActiveInfo(CDiskEditURL::IDD, pParent)
{
	selectedItem=0;
}

CDiskEditURL::~CDiskEditURL()
{

}

BOOL CDiskEditURL::OnInitDialog()
{
	selectedItem = 0;
	CDialog::OnInitDialog();
	/*
	DragAcceptFiles(TRUE);
	CDialog* edit = (CDialog*)GetDlgItem(IDC_EDITURL);
	ASSERT(edit);
	edit->DragAcceptFiles(TRUE);
	*/
	return TRUE;
}

void CDiskEditURL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
void CDiskEditURL::OnDropFiles()
{
	/*
	HDROP hDrop = (HDROP)wParam;
	char filename[MAX_PATH];
	DragQueryFile(hDrop,    // Struture Identifier

        0,        // -1 to Drop more than one file or ( integer 0 to max )

                   // to drop selected No of files

        filename,// Droped File Name

        MAX_PATH);   // Max char 
	*/	
}


BEGIN_MESSAGE_MAP(CDiskEditURL, CDialog)
	//	 ON_MESSAGE(WM_DROPFILES,OnDropFiles)// Message Handler for Drang and Drop
	ON_CONTROL(WM_DROPFILES,IDC_EDITURL,OnDropFiles)
//	ON_CONTROL(EN_UPDATE,IDC_EDITURL,OnDropFiles)
ON_BN_CLICKED(IDC_MOUNTBUTTON, &CDiskEditURL::OnBnClickedMountbutton)
ON_BN_CLICKED(IDC_EJECT, &CDiskEditURL::OnBnClickedEject)
ON_BN_CLICKED(IDC_RELOAD, &CDiskEditURL::OnBnClickedReload)
ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SLOTLIST, &CDiskEditURL::OnLvnItemActivateSlotlist)
ON_NOTIFY(HDN_ITEMCLICK, 0, &CDiskEditURL::OnHdnItemclickSlotlist)
ON_BN_CLICKED(IDC_REBOOT5, &CDiskEditURL::OnBnClickedReboot5)
ON_BN_CLICKED(IDC_REBOOT6, &CDiskEditURL::OnBnClickedReboot6)
ON_BN_CLICKED(IDC_REBOOT7, &CDiskEditURL::OnBnClickedReboot7)
ON_BN_CLICKED(IDC_MOUNTBUTTON, &CDiskEditURL::OnBnClickedMountbutton)
ON_CBN_SELCHANGE(IDC_COMBOSLOT, &CDiskEditURL::OnCbnSelchangeComboslot)
ON_BN_CLICKED(IDC_FILESELECT, &CDiskEditURL::OnBnClickedFileselect)
ON_BN_CLICKED(IDC_SWAP, &CDiskEditURL::OnBnClickedSwap)
END_MESSAGE_MAP()


// CDiskEditURL message handlers

void CDiskEditURL::OnBnClickedMountbutton()
{
	// TODO: Add your control notification handler code here
	CDialog* edit = (CDialog*)GetDlgItem(IDC_EDITURL);
	char path[MAX_PATH];
	int l = Edit_GetLine(edit->m_hWnd,0,path,MAX_PATH);
	path[l]=0;
	
	int slot,drive;
	if (getCurrentSlotAndDrive(slot,drive))
	{
		CEmulator::theEmulator->config->setSlot(slot,drive,path,REPLACECURRENT);
	}
}
void CDiskEditURL::OnBnClickedEject()
{
	
	int slot,drive;
	if (!getCurrentSlotAndDrive(slot,drive)) return ;	
	CEmulator::theEmulator->ejectDisk(slot,drive);
	
}

void CDiskEditURL::OnBnClickedReload()
{
	int slot,drive;
	if (!getCurrentSlotAndDrive(slot,drive)) return ;
	CEmulator::theEmulator->reloadDisk(slot,drive);
}

void CDiskEditURL::OnLvnItemActivateSlotlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	/*
	CListCtrl *pList = (CListCtrl*)GetDlgItem(IDC_SLOTLIST);
	ASSERT(pList);
	POSITION p = pList->GetFirstSelectedItemPosition();
	if (!p) return ;
	int i =pList->GetNextSelectedItem(p);
	selectedItem = pList->GetItemData(i);
	refreshItem();
	*/
	*pResult = 0;
}

int CDiskEditURL::getCurrentSlotAndDrive(int& slot, int &drive)
{
	if (selectedItem==-1) return 0;
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSLOT);
	int data = pCombo->GetItemData(selectedItem);
	slot = data / 10;
	drive = data %10;
	return 1;
}

void CDiskEditURL::refresh()
{

	CListCtrl *pList = (CListCtrl*)GetDlgItem(IDC_SLOTLIST);
	if (pList)
		{
		pList->DeleteAllItems();
		int nbi=0;
		for(int slot=5;slot<=7;slot++)
		{
			int maxdrive=2;
			if (slot==7) maxdrive=MAXSLOT7DRIVES;
			for(int disk=1;disk<=maxdrive;disk++)
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
					sizeStr.Format(_T("%4dMb"),info.size/(1024*1024));
				else
					sizeStr.Format(_T("%4dKo"),info.size/1024);
					
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
	}

//	int slot = selectedItem;
	BOOL beject = TRUE;
	BOOL breload = TRUE;
	BOOL bmount = TRUE;
	BOOL bSwap = FALSE;

	int slot,drive;
	if (getCurrentSlotAndDrive(slot,drive))
	{
		const CSlotInfo& info = CEmulator::theEmulator->getLocalIMGInfo(slot,drive);
		SetDlgItemText(IDC_EDITURL,info.url);
		switch(info.status)
		{
		case UNDEFINED:
			beject=FALSE;
			breload=FALSE;
			break;
		case FAILED:
			beject=FALSE;
			breload=TRUE;
			break;
		case EJECTED:
			beject=FALSE;
			breload=TRUE;
			break;
		case READY2MOUNT:
		case DELAYEDMOUNT:
			beject=TRUE;
			breload=FALSE;
			break;
		case MOUNTED:
			beject=TRUE;
			breload=FALSE;
		}

		int sw  = CEmulator::theEmulator->getSmartSwap();
		if ( ((sw/10) == slot) && ( (sw%10) == drive ) ) // OG A UNIFIER!!!!
			bSwap = TRUE;
	}
	else
	{
			beject=FALSE;
			breload=FALSE;
	}

	::EnableWindow(GetDlgItem(IDC_EJECT)->m_hWnd,beject);
	::EnableWindow(GetDlgItem(IDC_RELOAD)->m_hWnd,breload);
	::EnableWindow(GetDlgItem(IDC_MOUNTBUTTON)->m_hWnd,bmount);
	::EnableWindow(GetDlgItem(IDC_SWAP)->m_hWnd,bSwap);
}	
	

void CDiskEditURL::OnHdnItemclickSlotlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CDiskEditURL::OnBnClickedReboot5()
{
	CEmulator::theEmulator->onBootSlotChanged(5);
}

void CDiskEditURL::OnBnClickedReboot6()
{
	CEmulator::theEmulator->onBootSlotChanged(6);
}

void CDiskEditURL::OnBnClickedReboot7()
{
	CEmulator::theEmulator->onBootSlotChanged(7);}



void CDiskEditURL::OnCbnSelchangeComboslot()
{
	// TODO: Add your control notification handler code here
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSLOT);
	selectedItem = pCombo->GetCurSel();
	refresh();
//	SetDlgItemText(IDC_EDITURL,l_strFileName);
}

void CDiskEditURL::OnBnClickedFileselect()
{ 
	
//	    CFileDialog l_SampleDlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT,"2MG Images (*.img)|*.2mg|II Images (*.dsk)|*.dsk|II Images (*.nib)|*.nib|Archives (*.zip)|*.zip||");
	CFileDialog l_SampleDlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT,"Disk Image (*.img;*.dsk;*.nib;*.bin;*.zip)||");

		int iRet = l_SampleDlg.DoModal();
      CString l_strFileName;
      l_strFileName = l_SampleDlg.GetPathName();

      if(iRet == IDOK)	
	  {
		//	CDialog* edit = (CDialog*)GetDlgItem(IDC_EDITURL);
			SetDlgItemText(IDC_EDITURL,l_strFileName);
			OnBnClickedMountbutton();
		 // MessageBox(l_strFileName);
		//	  MessageBox(l_strFileName);
    
	  }
	/*	
	  else
          MessageBox("No File Selected!");
 */
}

void CDiskEditURL::OnBnClickedSwap()
{
//	int slot = selectedItem;
	int slot,drive;
	if (!getCurrentSlotAndDrive(slot,drive)) return ;
	CEmulator::theEmulator->swapDisk(slot,drive);
}
