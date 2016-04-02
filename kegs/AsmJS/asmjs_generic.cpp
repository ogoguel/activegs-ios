/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../src/defc.h"
#include "../src/video.h"
#ifdef __EMSCRIPTEN__
#include <sdl.h>
#else
#include <sdl/sdl.h>

#endif

extern   int  g_lores_colors[];
extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

void x_hide_pointer(int do_hide)
{

}

void x_full_screen(int do_full)
{

}

void x_show_color_array()
{
}


void x_recenter_hw_mouse()
{
}

void x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
}



SDL_Surface* screen ;
//SDL_Surface* offscreen ;

void x_video_init()
{
int	lores_col;

	printf("x_video_init\n");

 	SDL_Init(SDL_INIT_VIDEO);

	screen = SDL_SetVideoMode(762, 504, 32, SDL_HWSURFACE);
	printf("BitsPerPixel: %d \n",screen->format->BitsPerPixel);
	printf("Rmask: %X \n",screen->format->Rmask);
	printf("Gmask: %X \n",screen->format->Gmask);
	printf("Bmask: %X \n",screen->format->Bmask);
	printf("Amask: %X \n",screen->format->Amask);

/*
	offscreen = SDL_CreateRGBSurface(SDL_HWSURFACE, screen->w, screen->h,
        screen->format->BitsPerPixel,
        screen->format->Rmask,
        screen->format->Gmask,
        screen->format->Bmask,
        screen->format->Amask
    );
*/


	s_video.g_screen_depth = 32;
	s_video.g_screen_mdepth = 32;

	s_video.g_red_mask = 0xff;
	s_video.g_green_mask = 0xff;
	s_video.g_blue_mask = 0xff;
	s_video.g_red_left_shift = 0;
	s_video.g_green_left_shift = 8;
	s_video.g_blue_left_shift = 16;
	s_video.g_red_right_shift = 0;
	s_video.g_green_right_shift = 0;
	s_video.g_blue_right_shift = 0;

	video_get_kimages();

	if(s_video.g_screen_depth != 8) {
		//	Allocate g_mainwin_kimage
		video_get_kimage(&s_video.g_mainwin_kimage, 0, s_video.g_screen_depth, s_video.g_screen_mdepth);
	}

	for(int i = 0; i < 256; i++) {
		lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}

	init_kimage(&g_kimage_offscreen,0,s_video.g_screen_depth, s_video.g_screen_mdepth);

	// init offscreen
	//memset(&g_kimage_offscreen,0,sizeof(g_kimage_offscreen));
	g_kimage_offscreen.width_req = X_A2_WINDOW_WIDTH;
	g_kimage_offscreen.width_act = X_A2_WINDOW_WIDTH;
	g_kimage_offscreen.height = X_A2_WINDOW_HEIGHT;
	//g_kimage_offscreen.depth = g_screen_depth;
	//g_kimage_offscreen.mdepth = g_screen_mdepth;
	x_get_kimage(&g_kimage_offscreen);

	s_video.g_installed_full_superhires_colormap = 1;
}

void x_get_kimage(Kimage *kimage_ptr)
{
	byte	*ptr;
	int	width;
	int	height;
	int	depth, mdepth;
	int	size;

	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;
	mdepth = kimage_ptr->mdepth;

	printf("x_get_kimage %dx%d\n",width,height);
/*
	 SDL_Surface* layer = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height,
        screen->format->BitsPerPixel,
        screen->format->Rmask,
        screen->format->Gmask,
        screen->format->Bmask,
        screen->format->Amask
    );

	 kimage_ptr->data_ptr =  NULL; //layer->pixels;
	kimage_ptr->dev_handle = (void *)layer;
*/

	size = 0;
	if(depth == s_video.g_screen_depth)
	{

		kimage_ptr->width_act = width ;
		size = height* kimage_ptr->width_act * mdepth >> 3;
		ptr = (byte *)malloc(size);

		if(ptr == 0)
		{
			printf("malloc for data fail, mdepth:%d\n", mdepth);
			exit(2);
		}
		kimage_ptr->data_ptr = ptr;
		kimage_ptr->dev_handle = (void *)-1;
	}
	else
	{
		
		kimage_ptr->width_act = width ;
		size = height* kimage_ptr->width_act * mdepth >> 3 ;
		ptr = (byte *)malloc(size);

		if(ptr == 0)
		{
			printf("malloc for data fail, mdepth:%d\n", mdepth);
			exit(2);
		}

		kimage_ptr->data_ptr = ptr;
		kimage_ptr->dev_handle = (void *)-1;
	}
	

	printf("kim: %p, dev:%p data: %p, size: %08x\n", kimage_ptr,kimage_ptr->dev_handle, kimage_ptr->data_ptr, size);

}

void x_video_shut()
{
}

void x_auto_repeat_on(int must)
{
}

void x_auto_repeat_off(int must)
{
}

void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height)
{

	SDL_LockSurface(screen);

	int pixsize = kimage_ptr->mdepth>>3 ;
	int srcrow = kimage_ptr->width_act * pixsize;
	int dstrow = screen->pitch;;
	int byte2copy = width * pixsize ;


//	printf("x_push_kimage %d x %d (%d)\n",width,height,kimage_ptr->mdepth);
	


	char* ptrdest = (char*)screen->pixels +dstrow*desty + destx*pixsize;
//	printf("writing to  %X\n",ptrdest);
	char* srcdest = (char*)kimage_ptr->data_ptr + srcrow*srcy + srcx*pixsize;
	for(int i=0;i<height;i+=1)
	{
		memcpy(ptrdest,srcdest,byte2copy);
		ptrdest += dstrow;;
		srcdest += srcrow;

	}

	SDL_UnlockSurface(screen);
//	SDL_Flip(screen);

	 g_video_offscreen_has_been_modified = 1;
//	 SDL_BlitSurface(offscreen,NULL,screen,NULL);

	 
}

void asmjs_video_update()
{
	if(g_video_offscreen_has_been_modified)
	{
		g_video_offscreen_has_been_modified = 0;
		SDL_Flip(screen);
	}
}

void x_release_kimage(Kimage *kimage_ptr)
{
    
}

void x_refresh_video()
{
}
	

void x_update_physical_colormap()
{
}

void x_update_modifiers(word32 _mod)
{
}




