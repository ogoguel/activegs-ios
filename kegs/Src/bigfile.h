//
//  bigfile.hpp
//  ActiveGS_Karateka
//
//  Created by Olivier Goguel on 14/12/2017.
//


#ifndef bigfile_hpp
#define bigfile_hpp

#include "defc.h"

#if _MSC_VER
#define mode_t int
#define ssize_t int
#endif

int big_open(const char *_pathname, int _flags, mode_t _mode);
off_t big_lseek(int _fd, off_t _offset, int _whence);
ssize_t big_read(int _fd, void *_buf, size_t _count);
ssize_t big_write(int _fd, void *_buf, size_t _count);
int big_close(int _fd);
ssize_t big_write(int _fd, const void* _buf, size_t _count);

#endif /* bigfile_hpp */

