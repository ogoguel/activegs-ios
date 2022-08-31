/************************************************************************/
/*			KEGS: Apple //gs Emulator			*/
/*			Copyright 2002 by Kent Dickey			*/
/*									*/
/*		This code is covered by the GNU GPL			*/
/*									*/
/*	The KEGS web page is kegs.sourceforge.net			*/
/*	You may contact the author at: kadickey@alumni.princeton.edu	*/
/************************************************************************/

const char rcsid_scc_windriver_c[] = "@(#)$KmKId: scc_windriver.c,v 1.4 2004-11-19 02:00:46-05 kentd Exp $";

/* This file contains the Win32 COM1/COM2 calls */

#include "../src/defc.h"
#include "../src/scc.h"

#ifdef UNDER_CE
#define vsnprintf _vsnprintf
#endif


extern word32 g_c025_val;

#ifdef _WIN32
int
scc_serial_win_init(int port)
{
	COMMTIMEOUTS commtimeouts;
	TCHAR	str_buf[8];
	Scc	*scc_ptr;
	HANDLE	host_handle;
	int	state;
	int	ret;

	scc_ptr = &(g_scc.scc_stat[port]);

	scc_ptr->state = 0;		/* mark as failed */

#ifdef UNICODE
	wsprintf(&str_buf[0], _T("COM%d"), port+1);
#else
	sprintf(&str_buf[0], "COM%d", port+1);
#endif
	host_handle = CreateFile(&str_buf[0], GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, 0, NULL);

	scc_ptr->host_handle = host_handle;
	scc_ptr->host_handle2 = malloc(sizeof(DCB));

	printf("scc_socket_init %d called, host_handle: %p\n", port,
				host_handle);

	if(host_handle == INVALID_HANDLE_VALUE) {
		scc_ptr->host_handle = 0;
		return 0;
	}

	scc_serial_win_change_params(port);


	commtimeouts.ReadIntervalTimeout = MAXDWORD;
	commtimeouts.ReadTotalTimeoutMultiplier = 0;
	commtimeouts.ReadTotalTimeoutConstant = 0;
	commtimeouts.WriteTotalTimeoutMultiplier = 0;
	commtimeouts.WriteTotalTimeoutConstant = 10;
	ret = SetCommTimeouts(host_handle, &commtimeouts);
	if(ret == 0) {
		printf("setcommtimeout ret: %d\n", ret);
	}

	state = 2;	/* raw serial */
	scc_ptr->state = state;

	return state;
}

void
scc_serial_win_change_params(int port)
{
	DCB	*dcbptr;
	HANDLE	host_handle;
	Scc	*scc_ptr;
	int	ret;

	scc_ptr = &(g_scc.scc_stat[port]);

	host_handle = scc_ptr->host_handle;
	dcbptr = (DCB*)scc_ptr->host_handle2;	// OG Added cast
	if(host_handle == 0) {
		return;
	}

	ret = GetCommState(host_handle, dcbptr);
	if(ret == 0) {
		printf("getcomm port%d ret: %d\n", port, ret);
	}

#if 1
	printf("dcb.baudrate: %d, bytesize:%d, stops:%d, parity:%d\n",
		(int)dcbptr->BaudRate, (int)dcbptr->ByteSize,
		(int)dcbptr->StopBits, (int)dcbptr->Parity);
	printf("dcb.binary: %d, ctsflow: %d, dsrflow: %d, dtr: %d, dsr: %d\n",
		(int)dcbptr->fBinary,
		(int)dcbptr->fOutxCtsFlow,
		(int)dcbptr->fOutxDsrFlow,
		(int)dcbptr->fDtrControl,
		(int)dcbptr->fDsrSensitivity);
	printf("dcb.txonxoff:%d, outx:%d, inx: %d, null: %d, rts: %d\n",
		(int)dcbptr->fTXContinueOnXoff,
		(int)dcbptr->fOutX,
		(int)dcbptr->fInX,
		(int)dcbptr->fNull,
		(int)dcbptr->fRtsControl);
	printf("dcb.fAbortOnErr:%d, fParity:%d\n", (int)dcbptr->fAbortOnError,
		(int)dcbptr->fParity);
#endif

	dcbptr->fAbortOnError = 0;

	dcbptr->BaudRate = scc_ptr->baud_rate;
	dcbptr->ByteSize = scc_ptr->char_size;
	dcbptr->StopBits = ONESTOPBIT;
	switch((scc_ptr->reg[4] >> 2) & 0x3) {
	case 2: // 1.5 stop bits
		dcbptr->StopBits = ONE5STOPBITS;
		break;
	case 3: // 2 stop bits
		dcbptr->StopBits = TWOSTOPBITS;
		break;
	}

	dcbptr->Parity = NOPARITY;
	switch((scc_ptr->reg[4]) & 0x3) {
	case 1:	// Odd parity
		dcbptr->Parity = ODDPARITY;
		break;
	case 3:	// Even parity
		dcbptr->Parity = EVENPARITY;
		break;
	}

	dcbptr->fNull = 0;
	dcbptr->fDtrControl = DTR_CONTROL_ENABLE;
	dcbptr->fDsrSensitivity = 0;
	dcbptr->fOutxCtsFlow = 0;
	dcbptr->fOutxDsrFlow = 0;
	dcbptr->fParity = 0;
	dcbptr->fInX = 0;
	dcbptr->fOutX = 0;
	dcbptr->fRtsControl = RTS_CONTROL_ENABLE;

	ret = SetCommState(host_handle, dcbptr);
	if(ret == 0) {
		printf("SetCommState ret: %d, new baud: %d\n", ret,
			(int)dcbptr->BaudRate);
	}
}

