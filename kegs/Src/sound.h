/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#ifdef INCLUDE_RCSID_C
const char rcsid_sound_h[] = "@(#)$KmKId: sound.h,v 1.17 2003-11-21 15:15:57-05 kentd Exp $";
#endif

#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(DRIVER_ANDROID)
# include <sys/ipc.h>
# include <sys/shm.h>
#endif

#define SOUND_SHM_SAMP_SIZE		(32*1024)

#define SAMPLE_SIZE		2
#define NUM_CHANNELS		2
#define SAMPLE_CHAN_SIZE	(SAMPLE_SIZE * NUM_CHANNELS)

#define MAX_SND_BUF	65536
#define VAL_C030_RANGE		(32768)
#define VAL_C030_BASE		(-16384)

#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

STRUCT(Doc_reg) {
	_ALIGNED(8)	double	dsamp_ev;
	_ALIGNED(8)	double	dsamp_ev2;
	_ALIGNED(8)	double	complete_dsamp;
	int	samps_left;
	word32	cur_acc;
	word32	cur_inc;
	word32	cur_start;
	word32	cur_end;
	word32	cur_mask;
	int	size_bytes;
	int	event;
	int	running;
	int	has_irq_pending;
	word32	freq;
	word32	vol;
	word32	waveptr;
	word32	ctl;
	word32	wavesize;
	word32	last_samp_val;


} XCODE_PRAGMA_PACK;

/* prototypes for hardware specific snd_driver.c functions */
void	x_snd_init(word32 *);
void	x_snd_shutdown();
void	x_snd_child_init();
int		x_snd_send_audio(byte *ptr, int size);
word32* x_sound_allocate(int size);

#define MAX_C030_TIMES		18000

class s_sound : public serialized
{
public:

	byte doc_ram[0x10000 + 16];

	word32 doc_sound_ctl ;
	word32 doc_saved_val ;
	int	g_doc_num_osc_en ;
	_ALIGNED(8)	double	g_dcycs_per_doc_update ;
	_ALIGNED(8)	double	g_dupd_per_dcyc ;
	_ALIGNED(8)	double	g_drecip_osc_en_plus_2 ;

	int	g_doc_saved_ctl ;
	int	g_queued_samps ;
	int	g_queued_nonsamps ;
	int	g_num_osc_interrupting ;

	int	g_audio_enable;

	Doc_reg g_doc_regs[32];

	word32 doc_reg_e0 ;

	int	g_audio_rate ;		// OG Preferred Audio Rate
	_ALIGNED(8)	double	g_daudio_rate ;
	_ALIGNED(8)	double	g_drecip_audio_rate ;
	_ALIGNED(8)	double	g_dsamps_per_dcyc ;
	_ALIGNED(8)	double	g_dcycs_per_samp ;
	float	g_fsamps_per_dcyc ;

	int	g_doc_vol ;

	_ALIGNED(8)	double g_last_sound_play_dsamp ;

	float c030_fsamps[MAX_C030_TIMES + 1];
	int g_num_c030_fsamps ;

	int	g_sound_shm_pos ;

	int g_samp_buf[2*MAX_SND_BUF];
	word32 zero_buf[SOUND_SHM_SAMP_SIZE];

	_ALIGNED(8)	double g_doc_dsamps_extra ;

	word32	g_last_c030_vbl_count;
	int	g_c030_state ;

	// TO CHECK!
	int g_sound_play_depth ;


	// sound_driver.c

	int	g_bytes_written ;

	#define ZERO_BUF_SIZE		2048

	word32 g_snd_zero_buf[ZERO_BUF_SIZE];

	int	g_zeroes_buffered ;
	int	g_zeroes_seen ;
	int	g_sound_paused ;
	int	g_childsnd_vbl ;
	int	g_childsnd_pos ;


	s_sound()
	{

		INIT_SERIALIZED();

		g_doc_num_osc_en = 1;
		g_dcycs_per_doc_update = 1.0;
		g_dupd_per_dcyc = 1.0;
		g_drecip_osc_en_plus_2 = 1.0 / (double)(1 + 2);

		g_audio_enable = -1;

		doc_reg_e0 = 0xff;

		g_audio_rate = 44100;		// OG Preferred Audio Rate
		g_doc_vol = 2;
	}

	DEFAULT_SERIALIZE_IN_OUT ;

} XCODE_PRAGMA_PACK;


#ifdef _WIN32
#pragma pack(pop)
#endif


extern s_sound g_sound;
extern void x_async_sound_init();
extern void x_async_snd_shutdown();
extern void x_preload_sounds();

// WAV SUPPORT


struct OASound
{
    
    int m_BufferId;
    int m_SourceId;
    int frequency;
    int depth;
    int nbchannel;
    int     successfullyLoaded;
};


typedef struct
{
    const char* filename;
    //    unsigned int         ref;
    OASound     sound;
} system_sound;

extern system_sound g_system_sounds[];

typedef enum
{
    SOUND_EJECTION=0,
    SOUND_SPIN_UP_SEARCH_1,
    SOUND_SPIN_UP_SEARCH_2,
    SOUND_SKIP_SEARCH_1,
    SOUND_SKIP_SEARCH_2,
    SOUND_SKIP_SEARCH_3,
    SOUND_SKIP_SEARCH_4,
    SOUND_SKIP_SEARCH_5,
    SOUND_NB
} enum_sound;

void play_sound(enum_sound hSound);



bool x_load_wav(const char* _Path, unsigned char** _outDataBuffer, unsigned int & _outFileSize, OASound & _sound);
OASound async_init_wav(const char* _file);
int async_release_wav(OASound* _pSound);
int async_stop_wav(OASound* _pSound);
int async_play_wav(OASound* _pSound, int _Loop, float _Attenuation, float _StartAt  );
