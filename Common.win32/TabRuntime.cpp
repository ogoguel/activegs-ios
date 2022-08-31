// TabRuntime.cpp : fichier d'implémentation
//

#include "../Common.win32/stdafx.h"
#include "activecommon.h"
#include "tabruntime.h"
#include "../common/ki.h"
#include "sliderzip.h"
#include "../kegs/src/defcomm.h"
#include "../kegs/src/moremem.h"
#include "../kegs/src/paddles.h"
#include "../kegs/src/sim65816.h"
#include "../kegs/src/saveState.h"
#include "../kegs/src/video.h"

// Boîte de dialogue CTabRuntime

IMPLEMENT_DYNAMIC(CTabRuntime, CDialog)


CTabRuntime::CTabRuntime(CWnd* pParent /*=NULL*/)
	: CTabActiveInfo(CTabRuntime::IDD, pParent)
{
}

CTabRuntime::~CTabRuntime()
{
}

void CTabRuntime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ZIPSLIDER, m_SliderZip);
}


BEGIN_MESSAGE_MAP(CTabRuntime, CDialog)
	ON_CBN_SELCHANGE(IDC_SPEEDMODE, OnCbnSelchangeSpeedmode)
	ON_CBN_DROPDOWN(IDC_SPEEDMODE, OnCbnDropdownSpeedmode)
	ON_CBN_CLOSEUP(IDC_SPEEDMODE, OnCbnCloseupSpeedmode)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnNeedText )
		ON_CBN_SELCHANGE(IDC_JOYSTICK, OnCbnSelchangeJoystick)
	ON_BN_CLICKED(IDC_CONSOLEBUTTON3, OnBnClickedConsolebutton)
	ON_BN_CLICKED(IDC_PAUSE, OnBnClickedButton1)

	ON_BN_CLICKED(IDC_CONSOLEBUTTON2, &CTabRuntime::OnBnClickedConsolebutton2)
	ON_BN_CLICKED(IDC_SAVESTATE, &CTabRuntime::OnBnClickedSavestate)
	ON_BN_CLICKED(IDC_RESTORESTATE, &CTabRuntime::OnBnClickedRestorestate)
	ON_CBN_SELCHANGE(IDC_VIDEOFX, &CTabRuntime::OnCbnSelchangeVideofx)
	ON_CBN_SELCHANGE(IDC_COLORMODE, &CTabRuntime::OnCbnSelchangeColormode)
END_MESSAGE_MAP()

// Gestionnaires de messages CTabRuntime


void fillCombo(CComboBox *_combo,option_id _id,int _cur)
{
	int pos=0;
	int i=0;
	const char* s;

	namevalue& nv = option.find(_id);
	while(s=nv.def->convertTableDesc[i])
	{
		_combo->InsertString(pos,s);	
		_combo->SetItemData(pos,nv.def->convertTableInt[i]);
		pos++;
		i++;
	}
	int o = nv.getOrder(_cur);
	_combo->SetCurSel(o);
}


BOOL CTabRuntime::OnInitDialog()
{
	CDialog::OnInitDialog();

	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_SPEEDMODE);
	fillCombo(pCombo,OPTION_SPEED,g_sim65816.get_limit_speed());
	
	/*CComboBox * */pCombo = (CComboBox*)GetDlgItem(IDC_COLORMODE);
	fillCombo(pCombo,OPTION_COLORMODE,r_sim65816.get_color_mode());

/*CComboBox * */pCombo = (CComboBox*)GetDlgItem(IDC_VIDEOFX);
	fillCombo(pCombo,OPTION_VIDEOFX,r_sim65816.get_video_fx());
	
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem(IDC_ZIPSLIDER);
	pSlider->SetRange(0,15);
	pSlider->SetPos(g_moremem.g_zipgs_reg_c05a&0x0F);

	CComboBox *pComboJoy = (CComboBox*)GetDlgItem(IDC_JOYSTICK);
	fillCombo(pComboJoy,OPTION_JOYSTICKMODE,g_joystick_type);

	CComboBox *pComboState = (CComboBox*)GetDlgItem(IDC_STATE);
	for(int i=0;i<5;i++)
	{
		MyString str;
		
		int state_id = i;
		if (state_id==0)
			str  = "Mem";
		else
			str.Format("#%d",state_id);
		pComboState->InsertString(i,str.c_str());	
		pComboState->SetItemData(i,state_id);
	}
	pComboState->SetCurSel(0);

	enableSpeedChange=true;
	OnCbnSelchangeSpeedmode();
	refresh();
	setPauseText();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}