void
scc_serial_win_fill_readbuf(int port, int space_left, double dcycs)
{
	byte	tmp_buf[256];
	Scc	*scc_ptr;
	HANDLE	host_handle;
	DWORD	bytes_read;
	DWORD	i;
	int	ret;

	scc_ptr = &(g_scc.scc_stat[port]);

	host_handle = scc_ptr->host_handle;
	if(host_handle == 0) {
		return;
	}

	/* Try reading some bytes */
	space_left = MIN(256, space_left);
	ret = ReadFile(host_handle, tmp_buf, space_left, &bytes_read, NULL);

	if(ret == 0) {
		printf("ReadFile ret 0\n");
	}

	if(ret && (bytes_read > 0)) {
		for(i = 0; i < bytes_read; i++) {
			scc_add_to_readbuf(port, tmp_buf[i], dcycs);
		}
	}
	
}

void
scc_serial_win_empty_writebuf(int port)
{
	Scc	*scc_ptr;
	HANDLE	host_handle;
	int	rdptr;
	int	wrptr;
	int	done;
	word32	err_code;
	DWORD	bytes_written;
	int	ret;
	int	len;

	scc_ptr = &(g_scc.scc_stat[port]);

	//printf("win_empty_writebuf, host_handle: %d\n", scc_ptr->host_handle);
	host_handle = scc_ptr->host_handle;
	if(host_handle == 0) {
		return;
	}

	/* Try writing some bytes */
	done = 0;
	while(!done) {
		rdptr = scc_ptr->out_rdptr;
		wrptr = scc_ptr->out_wrptr;
		if(rdptr == wrptr) {
			//printf("...rdptr == wrptr\n");
			done = 1;
			break;
		}
		len = wrptr - rdptr;
		if(len < 0) {
			len = SCC_OUTBUF_SIZE - rdptr;
		}
		if(len > 32) {
			len = 32;
		}
		if(len <= 0) {
			done = 1;
			break;
		}
		bytes_written = 1;
		ret = WriteFile(host_handle, &(scc_ptr->out_buf[rdptr]), len,
				&bytes_written, NULL);
		printf("WriteFile ret: %d, bytes_written:%d, len:%d\n", ret,
			(int)bytes_written, len);

		err_code = (word32)-1;
		if(ret == 0) {
			err_code = (word32)GetLastError();
			printf("WriteFile ret:0, err_code: %08x\n", err_code);
		}

		if(ret == 0 || (bytes_written == 0)) {
			done = 1;
			break;
		} else {
			rdptr = rdptr + bytes_written;
			if(rdptr >= SCC_OUTBUF_SIZE) {
				rdptr = rdptr - SCC_OUTBUF_SIZE;
			}
			scc_ptr->out_rdptr = rdptr;
		}
	}
}

#endif
