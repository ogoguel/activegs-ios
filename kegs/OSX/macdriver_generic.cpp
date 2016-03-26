/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#if !defined(DRIVER_OSX) && !defined(DRIVER_IOS)
#error
#endif

#if  defined(DRIVER_IOS)
#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGBitmapContext.h>
#include <CoreFoundation/CFURL.h>

#else
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFURL.h>
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>

/*
#if !defined(ACTIVEGSPLUGIN) && !defined(ACTIVEGSCOCOA)
#define ENABLEQD
#endif
 */
#endif

#include <AudioToolbox/AudioToolbox.h>

#include "../src/defc.h"
#include "../src/protos_macdriver.h"
#include "../src/video.h"
#include "../src/raster.h"
#include "../src/sound.h"
#include "../src/StdString.h"



int	macUsingCoreGraphics=0;

word32	g_mac_shift_control_state = 0;

#ifdef ENABLEQD
WindowRef	g_main_window;
CGrafPtr mac_window_port;
#endif





//extern Kimage g_mainwin_kimage;


extern int Verbose;
/*
extern int g_video_act_width;
extern int g_video_act_height;
extern int g_video_act_margin_left;
extern int g_video_act_margin_right;
extern int g_video_act_margin_top;
extern int g_video_act_margin_bottom;
*/
// extern int g_screen_depth;
// extern int g_force_depth;


//extern int g_screen_mdepth;


extern int g_send_sound_to_file;

//extern int g_quit_sim_now; // OG Not need anymore
extern int g_config_control_panel;


int	g_auto_repeat_on = -1;
int	g_x_shift_control_state = 0;


extern int Max_color_size;

extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

int	g_alt_left_up = 1;
int	g_alt_right_up = 1;

//extern word32 g_full_refresh_needed;

//extern int g_border_sides_refresh_needed;
//extern int g_border_special_refresh_needed;
//extern int g_status_refresh_needed;

extern int g_lores_colors[];

extern int g_a2vid_palette;

//extern int g_installed_full_superhires_colormap;

//extern int g_screen_redraw_skip_amt;

//extern word32 g_a2_screen_buffer_changed;

int g_upd_count = 0;




void
update_window(void)
{
	
	// OG Not needed
	/*
	SetPortWindowPort(g_main_window);
	PenNormal();
	*/
	
	g_video.g_full_refresh_needed = -1;
	g_video.g_a2_screen_buffer_changed = -1;
	g_video.g_status_refresh_needed = 1;
	g_video.g_border_sides_refresh_needed = 1;
	g_video.g_border_special_refresh_needed = 1;

	g_upd_count++;
	if(g_upd_count > 250) {
		g_upd_count = 0;
	}

}


void x_update_modifiers(word32 state)
{
//#ifndef ACTIVEIPHONE

	word32	state_xor;
	int	is_up;

	state = state & (
					 cmdKey | controlKey |
					 shiftKey | alphaLock | optionKey 
				);
	state_xor = g_mac_shift_control_state ^ state;
//	if (state_xor) printf("modified:%X\n",state);
	is_up = 0;
	if(state_xor & controlKey) {
		is_up = ((state & controlKey) == 0);
		adb_physical_key_update(0x36, is_up);
	}
	if(state_xor & alphaLock) {
		is_up = ((state & alphaLock) == 0);
		adb_physical_key_update(0x39, is_up);
	}
	if(state_xor & shiftKey) {
		is_up = ((state & shiftKey) == 0);
		adb_physical_key_update(0x38, is_up);
	}
	if(state_xor & cmdKey) {
		is_up = ((state & cmdKey) == 0);
		adb_physical_key_update(0x37, is_up);
	}
	if(state_xor & optionKey) {
		is_up = ((state & optionKey) == 0);
		adb_physical_key_update(0x3a, is_up);
	}
//#endif

	g_mac_shift_control_state = state;
}


void
x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
}


void
x_update_physical_colormap()
{
}

void x_show_color_array()
{
	int i;

	for(i = 0; i < 256; i++) {
		printf("%02x: %08x\n", i, g_palette_8to1624[i]);
	}
}



