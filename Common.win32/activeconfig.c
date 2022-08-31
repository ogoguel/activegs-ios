
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "../src/defc.h"
#include "ki.h"

extern Iwm	iwm;

void (*ki_loadingptr)(int,int,int) = NULL;

void prepareDriveInfo(Disk* dsk,int slot, int drive)
{
	int l = dsk->image_size;
	int s = dsk->image_start ;
	int p = (dsk->image_type == DSK_TYPE_PRODOS);
	readImageInfo(slot,drive+1,l,s,p);
}

int toggleJoystick(int _force)
 {
	 if (_force==-2)
	 {
		 // auto
		 	joystick_init();	// ? Joy
	 }
	 else
	 if (_force==-1)
	 {
		 if (g_joystick_type == JOYSTICK_MOUSE) 
			 g_joystick_type = JOYSTICK_NONE;
		 else
		 if (g_joystick_type == JOYSTICK_NONE) 
		 {
			joystick_init();	// ? Joy
			// can be mouse if not detected
		 }
		 else
		 {
			 // joystick_win32
			g_joystick_type=JOYSTICK_MOUSE;
		 }
	 }
	 else
	 {
		 g_joystick_type = _force; 
		if (g_joystick_type == JOYSTICK_WIN32_1) 
			   joystick_init();
	 }

    if (g_joystick_type == JOYSTICK_MOUSE)
		printf("joystick set to mouse emulation\n");
	else
		if (g_joystick_type == JOYSTICK_NONE)
		printf("joystick set to none\n");
	else
		printf("joystick set to native\n");

	refreshInfo();
	return 1;
}


#ifndef _CONSOLE

/*
void	ejectDisk(int slot,int disk)
{
	setLocalIMG(slot,disk,NULL);
}
*/


void checkImages()
{
	extern	int	g_highest_smartport_unit ;
	extern Iwm	iwm;

	if (mountImages())
	{
		printf("Mounting images...\n");
		
		g_highest_smartport_unit = -1;
			
		insert_disk(5,0,getLocalIMG(5,1),0,0,0,0);
		insert_disk(5,1,getLocalIMG(5,2),0,0,0,0);
		insert_disk(6,0,getLocalIMG(6,1),0,0,0,0);
		insert_disk(6,1,getLocalIMG(6,2),0,0,0,0);
	
		insert_disk(7,0,getLocalIMG(7,1),0,0,0,0);
		insert_disk(7,1,getLocalIMG(7,2),0,0,0,0);
	
		refreshInfo();

	}
}
/*
extern void ejectDiskCtrl(int _slot,int _drive);
void ejectDisk(int _slot,int _drive)
{
	if (!g_inMount)
		ejectDiskCtrl(_slot,_drive);
}
*/

void config_init() 
{ 
	/*
#ifndef _USRDLL
	insert_disk(5,0,"nucleus.2mg",0,0,0,0);
	insert_disk(6,0,"flobynoid (2002).dsk",0,0,0,0);
#endif
	*/
}

char bram_default[]={
(char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x0D, (char)0x06, (char)0x02, (char)0x01, (char)0x01, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x00,
(char)0x00, (char)0x00, (char)0x07, (char)0x06, (char)0x02, (char)0x01, (char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x0F, (char)0x06, (char)0x06, (char)0x00, (char)0x05, (char)0x06,
(char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x03, (char)0x02, (char)0x02, (char)0x02,
(char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06,
(char)0x07, (char)0x00, (char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06, (char)0x07, (char)0x08, (char)0x09, (char)0x0A, (char)0x0B, (char)0x0C, (char)0x0D,
(char)0x0E, (char)0x0F, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFD, (char)0x96, (char)0x57, (char)0x3C
};

void updateBramCheckSum(unsigned char* buf)	// original code in FF/B61D
{
	register short int crc=0;			// LDA #0
	int i;									// CLC
	for(i=255-4-1;i>=0;i--)						// LDX #FA
	{
		crc = (crc << 1) | ((crc>>15) & 1);	// ROL (1-bits)
#ifdef UNDER_CE
		crc += buf[i];
		crc += buf[i+1]*256;
#else
		crc += *(unsigned short*)(buf+i);	// ADC bram,X
#endif
	}									    // DEX
											// CPX #FF
											// BNE Loop
	*(unsigned short*)(buf+252) = crc;		// TAX
	*(unsigned short*)(buf+254) = crc ^0xAAAA;	// EOR #AAAA
}


void initializeBram(byte* g_bram_ptr)
{
	memcpy(g_bram_ptr,bram_default,256);
	updateBramParameters(g_bram_ptr);
	updateBramCheckSum(g_bram_ptr);
	
}
#else

void config_init() 
{
//	insert_disk(7,0,"system6.2mg",0,0,0,0);
//	insert_disk(7,0,"C:\\Work\\GS\\Images\\GSOS_6.2MG",0,0,0,0);
//	insert_disk(7,0,"OOTW.2MG",0,0,0,0);

//	insert_disk(7,0,"C:\\Work\\GS\\Images\\NFCDemoDrive.2mg",0,0,0,0);

	insert_disk(5,0,"nucleus.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\ACS_Demo2.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\StarWizard(OK).2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\BeamDemo.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\Bestof2vol1.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\MODULAE.2MG",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\delta.2MG",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\TFORCE1.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\manoir.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\BOUNCIN_FERNO.2MG",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\Rastan with Intro Disk 1.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\Aaargh!.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\CaliforniaDemo.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\xmasdemo.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\nodrugs.2mg",0,0,0,0);
//	insert_disk(5,0,"C:\\Work\\GS\\Images\\Star_Trek_Classic.2mg",0,0,0,0);

//	insert_disk(5,0,"C:\\Work\\GS\\Images\\shark.2mg",0,0,0,0);
	
//	insert_disk(6,0,"C:\\Work\\GS\\Images\\flobynoid (2002).dsk",0,0,0,0);
//	insert_disk(6,0,"C:\\Work\\GS\\Images\\Ultima III - Exodus (Master) (Disk 1 of 2).zip_0_Ultima III - Exodus (Master) (Disk 1 of 2)[1].dsk",0,0,0,0);
//	insert_disk(6,0,"C:\\Work\\GS\\Images\\karateka.dsk",0,0,0,0);

}

#endif



void monitorStatus(int motorOn,int drive,int select)
{
	if (ki_loadingptr)
		ki_loadingptr(motorOn,drive,select);
	
}

#ifndef UNDER_CE
char targetSpeed[256];
char estimatedSpeed[256];

void	updateInfo(const char* target,const char *speed)
{
	strcpy(targetSpeed,target);
	strcpy(estimatedSpeed,speed);

}
#endif


// SIM86516

void fatalExit(int ret)
{
	char str[255];
	unsigned short strw[1024];
	sprintf(str,"Critical Error : %d",ret);

#ifndef UNDER_CE
	MessageBox(NULL,str,"ActiveGS Halted!",MB_OK);
#else
	
	MultiByteToWideChar(CP_ACP, 0,str,strlen(str),strw,1024);
  
	MessageBox(NULL,strw,_T("ActiveGS Halted!"),MB_OK);
#endif
	ExitThread(ret);
}