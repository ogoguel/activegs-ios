/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"

#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
#	include <AudioToolbox/AudioToolbox.h>
#	import <OpenAL/alc.h>
#	import <OpenAL/al.h>
#	include <unistd.h>
#else
#	include "../OpenAL/AL/alc.h"
#	include "../OpenAL/AL/al.h"
#endif

#include "sound.h"

//extern int g_audio_rate;

#define AL_NUM_BUFFERS 8
#define AL_BUFFER_SIZE  (SOUND_SHM_SAMP_SIZE*4*2)
ALuint source=0, buffers[AL_NUM_BUFFERS];
int sndQueues[AL_NUM_BUFFERS];

const ALenum format = AL_FORMAT_STEREO16;
unsigned char *al_tmp_buf[AL_NUM_BUFFERS][AL_BUFFER_SIZE];
ALCdevice *dev=NULL;
ALCcontext *ctx=NULL;

int nblog = 2000;
//#define SNDLOG(STR,VAL) if (nblog){outputInfo(STR,VAL); nblog--;}
//#define SNDLOG(STR,VAL) if (nblog){printf(STR,VAL); nblog--;}
#define SNDLOG(STR,VAL) 


// http://www.subfurther.com/blog/?p=602

#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
typedef ALvoid AL_APIENTRY (*alBufferDataStaticProcPtr) (const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq);


ALvoid  alBufferDataStaticProc(const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq)
{
   
	static alBufferDataStaticProcPtr proc = NULL;

    if (proc == NULL) {
        proc = (alBufferDataStaticProcPtr) alGetProcAddress((const ALCchar*) "alBufferDataStatic");
    }
    
    if (proc)
        proc(bid, format, data, size, freq);
	
    return;
}
#endif

void resetQueues()
{
	
	for(int i=0;i<AL_NUM_BUFFERS;i++)
	{
		sndQueues[i]=0;
			
	}
	SNDLOG("resetQueues %d\n",0);
}

int getQueueFromBuffer(int _bid)
{
	for(int i=0;i<AL_NUM_BUFFERS;i++)
	{
		if (buffers[i]==_bid)
			return i;
	}
	SNDLOG("id %d not found\n",_bid);

	return -1;
}

void freeQueue(int _id)
{
	if (!sndQueues[_id])
	{
		SNDLOG("buffer id %d was not occupied\n",_id);
	}
	sndQueues[_id]=0;
}

int getAvailableQueue()
{

	for(int i=0;i<AL_NUM_BUFFERS;i++)
	{
		if (!sndQueues[i])
		{
			sndQueues[i]=1;
			return i;
		}
	}
	
	SNDLOG("no available queue",0);
	return -1;
}

int async_snd_send_audio(byte *ptr, int in_size);

int x_snd_send_audio(byte *ptr, int in_size)
{

	return async_snd_send_audio(ptr,in_size);

}
	

void x_snd_child_init()
{

	printf("x_snd_child_init");

	if (!dev)
	{
		printf("err: async_sound_init not called\n");
		return ;
	}
	set_audio_rate(g_sound.g_audio_rate);
}