void
x_get_kimage(Kimage *kimage_ptr)
{
#ifdef ENABLEQD
	PixMapHandle	pixmap_handle;
	GWorldPtr	world;
	Rect		world_rect;
	OSStatus	err;
#endif
//	word32	*wptr;
	byte	*ptr;
//	int	row_bytes;
	int	width;
	int	height;
	int	depth, mdepth;
	int	size;

	
	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;
	mdepth = kimage_ptr->mdepth;

	size = 0;
	if(depth == s_video.g_screen_depth) 
		{

			if (!macUsingCoreGraphics)
			
			{
#ifdef ENABLEQD
		SetRect(&world_rect, 0, 0, width, height);
		err = NewGWorld( &world, 0, &world_rect, NULL, NULL, 0);
		pixmap_handle = GetGWorldPixMap(world);
		err = LockPixels(pixmap_handle);
		ptr = (byte *)GetPixBaseAddr(pixmap_handle);
		int row_bytes = ((*pixmap_handle)->rowBytes & 0x3fff);
		kimage_ptr->width_act = row_bytes / (mdepth >> 3);
		mac_printf("Got depth: %d, bitmap_ptr: %p, width: %d\n", depth,	ptr, kimage_ptr->width_act);
		mac_printf("pixmap->base: %08x, rowbytes: %08x, pixType:%08x\n",(int)(*pixmap_handle)->baseAddr,(*pixmap_handle)->rowBytes,(*pixmap_handle)->pixelType);
		word32* wptr = (word32 *)(*pixmap_handle);
		mac_printf("wptr: %p=%08x %08x %08x %08x %08x %08x %08x %08x\n",wptr,wptr[0], wptr[1], wptr[2], wptr[3],wptr[4], wptr[5], wptr[6], wptr[7]);
		kimage_ptr->dev_handle = pixmap_handle;
		kimage_ptr->data_ptr = ptr;
#endif
		}
			else
			{
			
				
				kimage_ptr->width_act = width ;
			size = height* kimage_ptr->width_act * mdepth >> 3;
			ptr = (byte *)malloc(size);
			
			if(ptr == 0) {
				mac_printf("malloc for data fail, mdepth:%d\n", mdepth);
				exit(2);
			}
			
			kimage_ptr->data_ptr = ptr;
				kimage_ptr->dev_handle = (void *)-1;
			}
		}
	else {

		/* allocate buffers for video.c to draw into */

		
		kimage_ptr->width_act = width ;
		size = height* kimage_ptr->width_act * mdepth >> 3 ;		
		ptr = (byte *)malloc(size);

		if(ptr == 0) {
			mac_printf("malloc for data fail, mdepth:%d\n", mdepth);
			exit(2);
		}

		kimage_ptr->data_ptr = ptr;
		kimage_ptr->dev_handle = (void *)-1;
	}

	mac_printf("kim: %p, dev:%p data: %p, size: %08x\n", kimage_ptr,
		kimage_ptr->dev_handle, kimage_ptr->data_ptr, size);

}


#ifdef ENABLEQD
PixMapHandle	pixmap_backbuffer=NULL;
GWorldPtr	backbuffer=NULL;
#endif

// OG MAx OSX 10.5 and iOS 3.1 do not support passing null while creating bitmapcontext = must provide its own array
#if ( MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_5 ) || ( __IPHONE_OS_VERSION_MAX_REQUIRED < 4000 )
#define CGBITMAPCONTEXT_REQUIRES_DATA
#endif



