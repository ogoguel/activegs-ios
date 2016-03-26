/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "CEMulatorCtrl.h"
#include "../kegs/src/clock.h"
#include "../kegs/src/sim65816.h"
#include "../kegs/src/driver.h"


extern void addConsole(const char*);


void showStatus(const MYCHAR* format,...)
{
	
	va_list argptr;
	va_start( argptr, format );  
	MyString st;
	st.FormatV(format,argptr);
	va_end(argptr);

//	outputInfoInternal(st.c_str());

	CEmulator* emu = CEmulator::theEmulator;
	if (emu)
			emu->showStatus(st.c_str());
}

/*
void	showError(const char* format,...)
{

	va_list argptr;
	va_start( argptr, format );  
	MyString st;
	st.FormatV(format,argptr);
	va_end(argptr);

	outputInfoInternal(st.c_str());
}
*/


void showProgress(const char* file,int percent)
{
	MyString progress;
	if (percent>=0)
		progress.Format("Downloading: %s (%3d%%)",getfile(file),percent);
	else
		progress.Format("Downloading: %s (%4dK)",getfile(file),(int)(-percent/1024));
	
//	outputInfoInternal(progress.c_str());
	
	CEmulator* emu = CEmulator::theEmulator;
	if (emu)
		emu->showStatus(progress.c_str());
	
	
#ifdef DRIVER_IOS
	extern void x_downloadinginprogress(int);
	x_downloadinginprogress(percent);
#endif
}

void x_notify_motor_status(int _motorOn,int _slot,int _drive, int _curtrack)
{

	CEmulator* emu = CEmulator::theEmulator;
	if (emu)
	{
        emu->setLight(_motorOn,_slot,_drive,_curtrack);
		if (!option.getIntValue(OPTION_DONOTSHOWDISKSTATUS))
		{
			emu->loadingInProgress(_motorOn,_slot,_drive);
		}
	}
}

void x_clk_setup_bram_version()
{
	extern	void initializeBram(unsigned char*);
//	extern byte	g_bram[2][256];
	g_clock.g_bram_ptr = (&g_clock.g_bram[0][0]);	// ROM 01 only
	initializeBram(g_clock.g_bram_ptr);	
}





void x_config_load_additional_roms()
{
}

void x_notify_disk_ejection(int _slot,int _drive)
{
	CEmulator::theEmulator->ejectDisk(_slot,_drive);
}


void apply_patch(int _slot,int _disk,byte * _buf, int _pos, int _len)
{
	CEmulator* emu = CEmulator::theEmulator;
	CSlotInfo& ref = emu->getLocalIMGInfo(_slot,_disk);
	int t = ref.patch_addr - _pos;
	if (ref.patch_addr !=0 && t>=0 && t<_len)
	{
		const char*s = ref.patch_value;
		while(*s && t<_len)
		{
			byte a = *(s++)-'0';
			if (a>9) a-='A'-'0'-10;
			byte b = *(s++)-'0';
			if (b>9) b-='A'-'0'-10;
			_buf[t++]=a*16+b;
		}
	}
}


void apply_breakpoints()
{
    CEmulator* emu = CEmulator::theEmulator;
    if (!emu) return ;
    CEmulatorConfig* config = emu->config;
    for(int i=0;i<config->nb_bps;i++)
        set_bp(config->BPs[i].addr,config->BPs[i].mode,config->BPs[i].value,config->BPs[i].check);
}
