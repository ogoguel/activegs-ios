//
//  bigfile.cpp
//  ActiveGS_Karateka
//
//  Created by Olivier Goguel on 14/12/2017.
//


#include "bigfile.h"


#if KARATEKA_BIGFILE
#define ONLY_BIG
#endif

#define BIG 0x8000

struct CBigFile
{
    const char* filename;
    off_t seek_pos;
    const char* buffer;
    int size;
    bool opened;
    int code;
};


#if KARATEKA_BIGFILE
extern "C" const char karateka_bin[];
extern "C" const char karateka_activegsxml[];


CBigFile files[] ={
    "karateka.bin", 0, karateka_bin,143360,false,0xEA,
    "karateka.activegsxml", 0, karateka_activegsxml,5260,false,0XEA,
    
    NULL,0,NULL,0,false
};
#else
CBigFile files[] ={
    NULL,0,NULL,0,false
};

#endif


/*
off_t seek_pos = -1;
const char* buffer = NULL;
int   file_size = -1;
*/

static const char* getfile(const char* p)
{
    int l = strlen(p);
    while(l>=0)
    {
        if (p[l]=='/')
            return p+l+1;
        l--;
    }
    return p;
}


int big_open(const char *_pathname, int _flags, mode_t _mode)
{
   const char* file = getfile(_pathname);
    if (file[0]=='*')
    {
#if KARATEKA_BIGFILE
        int l = 0;
        while(files[l].filename != NULL)
        {
            if (!strcmp(file+1,files[l].filename))
            {
                if (files[l].opened)
                {
                    printf("already opened %s",_pathname);
                //    return -1;
                }
                files[l].opened = true;
                files[l].seek_pos = 0;
                return BIG+l;
            }
            l++;
        }
        return -1;
#else
     return open(_pathname+1,_flags,_mode);   
#endif
    }
    else
    {
#ifdef ONLY_BIG
        printf("missing file %s\n",_pathname);
        return -1;
#else
        return open(_pathname,_flags,_mode);
#endif
    }
}



off_t big_lseek(int _fd, off_t _offset, int _whence)
{
    if ( ( _fd & BIG ) == 0 )
    {
#ifdef ONLY_BIG
        return 0;
#else
        return lseek(_fd, _offset, _whence);
#endif
    }
    else
    {
         CBigFile& file = files[_fd &~BIG];
        if (_whence == SEEK_SET)
            file.seek_pos = _offset;
        else
        if (_whence == SEEK_END)
            file.seek_pos = file.size-1;
        
        return file.seek_pos;
    }
}

ssize_t big_read(int _fd, void *_buf, size_t _count)
{
     if ( ( _fd & BIG ) == 0 )
     {
#ifdef ONLY_BIG
         return 0;
#else
         return read(_fd, _buf, _count);
#endif
     }
    else
     {
          CBigFile& file = files[_fd &~BIG];
         
         if (_count + file.seek_pos > file.size)
             _count = file.size - file.seek_pos;
         
         memcpy(_buf,file.buffer+file.seek_pos,_count);
         char *decode = (char*)_buf;
         for(int i=0;i<_count;i++)
             decode[i] ^= file.code;
         file.seek_pos += _count;
         return _count;
     }
}

ssize_t big_write(int _fd, void* _buf, size_t _count)
{
    if ( ( _fd & BIG ) == 0 )
    {
        return _count;
    }
    else {
        return write(_fd, _buf, _count);
    }
}
    

int big_close(int _fd)
{
     if ( ( _fd & BIG ) == 0 )
     {
         CBigFile& file = files[_fd &~BIG];
         if (!file.opened)
         {
             printf("nothing to close");
             return -1;
         }
         file.opened=false;
     }
    else
        close(_fd);
    return 0;
}

