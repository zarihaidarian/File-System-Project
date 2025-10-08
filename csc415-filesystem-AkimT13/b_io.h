/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Devarsh Hirpara
* Student IDs:: 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: b_io.c=h
*
* Description:: Interface of basic I/O Operations
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

