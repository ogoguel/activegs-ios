/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../Libraries/libpng/png.h"
#include "CEMulatorCtrl.h"
#include "../kegs/Src/video.h"
#define abort_(X) { printf(X); return 0;}

extern word32 g_a2palette_8to1624[256];


#ifdef DRIVER_OSX
extern int macUsingCoreGraphics;
#endif

int savePNG(MyString& _path)
{

	int _borderWidth = option.getIntValue(OPTION_PNGBORDER);
	
	_path = "({ \"error\" : \"Cannot create screenshot\" })";

	CEmulator* pEmu = CEmulator::theEmulator ;
	int bootslot = pEmu->config->bootslot;

	MyString filename;
	CSlotInfo& slot = pEmu->getLocalIMGInfo(bootslot,1);
	
	printf(slot.shortname.c_str());
	MyString filenoext;
	if (!slot.shortname.IsEmpty())
		filenoext = getfilenoext(slot.shortname.c_str());
	else
		filenoext = "nodisk";
		
	int i=0;
//	CDownload dl;
	while(i<100)
	{
		filename.Format("%s%sscreenshot_%s_%d.png",
			CDownload::getPersistentPath(),
				ACTIVEGS_DIRECTORY_SEPARATOR,	
				filenoext.c_str(),
				i);
		
		FILE* f=fopen(filename.c_str(),"rb");
		if (!f) break;
		fclose(f);
		i++;
	}
	Kimage* kimage_ptr = g_video.g_a2_line_kimage[0];
	int a2width ;
	int a2height;

	if (kimage_ptr == &s_video.g_kimage_superhires)
	{
		a2width = 320*VIDEO_HFACTOR;
		a2height=200*VIDEO_VFACTOR;;
	}
	else
	{
		kimage_ptr = &s_video.g_mainwin_kimage;
		a2width = 280*VIDEO_HFACTOR;
		a2height = 192*VIDEO_VFACTOR;
	}

	int width = a2width + _borderWidth*2;
	int height = a2height + _borderWidth*2;


	png_byte color_type =PNG_COLOR_TYPE_RGB;
	png_byte bit_depth = 8;

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep row_pointers[1024];
	byte *pngraw = (byte*)malloc(width*height*3);
	int stride  =kimage_ptr->width_act * kimage_ptr->mdepth / 8;

	int bcolor = g_a2palette_8to1624[g_video.g_border_color];
	char border[3];
#ifdef DRIVER_OSX
	if (macUsingCoreGraphics)
	{
		border[0]=(char)bcolor;
		border[1]=(char)(bcolor >> 8);
		border[2]=(char)(bcolor >> 16);
	}
	else
#endif
	{
		border[2]=(char)bcolor;
		border[1]=(char)(bcolor >> 8);
		border[0]=(char)(bcolor >> 16);

}

	for(int i=0;i<height;i++)
	{
		byte* row = &pngraw[i*width*3];
		row_pointers[i]=row;
		for(int j=0;j<width;j++)
		{
			byte* dst= &row[j*3];
			
			if ( i<_borderWidth || i>=(a2height+_borderWidth)
				|| j<_borderWidth || j>=(a2width+_borderWidth) )
			{
				dst[0]=border[0];
				dst[1]=border[1];
				dst[2]=border[2];
			}
			else
			{
				byte* src= &kimage_ptr->data_ptr[(i-_borderWidth)*stride+(j-_borderWidth)* kimage_ptr->mdepth / 8];
			#ifdef DRIVER_OSX
				if (macUsingCoreGraphics)
				{
					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2];
				}
				else
			#endif
				{
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
				}
			}
		}
	}	

	/* create file */
	FILE *fp = fopen(filename.c_str(), "wb");
	if (!fp)
		abort_("[write_png_file] File could not be opened for writing");


	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
		     bit_depth, color_type, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	png_destroy_info_struct(png_ptr,&info_ptr);
	
	png_destroy_write_struct(&png_ptr,&info_ptr);
	png_free(png_ptr,NULL);
	free(pngraw);
    fclose(fp);

	outputInfo("screenshot %s saved\n",filename.c_str());
	_path.Format("({ \"info\" : \"Screenshot saved : %s\", \"filename\" : \"%s\" })",getfile(filename.c_str()),filename.c_str());

	return 1;
}