void x_video_init()
{
	int	lores_col;
	int	i;

	printf("Preparing graphics system\n");
	
	
#ifndef ENABLEQD
	s_video.g_screen_depth = s_video.g_screen_mdepth = 32;
#else
	// Get screen depth
	GDHandle g_gdhandle = GetGDevice();
	s_video.g_screen_mdepth = (**((**g_gdhandle).gdPMap)).pixelSize;
	
	s_video.g_screen_depth = s_video.g_screen_mdepth;
	
#endif
	
	
	if(s_video.g_screen_depth > 16) {
		/* 32-bit display */
		s_video.g_red_mask = 0xff;
		s_video.g_green_mask = 0xff;
		s_video.g_blue_mask = 0xff;
		
		if (macUsingCoreGraphics)
		{
			s_video.g_red_left_shift = 0;
			s_video.g_green_left_shift = 8;
			s_video.g_blue_left_shift = 16;
		}
		else
		{
			s_video.g_red_left_shift = 16;
			s_video.g_green_left_shift = 8;
			s_video.g_blue_left_shift = 0;			
			
		}
		
		s_video.g_red_right_shift = 0;
		s_video.g_green_right_shift = 0;
		s_video.g_blue_right_shift = 0;
	} else if(s_video.g_screen_depth > 8) {
		/* 16-bit display */
		s_video.g_red_mask = 0x1f;
		s_video.g_green_mask = 0x1f;
		s_video.g_blue_mask = 0x1f;
		s_video.g_red_left_shift = 10;
		s_video.g_green_left_shift = 5;
		s_video.g_blue_left_shift = 0;
		s_video.g_red_right_shift = 3;
		s_video.g_green_right_shift = 3;
		s_video.g_blue_right_shift = 3;
	}
	

	// OG Create  backbuffer
#ifdef USE_RASTER
	x_init_raster(NULL);
#endif
	
	init_kimage(&g_kimage_offscreen,0,s_video.g_screen_depth, s_video.g_screen_mdepth);
	g_kimage_offscreen.width_act = X_A2_WINDOW_WIDTH;
	g_kimage_offscreen.width_req = X_A2_WINDOW_WIDTH;
	g_kimage_offscreen.height = X_A2_WINDOW_HEIGHT;
	
	
	if (!macUsingCoreGraphics)
	{

#ifdef ENABLEQD
	/*
	Rect r;
	SetRect(&r, 0, 0, 704, 462);
	NewGWorld( &backbuffer, 0, &r, NULL, NULL, 0);
	pixmap_backbuffer = GetGWorldPixMap(backbuffer);
	*/	
		x_get_kimage(&g_kimage_offscreen);
		
		
#endif
	}
	else
	{
	
	
		
		int pixelsWide = g_kimage_offscreen.width_act;
		int pixelsHigh = g_kimage_offscreen.height;
		int	bitmapBytesPerRow   = (pixelsWide * 4);// 1

		CGColorSpaceRef colorSpace =  CGColorSpaceCreateDeviceRGB();
		char* bitmapData = NULL;
#ifdef CGBITMAPCONTEXT_REQUIRES_DATA 
		int	bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
		void* bitmapDataAllocPtr = NULL;
		bitmapData = (char*)memalloc_align(bitmapByteCount, 256, &bitmapDataAllocPtr);
		if (bitmapData == NULL)
		{
			fprintf (stderr, "Memory not allocated!");
			return ;
		}
		g_kimage_offscreen.dev_handle2 = bitmapDataAllocPtr;
#else
		bitmapData=NULL;
#endif
		CGContextRef    offscreenContext = CGBitmapContextCreate (bitmapData,// 4
										 pixelsWide,
										 pixelsHigh,
										 8,      // bits per component
										 bitmapBytesPerRow,
										 colorSpace,
										 kCGImageAlphaNoneSkipLast);
		if (offscreenContext== NULL)
		{
			#ifdef CGBITMAPCONTEXT_REQUIRES_DATA 
			if (bitmapDataAllocPtr)
				free (bitmapDataAllocPtr);// 5
			bitmapData=NULL;
			bitmapDataAllocPtr=NULL;
		#endif
			printf("context not created");
			return ;
		}
		
	
#ifndef CGBITMAPCONTEXT_REQUIRES_DATA
		bitmapData = (char*)CGBitmapContextGetData(offscreenContext);
#endif		
		if (!bitmapData)
		{
			printf("video initialization failed...");
			return ;
		}
		
		
		g_kimage_offscreen.data_ptr = (byte*)bitmapData;
		g_kimage_offscreen.dev_handle = (void*)offscreenContext;
		
		
		CGContextSetRGBFillColor (offscreenContext, 0, 0, 0, 1);
		CGContextFillRect (offscreenContext, CGRectMake (0,0, 704, 462 ));
		
		CGColorSpaceRelease( colorSpace );// 6
	}

	
	video_get_kimages();

	if(s_video.g_screen_depth != 8) {
		// Get g_mainwin_kimage
		video_get_kimage(&s_video.g_mainwin_kimage, 0, s_video.g_screen_depth,
							s_video.g_screen_mdepth);
	}

	for(i = 0; i < 256; i++) {
		lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}

	s_video.g_installed_full_superhires_colormap = 1;

	fflush(stdout);

}


