/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov, Devarsh Hirpara, Zari Haidarian
* Student IDs:: 922761746, 922991898, 917423031
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: b_io.c
*
* Description:: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "parsePath.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
    /** TODO add al the information you need in the file control block **/
    char *buf;  // holds the open file buffer
    int index;  // holds the current position in the buffer
    int buflen; // holds how many valid bytes are in the buffer
    int inUse;
    char *name;
    int location;
    int fileSize;     
    int filePos;
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
    // init fcbArray to all free
    for (int i = 0; i < MAXFCBS; i++)
    {
        fcbArray[i].buf = NULL; // indicates a free fcbArray
        fcbArray[i].inUse = 0;
        fcbArray[i].name = NULL;
    }

    startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
    for (int i = 0; i < MAXFCBS; i++)
    {
        if (fcbArray[i].inUse == 0)
        {
            fcbArray[i].inUse = 1;
            return i;
        }
    }
    return (-1); // all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
    if (startup == 0)
        b_init(); // init system if needed

    // Check if file exists
    ppInfo info;
    int res = parsePath(filename, &info);

    if (res < 0 && (flags & O_CREAT))
    {
        // File doesn't exist, but creation is allowed
        if (makeFile(filename) < 0)
        {
            return -1; // failed to create
        }
        // Try parsing again after creation
        res = parsePath(filename, &info);
        if (res < 0 || info.index < 0)
            return -1;
    }
    else if (res < 0)
    {
        return -1; // file doesn't exist and no O_CREAT
    }

    // File exists here; grab FCB
    int fd = b_getFCB();
    if (fd < 0)
        return -1;

    File *file = &info.parent[info.index];
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;
    fcbArray[fd].buf = malloc(B_CHUNK_SIZE); 
    fcbArray[fd].location = info.parent[info.index].location;
    fcbArray[fd].fileSize = file->size;      
    fcbArray[fd].name = strdup(filename);
    fcbArray[fd].filePos = 0;
    return fd; // all set
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
    if (startup == 0)
        b_init(); // Initialize our system

    // check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS))
    {
        return (-1); // invalid file descriptor
    }

    return (0); // Change this
}


// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
     if (startup == 0)
        b_init(); // Initialize our system

    if ((fd < 0) || (fd >= MAXFCBS) || (fcbArray[fd].buf == NULL))
        return -1;

    
    // counter of total bytes successfully writtemn 
    int bytesWritten = 0;

    // Loop until all bytes are written
    while (bytesWritten < count)
    {
        // Calculate how much space is left in the internal buffer
        int spaceLeft = B_CHUNK_SIZE - fcbArray[fd].index;
        // determine how much we still need to write
        int remainingToWrite = count - bytesWritten;
        // we only need to write however much we can fit
        int toWrite = (remainingToWrite < spaceLeft) ? remainingToWrite : spaceLeft;

        if (toWrite > 0)
        {   
            // Copy data from user buffer to internal buffer
            memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + bytesWritten, toWrite);
            // update values for fcb
            fcbArray[fd].index += toWrite;
            if (fcbArray[fd].index > fcbArray[fd].buflen)
                fcbArray[fd].buflen = fcbArray[fd].index;


            // advance in file and track how much we wrote
            bytesWritten += toWrite;

            
            fcbArray[fd].filePos += toWrite;

            //update the file size accordingly
            if (fcbArray[fd].filePos > fcbArray[fd].fileSize){
                fcbArray[fd].fileSize = fcbArray[fd].filePos;
            }
                
        }
        // If the internal buffer is full, flush it to disk
        if (fcbArray[fd].index == B_CHUNK_SIZE)
        {
            int blocksWritten = LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].location);
            if (blocksWritten != 1)
            {
                printf("[b_write] Error: LBAwrite failed at block %d\n", fcbArray[fd].location);
                return -1;
            }
            // Reset internal buffer and move to next block
            fcbArray[fd].index = 0;
            fcbArray[fd].buflen = 0;

            
            fcbArray[fd].location++;
        }
    }

    return bytesWritten;

    
}







// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+



int b_read(b_io_fd fd, char *buffer, int count){
    if (startup == 0) {
        b_init();  // Initialize file system on first use
    }

    // Validate file descriptor
    if (fd < 0 || fd >= MAXFCBS || fcbArray[fd].inUse == 0 || fcbArray[fd].buf == NULL) {
        return -1;
    }

    // Cap read size to avoid reading beyond file end
    if (fcbArray[fd].filePos + count > fcbArray[fd].fileSize) {
        count = fcbArray[fd].fileSize - fcbArray[fd].filePos;
    }

    int totalBytesRead = 0;  // Total bytes copied to user buffer

    //part 1
    // First, read from the current internal buffer if there are bytes left
    if (fcbArray[fd].index < fcbArray[fd].buflen) {
        int availableInBuffer = fcbArray[fd].buflen - fcbArray[fd].index;
        int bytesToCopy = (count < availableInBuffer) ? count : availableInBuffer;

        memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, bytesToCopy);
        fcbArray[fd].index += bytesToCopy;
        totalBytesRead += bytesToCopy;
        fcbArray[fd].filePos += bytesToCopy;
         // If we've read enough already, return now
        if (totalBytesRead == count) {
            return totalBytesRead;
        }
    }

    //part 2
    // Read full blocks directly from disk to user buffer
    int remaining = count - totalBytesRead;
    int fullBlocks = remaining / B_CHUNK_SIZE;

    if (fullBlocks > 0) {
        int blocksRead = LBAread(buffer + totalBytesRead, fullBlocks, fcbArray[fd].location);
        if (blocksRead != fullBlocks) {
            return -1; // Disk read error
        }

        fcbArray[fd].location += fullBlocks;
        int bytesRead = fullBlocks * B_CHUNK_SIZE;
        totalBytesRead += bytesRead;
        fcbArray[fd].filePos += bytesRead;
    }

    //part 3
    // Read any remaining bytes (less than one block) into internal buffer
    remaining = count - totalBytesRead;
    if (remaining > 0) {
        int readResult = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].location);
        if (readResult != 1) {
            return totalBytesRead > 0 ? totalBytesRead : -1;
        }

        fcbArray[fd].location++;  // Advance to next block
        fcbArray[fd].index = 0;
        fcbArray[fd].buflen = B_CHUNK_SIZE;

        int finalCopy = (remaining < fcbArray[fd].buflen) ? remaining : fcbArray[fd].buflen;
        memcpy(buffer + totalBytesRead, fcbArray[fd].buf, finalCopy);

        fcbArray[fd].index = finalCopy;
        totalBytesRead += finalCopy;
        fcbArray[fd].filePos += finalCopy;
    }

    return totalBytesRead;
}





// Interface to Close the file
int b_close(b_io_fd fd)
{
    if (fd < 0 || fd >= MAXFCBS || fcbArray[fd].buf == NULL) {
        return -1;  // invalid file descriptor
    }

    // Flush remaining buffer
    if (fcbArray[fd].index > 0) {
        LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].location);
        fcbArray[fd].index = 0;
        fcbArray[fd].buflen = 0;
    }

    
    ppInfo info;
    if (parsePath(fcbArray[fd].name, &info) == 0 && info.index >= 0) {
        info.parent[info.index].size = fcbArray[fd].fileSize;

        // Write the parent directory block back
        int written = LBAwrite(info.parent, 1, info.parentStartBlock);
        if (written != 1) {
            printf("[b_close] Warning: failed to update file size on disk\n");
        }
    }

    // Free and cleanup
    fcbArray[fd].inUse = 0;
    free(fcbArray[fd].buf);
    fcbArray[fd].buf = NULL;
    return 0;
}
