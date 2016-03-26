/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

void mac_snd_callback(SndChannelPtr snd_chan_ptr, SndCommand *in_sndcmd);
int mac_send_audio(byte *ptr, int in_size);
void child_sound_init_mac(void);
void macsnd_init(word32 *shmaddr);