void x_video_shut()
{
	
	
	printf("xdriver_end\n");
	
	
	if (macUsingCoreGraphics)
	{
		#ifdef CGBITMAPCONTEXT_REQUIRES_DATA 
		 if (g_kimage_offscreen.dev_handle2)
		 {
			 free(g_kimage_offscreen.dev_handle2);
			 g_kimage_offscreen.dev_handle2 = NULL;
	//	 bitmapDataAllocPtr=NULL;
	//	 bitmapData=NULL;
		 }
		
		#endif
		g_kimage_offscreen.data_ptr = NULL;
	 if (g_kimage_offscreen.dev_handle)
		{
			CGContextRelease((CGContextRef)g_kimage_offscreen.dev_handle);
			g_kimage_offscreen.dev_handle = NULL;
	//		bitmapData = NULL;
		}
	}
	else
	{
#ifdef ENABLEQD
		pixmap_backbuffer = NULL;
		DisposeGWorld(backbuffer);
		backbuffer = NULL;
#endif
	}
	
#ifndef ACTIVEGS
	extern int g_fatal_log;
if(g_fatal_log >= 0) 
{
	x_show_alert(1, 0);
}
#endif
}




//extern volatile int x_in_async_refresh;

void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
		int width, int height)
{
	

	// OG TO INVESTIGATE
	if (!kimage_ptr->data_ptr)
	{
		printf("x_push_kimage: k_image_ptr not yet initilialized\n");
		return ;
	}
	

	
	if (!g_kimage_offscreen.data_ptr)
	{
		printf("err:graphic not initialized yet");
		return;
	}
	int wd = kimage_ptr->width_act * kimage_ptr->mdepth>>3;
	int w = width *4;
		
	//	int	bitmapBytesPerRow   = (pixelsWide * 4);// 1
		int bitmapBytesPerRow = g_kimage_offscreen.width_act * (g_kimage_offscreen.mdepth >> 3);
		
	 byte* ptrdest = g_kimage_offscreen.data_ptr + bitmapBytesPerRow*desty + destx*4;
	byte* srcdest = kimage_ptr->data_ptr + wd*srcy + srcx*4;
	for(int i=0;i<height;i+=1)
	{
		memcpy(ptrdest,srcdest,w);
		ptrdest += bitmapBytesPerRow;
		srcdest += wd;

	}

	g_video_offscreen_has_been_modified = 1;

}


void
x_auto_repeat_on(int must)
{
}

void
x_auto_repeat_off(int must)
{
}

void
x_hide_pointer(int do_hide)
{
#ifdef ENABLEQD
	if(do_hide) {
	//	printf("hidecursor\n");
		HideCursor();
	} else {
		ShowCursor();
	}
#endif
}


void
update_main_window_size()
{
	/*
#ifdef ENABLEQD
	Rect	win_rect;
	int	width, height;
	int	left, excess_height;
	int	top, bottom;
	
	GetPortBounds(GetWindowPort(g_main_window), &win_rect);
	width = win_rect.right - win_rect.left;
	height = win_rect.bottom - win_rect.top;
	g_video_act_width = width;
	g_video_act_height = height;
	
	left = MAX(0, (width - A2_WINDOW_WIDTH) / 2);
	left = MIN(left, BASE_MARGIN_LEFT);
	g_video_act_margin_left = left;
	g_video_act_margin_right = left;
	
	
	excess_height = (height - A2_WINDOW_HEIGHT) / 2;
	bottom = MAX(0, excess_height / 2);		// No less than 0
	bottom = MIN(BASE_MARGIN_BOTTOM, bottom);	// No more than 30
	g_video_act_margin_bottom = bottom;
	excess_height -= bottom;
	top = MAX(0, excess_height);
	top = MIN(BASE_MARGIN_TOP, top);
	g_video_act_margin_top = top;
#endif
	 */
}


// OG Adding release
void x_release_kimage(Kimage* kimage_ptr)
{
	if (kimage_ptr->dev_handle == (void*)-1)
	{
		free(kimage_ptr->data_ptr);
		kimage_ptr->data_ptr = NULL;
	}
	else
	{
		if (!macUsingCoreGraphics)
		{
#ifdef ENABLEQD
		UnlockPixels((PixMapHandle)kimage_ptr->dev_handle);
		kimage_ptr->dev_handle = NULL;
		DisposeGWorld((GWorldPtr)kimage_ptr->dev_handle2);
		kimage_ptr->dev_handle2 = NULL;
#endif
		}
	}
}

// OG Addding ratio
int x_calc_ratio(float&x,float&y)
{
	return 1;
}


