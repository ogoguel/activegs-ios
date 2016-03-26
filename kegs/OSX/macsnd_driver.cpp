/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifndef DRIVER_OSX
#error
#endif

/************************************************************************/
/*			KEGS: Apple //gs Emulator			*/
/*			Copyright 2002 by Kent Dickey			*/
/*									*/
/*		This code is covered by the GNU GPL			*/
/*									*/
/*	The KEGS web page is kegs.sourceforge.net			*/
/*	You may contact the author at: kadickey@alumni.princeton.edu	*/
/************************************************************************/

const char rcsid_macsnd_driver_c[] = "@(#)$KmKId: macsnd_driver.c,v 1.4 2003-10-17 15:57:40-04 kentd Exp $";

#include "defc.h"

#include <Carbon/Carbon.h>


#include "sound.h"
#include <unistd.h>

#define MACSND_REBUF_SIZE	(64*1024)
#define MACSND_QUANTA		512
/* MACSND_QUANTA must be >= 128 and a power of 2 */

word32	g_macsnd_rebuf[MACSND_REBUF_SIZE];
volatile word32 *g_macsnd_rebuf_ptr;
volatile word32 *g_macsnd_rebuf_cur;
volatile int g_macsnd_playing = 0;

extern int Verbose;

//extern int g_audio_rate;
extern word32 *g_sound_shm_addr;
extern int g_sound_size;



SndChannelPtr	g_snd_channel_ptr;
ExtSoundHeader	g_snd_hdr;
SndCommand	g_snd_cmd;

void
mac_snd_callback(SndChannelPtr snd_chan_ptr, SndCommand *in_sndcmd)
{
	OSStatus err;
	int	samps;

	// This is an interrupt routine--no printf, etc!

	samps = g_macsnd_rebuf_ptr - g_macsnd_rebuf_cur;
	if(samps < 0) {
		samps += MACSND_REBUF_SIZE;
	}

	samps = samps & -(MACSND_QUANTA);	// quantize to 1024 samples
	if(g_macsnd_rebuf_cur + samps > &(g_macsnd_rebuf[MACSND_REBUF_SIZE])) {
		samps = &(g_macsnd_rebuf[MACSND_REBUF_SIZE]) -
							g_macsnd_rebuf_cur;
	}
	if(samps > 0) {
		g_macsnd_playing = 1;
		g_snd_hdr.numFrames = samps;
		g_snd_hdr.loopEnd = samps;
		g_snd_hdr.samplePtr = (char *)g_macsnd_rebuf_cur; // OG Cast from byte* to ,char*

		g_snd_cmd.cmd = bufferCmd;
		g_snd_cmd.param1 = 0;
		g_snd_cmd.param2 = (long) &g_snd_hdr;

		g_macsnd_rebuf_cur += samps;
		if(g_macsnd_rebuf_cur >= &(g_macsnd_rebuf[MACSND_REBUF_SIZE])) {
			g_macsnd_rebuf_cur -= MACSND_REBUF_SIZE;
		}

		err = SndDoImmediate(g_snd_channel_ptr, &g_snd_cmd);

		// And set-up callback
		g_snd_cmd.cmd = callBackCmd;
		g_snd_cmd.param1 = 0;
		g_snd_cmd.param2 = 0;
		err = SndDoCommand(g_snd_channel_ptr, &g_snd_cmd, TRUE);
	} else {
		g_macsnd_playing = 0;
	}
}

int x_snd_send_audio(byte *ptr, int in_size)
{
	SndCommand snd_cmd = {0};
	word32	*wptr, *macptr;
	word32	*eptr;
	int	samps;
	int	i;

	samps = in_size / 4;
	wptr = (word32 *)ptr;
	macptr = (word32 *)g_macsnd_rebuf_ptr;
	eptr = &g_macsnd_rebuf[MACSND_REBUF_SIZE];
	for(i = 0; i < samps; i++) {
		*macptr++ = *wptr++;
		if(macptr >= eptr) {
			macptr = &g_macsnd_rebuf[0];
		}
	}

	g_macsnd_rebuf_ptr = macptr;

	if(!g_macsnd_playing) {
		mac_snd_callback(g_snd_channel_ptr, &snd_cmd);
	}

	return in_size;
}

void x_snd_child_init()
{
	OSStatus	err;

	mac_printf("In mac child\n");
	fflush(stdout);
	mac_printf("pid: %d\n", getpid());
	fflush(stdout);

	//return;

	//g_snd_channel_ptr = 0;
	err = SndNewChannel(&g_snd_channel_ptr, sampledSynth, initStereo,
			NewSndCallBackUPP(mac_snd_callback));
	mac_printf("SndNewChannel ret: %d\n", (int)err);
	fflush(stdout);

	memset(&g_snd_hdr, 0, sizeof(g_snd_hdr));
	g_snd_hdr.sampleSize = 16;
	g_snd_hdr.numChannels = 2;
	g_sound.g_audio_rate = 44100;
	g_snd_hdr.sampleRate = g_sound.g_audio_rate << 16;
	g_snd_hdr.numFrames = 0;	// will be set in mac_send_audio
	g_snd_hdr.encode = extSH;
	g_snd_hdr.baseFrequency = 0;
	g_snd_hdr.samplePtr = 0;

	set_audio_rate(g_sound.g_audio_rate);

	mac_printf("End of child_sound_init_mac\n");
	fflush(stdout);
}

void x_snd_init(word32 *shmaddr)
{
	g_macsnd_rebuf_cur = &g_macsnd_rebuf[0];
	g_macsnd_rebuf_ptr = &g_macsnd_rebuf[0];
	mac_printf("macsnd_init called\n");
	//child_sound_loop(-1, -1, shmaddr);
}

void x_snd_shutdown()
{
}



word32* x_sound_allocate(int size)
{
	word32	*shmaddr;
	shmaddr = (word32*)malloc(size);
	memset(shmaddr, 0, size);
	return shmaddr;
}


void x_sound_free(word32* _mem)
{
	if (_mem)
		free(_mem);
	
}

void x_async_sound_init()
{
}

void x_async_snd_shutdown()
{
}
