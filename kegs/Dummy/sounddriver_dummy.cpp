/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../Src/defc.h"
#include "../Src/sound.h"

//extern int g_audio_rate;

void x_snd_init(word32 *shmaddr)
{
	
}

void x_snd_shutdown()
{

}


void x_snd_child_init()
{

	set_audio_rate(g_sound.g_audio_rate);

}

 word32* x_sound_allocate(int size)
 {

     return NULL;
 }
 
 void x_sound_free(word32* mem)
 {

 }


int x_snd_send_audio(byte *ptr, int in_size)
{

	return in_size;
}

 void x_play_sound(enum_sound _hSound)
 {
 }
  