void x_snd_init(word32 *shmaddr)
{
	
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

// ---------------------------------------------------------------------

void x_async_sound_init()
{

	if (!g_sound.g_audio_enable)
	{
		printf("sound disabled : discarding openal driver\n");
		return ;
	}

	if (dev)
	{
		printf("err: async_sound_init already initialized!");
		return ;
	}
	dev = alcOpenDevice(NULL);
    if(!dev)
    {
        fprintf(stderr, "Oops\n");
        return ;
    }
	ctx = alcCreateContext(dev, NULL);
	
    alcMakeContextCurrent(ctx);
    if(!ctx)
    {
        fprintf(stderr, "Oops2\n");
        return ;
    }
	
	alGenBuffers(AL_NUM_BUFFERS, buffers);
	alGenSources(1, &source);
	if(alGetError() != AL_NO_ERROR)
	{
		fprintf(stderr, "Error generating :(\n");
		return ;
	}
	
	resetQueues();

    x_preload_sounds();  
}


void x_async_snd_shutdown()
{
	
	if (!g_sound.g_audio_enable)
		return ;
	
	printf("async_snd_shutdown\n");

	if (!dev)
	{
		printf("err: openal device not initialized\n");
		return ;
	}
	alDeleteSources(1, &source);
	source = 0;
	alDeleteBuffers(AL_NUM_BUFFERS, buffers);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	ctx=NULL;
	alcCloseDevice(dev);
	dev=NULL;

}

int async_snd_send_audio(byte *ptr, int in_size)
{
	
	
	int sizeref = in_size;
	int nbw = 0;
	// remplit les buffers avec la taille
	while(in_size)
	{
		
		ALint val;
		ALuint buffer = -1;
		int ibuffer=1001;
		
		int nb = MIN(AL_BUFFER_SIZE,in_size);

		// trying to get an unprocess buffer
		ALint processed=-1;
		ALint queued=-1;
		alGetSourcei (source, AL_BUFFERS_QUEUED, &queued);
		SNDLOG("queued  %d\n",queued);
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		SNDLOG("processed  %d\n",processed);
		
		// libère toute les queues
		while(processed--)
		{
			alSourceUnqueueBuffers(source, 1, &buffer);
			if(alGetError() != AL_NO_ERROR)
			{
				SNDLOG("#",0);
				return sizeref;
			}
			ibuffer = getQueueFromBuffer(buffer);
			freeQueue(ibuffer);
		}

		
		// réalloue un buffer
		ibuffer = getAvailableQueue();
		if (ibuffer<0)
		{
			SNDLOG("*",0);
			return sizeref;
		}
		buffer = buffers[ibuffer];
				
		SNDLOG("sendaudio: %d\n",in_size);
		
		// on a assez de donnees pour remplir tout le buffer
		
		memcpy(&al_tmp_buf[ibuffer][0],ptr+nbw,nb);
		
		nbw +=  nb;
		in_size -= nb;
	
		
		// on peut l'envoyer
#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
		alBufferDataStaticProc
#else
		alBufferData
#endif
		(buffer, format, &al_tmp_buf[ibuffer][0], nb, g_sound.g_audio_rate);
		
		if(alGetError() != AL_NO_ERROR)
        {
            printf("Error alBufferData :(%d)\n",ibuffer);
        }
		
		alSourceQueueBuffers(source, 1, &buffer);
		if(alGetError() != AL_NO_ERROR)
		{
			printf( "Error alSourceQueueBuffers :(\n");
		}
			
		
		alGetSourcei(source, AL_SOURCE_STATE, &val);
		if(alGetError() != AL_NO_ERROR)
		{
			printf( "Error alSourceQueueBuffers :(\n");
		}
		if(val != AL_PLAYING)
		{
			alSourcePlay(source);
			if(alGetError() != AL_NO_ERROR)
			{
				printf( "Error alSourcePlay :(\n");
			}
			SNDLOG( "playSound\n",0);
			
		}
	}
	return nbw;
}



OASound async_init_wav(const char* _file)
{
    
    OASound sound;
    memset(&sound,0,sizeof(OASound));
    
    ALvoid* l_DataBuffer = NULL;
    unsigned int l_FileSize = 0;
   
    // OpenAL: first 
    if(x_load_wav(_file, (unsigned char**)&l_DataBuffer , l_FileSize,sound))
        //    if((l_DataBuffer != NULL) && (l_FileSize > 0))
    {
        if(l_DataBuffer == NULL)
        {
            printf("*** DataBuffer NULL!!! *** \n");
        }
        // grab a buffer ID from openAL
        alGenBuffers(1, (ALuint*)&sound.m_BufferId);
        
        ALenum l_error = alGetError();
        
        if(l_error != AL_NO_ERROR)
        {
            printf("**** OpenAL ERROR buffer%i\n", l_error);
            return sound;
        }
        else
        {
            printf("**** OpenAL OK buffer %i\n", sound.m_BufferId);
        }
        
        ALuint f;
        if (sound.depth==16)
        {
            if (sound.nbchannel == 1)
                f = AL_FORMAT_MONO16;
            else
                f = AL_FORMAT_STEREO16;
        }
        else
        {
            if (sound.nbchannel == 1)
                f = AL_FORMAT_MONO8;
            else
                f = AL_FORMAT_STEREO8;
            
        }
            
        
        alBufferData(sound.m_BufferId, f, l_DataBuffer, l_FileSize, sound.frequency);
        
        l_error = alGetError();
         if(l_error != AL_NO_ERROR)
        {
            printf("**** OpenAL ERROR format\n");
            return sound;
        }
        
        // create Source
        alGenSources(1, (ALuint*)&sound.m_SourceId);
        l_error = alGetError();
          if(l_error != AL_NO_ERROR)
        {
            printf("**** OpenAL ERROR source\n");
            return sound;
        }
        
        alSourcei(sound.m_SourceId, AL_BUFFER, sound.m_BufferId);
        
        l_error = alGetError();
         if(l_error != AL_NO_ERROR)
        {
            printf("**** OpenAL ERROR source buffer\n");
            return sound;
        }
        
        // if loop:
        //alSourcei(m_SourceId, AL_LOOPING, AL_TRUE);
        
        alSourcef(sound.m_SourceId, AL_PITCH, 1.0f);
        //l_error = alGetError();
        
        //ASSERTC(l_error == AL_NO_ERROR,"**** OpenAL ERROR buffer\n");
        //if(l_error != AL_NO_ERROR)
        //{
        //    CONSOLEMSG("**** OpenAL ERROR properties\n");
        //    return FAILURE;
        //}
        
        alSourcef(sound.m_SourceId, AL_GAIN, 1.0f);     
        
        l_error = alGetError();
        
        if(l_error != AL_NO_ERROR)
        {
            printf("**** OpenAL ERROR properties\n");
            return sound;
        }
           }
    else
    {
        return sound;
    }
    
    if (l_DataBuffer)
    {
        free(l_DataBuffer);
        l_DataBuffer = NULL;
    }
    
    //printf("**** OpenAL Resource load SUCCESS\n");
    sound.successfullyLoaded = 1;
	return sound;
}

int async_release_wav(OASound* _pSound)
{
    if (!_pSound)
        return 0;
    
    
	if(_pSound->m_SourceId)
	{
	    
        alDeleteSources(1, (ALuint*)&_pSound->m_SourceId);
        _pSound->m_SourceId = 0;
        
	}
    if(_pSound->m_BufferId)
    {
        alDeleteBuffers(1, (ALuint*)&_pSound->m_BufferId);
        _pSound->m_BufferId = 0;
    }
    _pSound->successfullyLoaded=0;
    return 1;
}


// ****************************************************************************
// 
// ****************************************************************************
int async_play_wav(OASound* _pSound, int _Loop, float _Attenuation, float _StartAt  )
{
    if (!_pSound)
        return 0;
    
    if (!_pSound->successfullyLoaded)
        return 0;
    
    
	if(!_pSound->m_SourceId)
        return 0;
    
	    
        alSourcePlay(_pSound->m_SourceId);
        
        int l_error = alGetError();
        
        if(l_error != AL_NO_ERROR)
        {
            printf("**** OpenAL ERROR play %i\n", l_error);
        }
	
    return 1;
}

// ****************************************************************************
// 
// ****************************************************************************
int async_stop_wav(OASound* _pSound)
{
    if (!_pSound)
        return 0;
    
    if (!_pSound->successfullyLoaded)
        return 0;
    
	if(!_pSound->m_SourceId)
        return 0;
    
    
    alSourceStop(_pSound->m_SourceId);
	return 1;
}

void x_play_sound(enum_sound _hSound)
{

    async_play_wav(&g_system_sounds[_hSound].sound,0,0,0);
    
}

int x_is_sound_playing(enum_sound _hSound)
{
    OASound& sound = g_system_sounds[_hSound].sound;
    if (sound.m_SourceId==0)
        return 0;
    ALint val;
    alGetSourcei(sound.m_SourceId, AL_SOURCE_STATE, &val);
   return (val == AL_PLAYING);
}


#if 1

/*
 * Struct to hold fmt subchunk data for WAVE files.
 */
struct WAVE_Format {
    char subChunkID[4];
    long subChunkSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
};

/*
 * Struct to hold the data of the wave file
 */
struct WAVE_Data {
    char subChunkID[4]; //should contain the word data
    long subChunk2Size; //Stores the size of the data block
};

struct RIFF_Header {
    char chunkID[4];
    long chunkSize;//size not including chunkSize or chunkID
    char format[4];
};

bool loadWavFile(const char* filename, unsigned char** buffer,
                 unsigned int* size, int* frequency,
                 int* depth,int* nbchannel) {
    //Local Declarations
    FILE* soundFile = NULL;
    WAVE_Format wave_format;
    RIFF_Header riff_header;
    WAVE_Data wave_data;
    unsigned char* data;
    
    
        soundFile = fopen(filename, "rb");
        if (!soundFile)
        {
            printf("could not open %s\n",filename);
            return false;
        }
           
        // Read in the first chunk into the struct
        fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);
        
        //check for RIFF and WAVE tag in memeory
        if ((riff_header.chunkID[0] != 'R' ||
             riff_header.chunkID[1] != 'I' ||
             riff_header.chunkID[2] != 'F' ||
             riff_header.chunkID[3] != 'F') ||
            (riff_header.format[0] != 'W' ||
             riff_header.format[1] != 'A' ||
             riff_header.format[2] != 'V' ||
             riff_header.format[3] != 'E'))
        {
            fclose(soundFile);
            printf ("Invalid RIFF or WAVE Header in %s\n",filename);
            return false;
        }
    
        //Read in the 2nd chunk for the wave info
        fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);
        //check for fmt tag in memory
        if (wave_format.subChunkID[0] != 'f' ||
            wave_format.subChunkID[1] != 'm' ||
            wave_format.subChunkID[2] != 't' ||
            wave_format.subChunkID[3] != ' ')
        {
            fclose(soundFile);
            printf ("Invalid Wave Format");
            return false;
        }
           
        
        //check for extra parameters;
        if (wave_format.subChunkSize > 16)
            fseek(soundFile, sizeof(short), SEEK_CUR);
        
        //Read in the the last byte of data before the sound file
        fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);
        //check for data tag in memory
        if (wave_data.subChunkID[0] != 'd' ||
            wave_data.subChunkID[1] != 'a' ||
            wave_data.subChunkID[2] != 't' ||
            wave_data.subChunkID[3] != 'a')
        {
            
            fclose(soundFile);
            printf ( "Invalid data header");
            return false;
        }
                    
        //Allocate memory for data
        data = (unsigned char*)malloc(wave_data.subChunk2Size);
    
        // Read in the sound data into the soundData variable
        if (!fread(data, wave_data.subChunk2Size, 1, soundFile))
        {
            free(data);
            fclose(soundFile);
            printf ( "error loading WAVE data into struct!");
            return false;
        }
           
        //Now we set the variables that we passed in with the
        //data from the structs
        *size = wave_data.subChunk2Size;
        *frequency = wave_format.sampleRate;
        *nbchannel = wave_format.numChannels ;
        *depth = wave_format.bitsPerSample;
        *buffer = data;
        fclose(soundFile);
        return true;
  }

bool x_load_wav(const char* _Path, unsigned char** _outDataBuffer, unsigned int & _outFileSize,  OASound & _sound  )
{
   
   return  loadWavFile(_Path,_outDataBuffer,&_outFileSize,&_sound.frequency,&_sound.depth,&_sound.nbchannel);
   }

#endif
