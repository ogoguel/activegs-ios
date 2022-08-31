// TestRaster.cpp : Defines the entry point for the console application.
//


#include "../../common.win32/stdafx.h"
#include "../../Common/CemulatorCtrl.h"
#include "../src/raster.h"
#include "../src/graphcounter.h"

#ifdef USE_RASTER
#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

LPDIRECT3DDEVICE9 p_dx_Device = NULL;
LPDIRECT3D9 p_dx_Object = NULL;
int screenHeight;

int x_init_raster(void* _data)
{
	HWND hwnd = (HWND)_data;

 p_dx_Object = Direct3DCreate9(D3D_SDK_VERSION);
 if (p_dx_Object == NULL)
 {
    printf("DirectX Runtime library not installed!\n");
return 0;
 }

//	IDirect3DDevice9 * device;

D3DPRESENT_PARAMETERS dx_PresParams;
 
 ZeroMemory( &dx_PresParams, sizeof(dx_PresParams) );
 dx_PresParams.Windowed = TRUE;
 dx_PresParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
 dx_PresParams.BackBufferFormat = D3DFMT_UNKNOWN;

 
// HWND hWND=NULL;

 HRESULT err =p_dx_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &dx_PresParams, &p_dx_Device);
//HRESULT err = IDirect3D9::CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,NULL,0,NULL,&device);

if (err!=D3D_OK)
{
	printf("failed device");
	return 0;
}

	D3DRASTER_STATUS status;
	err =  p_dx_Device->GetRasterStatus(0,&status);
if (err!=D3D_OK)
{
	printf("failed getraster");
	return 0;
}

	screenHeight = GetSystemMetrics(SM_CYSCREEN) ;

	printf("screenHeight:%d\n",screenHeight);


	return 1;
}

int getScanLine()
{
	if (!p_dx_Device) return -1;

	D3DRASTER_STATUS status;
	HRESULT err =  p_dx_Device->GetRasterStatus(0,&status);
	if (err!=D3D_OK)
	{
		printf("failed getraster");
		return 0;
	}

	static int mins = 100000;
	static int maxs = 0;
	
	if (mins>status.ScanLine) mins=status.ScanLine;
	if (maxs<status.ScanLine) maxs=status.ScanLine;
//	if (status.InVBlank) __asm int 3;
	return status.ScanLine;

}

int getVbl()
{
	D3DRASTER_STATUS status;
	if (!p_dx_Device) return -1;
	HRESULT err =  p_dx_Device->GetRasterStatus(0,&status);
	if (err!=D3D_OK)
	{
		printf("failed getraster");
		return 0;
	}
	return status.InVBlank;

}

int x_wait_for_last_refresh(double _w)
{
#ifndef WIN32

#ifdef ENABLE_GRAPH
	double s = get_dtime();
#endif
CHANGE_BORDER(0,0xFFFFFF);
CHANGE_BORDER(2,0xFFFFFF);
		// attends la prochaine vbl
		//... x_wait_for_vbl();
CHANGE_BORDER(0,0x000000);
CHANGE_BORDER(2,0);
#ifdef ENABLE_GRAPH
		double d = get_dtime() - s;
		g_graph.add_graph(2,(float)d,g_sim65816.g_vbl_count);
#endif

#else
	while(1)
		{
			int l =getScanLine();
			if (l<screenHeight-20)
				break;
		}
		return 1;
#endif
}

int x_wait_for_vbl()
{
	
#ifdef ENABLE_GRAPH
	double s = get_dtime();
#endif
CHANGE_BORDER(0,0xFFFFFF);
CHANGE_BORDER(2,0xFFFFFF);
		// attends la prochaine vbl
	//	while(!getVbl());
		while(1)
		{
			int l =getScanLine();
			if (l>screenHeight-20)
				break;
		}
CHANGE_BORDER(0,0x000000);
CHANGE_BORDER(2,0);
#ifdef ENABLE_GRAPH
		double d = get_dtime() - s;
		g_graph.add_graph(2,(float)d,g_sim65816.g_vbl_count);
#endif // graph
	
		return 1;
}

#endif