void CTabRuntime::OnCbnSelchangeSpeedmode()
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_SPEEDMODE);
	int sel = pCombo->GetCurSel();
	int sp = pCombo->GetItemData(sel);
	CEmulator* emu = CEmulator::theEmulator ;
	emu->onSpeedChanged(sp);

	if (enableSpeedChange)
	{
		CWnd *pSlider = (CWnd*)GetDlgItem(IDC_ZIPSLIDER);
		pSlider->EnableWindow(g_sim65816.get_limit_speed() == SPEED_ZIP?SW_SHOW:SW_HIDE);
	}

}

void CTabRuntime::OnCbnDropdownSpeedmode()
{

	enableSpeedChange=false;
}

void CTabRuntime::OnCbnCloseupSpeedmode()
{
	enableSpeedChange=true;
}

BOOL CTabRuntime::OnNeedText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

		CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem(IDC_ZIPSLIDER);
		if (pSlider)
		{
			CString string;
			string.Format("%3d%%", (pSlider->GetPos()+1)*100/16);
			::lstrcpy(pTTT->szText, (LPCTSTR) string);
			return TRUE;
		}
		return FALSE;    

}

void CTabRuntime::refresh()
{
	CComboBox *pComboJoy = (CComboBox*)GetDlgItem(IDC_JOYSTICK);
	namevalue& nv = option.find(OPTION_JOYSTICKMODE);
	int sel = nv.getOrder(g_joystick_type);
	pComboJoy->SetCurSel(sel);
}


int toggleJoystick(int _force);

void CTabRuntime::OnCbnSelchangeJoystick()
{
	CComboBox *pComboJoy = (CComboBox*)GetDlgItem(IDC_JOYSTICK);
	int sel = pComboJoy->GetCurSel();
	int joy = pComboJoy->GetItemData(sel);

	option.setIntValue(OPTION_JOYSTICKMODE,joy);

	outputInfo("switching to %s\n",option.getDescription(OPTION_JOYSTICKMODE,joy));
	
	option.saveOptions(1);
	refresh();


}


void CTabRuntime::OnBnClickedConsolebutton()
{
	CConsole* c = ((CEmulatorWin32*)CEmulator::theEmulator)->theConsole;
	if (c)
		c->toggleConsole();
}



void CTabRuntime::setPauseText()
{
	CWnd *pWnd = (CWnd*)GetDlgItem(IDC_PAUSE);
	if (pWnd)
	{/*
		if (r_sim65816.get_state() == IN_PAUSE )
			pWnd->SetWindowText("Resume");
		else
			pWnd->SetWindowText("Pause");
			*/
		pWnd->SetWindowText("Pause/Resume");
	}
}
void CTabRuntime::OnBnClickedButton1()
{
	if (r_sim65816.get_state()==IN_PAUSE)
		r_sim65816.resume();
	else
	if (r_sim65816.get_state()==RUNNING)
		r_sim65816.pause();
	else
	{
		printf("emulator not running nor in pause...\n");
		return ;
	}
	setPauseText();

}

void CTabRuntime::OnBnClickedConsolebutton2()
{
	CEmulator*pEmu = CEmulator::theEmulator;
	if (!pEmu)
		return ;

	MyString ret;
	pEmu->processCommand("screenshot",ret);
	::MessageBox(NULL,ret.c_str(),"Screenshot saved",MB_OK);
}


void CTabRuntime::getStateFilename(MyString& filename)
{
		CComboBox *pComboState = (CComboBox*)GetDlgItem(IDC_STATE);
	int s = pComboState->GetCurSel();
	int id = pComboState->GetItemData(s);
	if (id==0)
		return ;
	 
	 filename = CDownload::getPersistentPath();
	filename += ACTIVEGS_DIRECTORY_SEPARATOR;

	MyString f;
	f.Format( "state_%d.state",id);
	filename += f;

}

void CTabRuntime::OnBnClickedSavestate()
{
	MyString path;
	
	getStateFilename(path);
	g_savestate.saveState(path.c_str());

}

void CTabRuntime::OnBnClickedRestorestate()
{
	MyString path;
	getStateFilename(path);
	g_savestate.restoreState(path.c_str());
	
}



void CTabRuntime::OnCbnSelchangeVideofx()
{
	CComboBox *pComboJoy = (CComboBox*)GetDlgItem(IDC_VIDEOFX);
	int vfx = pComboJoy->GetCurSel();
	int vfxdata = pComboJoy->GetItemData(vfx);
	option.setIntValue(OPTION_VIDEOFX,vfxdata);
	option.saveOptions(1);
		x_refresh_video();
}

void CTabRuntime::OnCbnSelchangeColormode()
{
	CComboBox *pComboJoy = (CComboBox*)GetDlgItem(IDC_COLORMODE);
	int cm = pComboJoy->GetCurSel();
	int cmdata = pComboJoy->GetItemData(cm);
	option.setIntValue(OPTION_COLORMODE,cmdata);
	option.saveOptions(1);
	x_refresh_video();
	

	
}