void x_refresh_buffer()
{
#ifdef ENABLEQD
Rect	src_rect, dest_rect;
CGrafPtr window_port;

PixMapHandle pixmap_handle = (PixMapHandle)g_kimage_offscreen.dev_handle;
SetRect(&src_rect, 0, 0, g_kimage_offscreen.width_act, g_kimage_offscreen.height);
dest_rect = src_rect;

	SetPortWindowPort(g_main_window);
	window_port = GetWindowPort(g_main_window);
	
	CopyBits( (BitMap *)(*pixmap_handle),
			 GetPortBitMapForCopyBits(window_port), &src_rect, &dest_rect,
			 srcCopy, NULL);
#endif
}

void x_recenter_hw_mouse()
{

}

#if 0
bool x_load_wav(const char* _Path, unsigned char** _outDataBuffer, unsigned int & _outFileSize,  OASound & _sound  )
{
    
    CFStringRef	rawCFString    = CFStringCreateWithCString( NULL, _Path, CFStringGetSystemEncoding() );
    CFURLRef      urlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, rawCFString, kCFURLPOSIXPathStyle, FALSE );
    
    
    AudioFileID aSoundID = 0;
    bool bSuccess=FALSE;
    
    // Open file
    OSStatus error = AudioFileOpenURL(/*(CFURLRef)aFileURL*/urlRef, kAudioFileReadPermission, 0, &aSoundID);
    if (error == noErr)
    { // success
        
        //get Format
        AudioFileTypeID format = 0;
        UInt32 sizeFormat = sizeof(AudioFileTypeID);
        OSStatus result = AudioFileGetProperty(aSoundID, kAudioFilePropertyFileFormat,  &sizeFormat, (void*)&format);
        if(result != noErr)
        {
            printf("cannot find file format");
            goto cleanup;
        }
        
        if (format !=kAudioFileWAVEType)
        {
            printf("sound format not supported");
            goto cleanup;
            
        }
        
        //get DataFormat
        AudioStreamBasicDescription dataFormat ;
        UInt32 sizeDataFormat = sizeof(AudioStreamBasicDescription);
        result = AudioFileGetProperty(aSoundID, kAudioFilePropertyDataFormat,  &sizeDataFormat, (void*)&dataFormat);
        if(result != noErr)
        {
            printf("cannot find file dataformat");
            goto cleanup;
        }
        
        _sound.frequency = (int)dataFormat.mSampleRate;
        _sound.depth  = dataFormat.mBitsPerChannel;
        _sound.nbchannel = dataFormat.mChannelsPerFrame;
        
        //get File size
        UInt64 outDataSize = 0;
        UInt32 thePropSize = sizeof(UInt64);
        result = AudioFileGetProperty(aSoundID, kAudioFilePropertyAudioDataByteCount,
                                      &thePropSize, &outDataSize);
        if(result != noErr)
        {
            printf("cannot find file size");
            goto cleanup;
        }
        
        _outFileSize = (UInt32)outDataSize;
        
        
        
        // Create buffer
        *_outDataBuffer = (UInt8*)malloc(_outFileSize);
        
        //copy file into buffer
        OSStatus l_readResult = noErr;
        l_readResult = AudioFileReadBytes(aSoundID, false, 0, (UInt32*)&_outFileSize, *_outDataBuffer);
        
        if (l_readResult == noErr)
            bSuccess = TRUE; //Ouf!
        
    }
    
cleanup:
    
    if (!bSuccess)
        printf(" --- Error %ld loading sound at path: %s",error,_Path);
    else
    {
    //    printf(" --- Sound loaded : %s",_Path);
    }
    
    if (aSoundID)
        AudioFileClose(aSoundID);
    
    if (urlRef)
        CFRelease(urlRef);
      CFRelease(rawCFString);
    
    return bSuccess;
}
#endif


void x_preload_sounds()
{
	
    for(int i=0;i<SOUND_NB;i++)
    {
        MyString path;
        
#if defined(DRIVER_IOS)
        extern MyString resourcesPath;
        path.Format("%s/%s",resourcesPath,g_system_sounds[i].filename);
#else
        extern void mac_get_resource_path(const char *_path,MyString& _resPath);
        mac_get_resource_path(g_system_sounds[i].filename,path);
#endif
        g_system_sounds[i].sound = async_init_wav(path.c_str());
        
    }
}


