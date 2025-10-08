/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name::  Gerious Heishan
* Student IDs:: 922559089
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island 
* Project:: Basic File System
*
* File:: b_seek.c
*
* Description:: Implements b_seek() function from b_io.h file.
*
**************************************************************/


#include <unistd.h>  
#include "b_io.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

static int startup = 0;
void b_init() {
    startup = 1;
}

// The b_seek function
int b_seek(b_io_fd fd, off_t offset, int whence) {
    // Initialize the system if not done
    if (startup == 0) {
        b_init();  
    }

    // Check file descriptor
    if (fd < 0 || fd >= MAXFCBS || fcbArray[fd].buf == NULL) {
        return -1;  // if Invalid file 
    }

    // Move the file position 
    off_t result = lseek(fcbArray[fd].fileDesc, offset, whence);
    if (result == (off_t)-1) {
        return -1;  // Error occurred during seek
    }

    // Reset 
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;

    return (int)result;  // Return the new position as int
}