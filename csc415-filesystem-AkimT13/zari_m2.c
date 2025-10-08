/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Zari Haidarian, Devarsh Hirpara
* Student IDs:: 917423031, 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: zari_m2.c
*
* Description:: Implementing fs_stat(), fs_delete(), and
*               fs_rmdir() functions 
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "initRoot.h"
#include "freeSpace.h"
#include "parsePath.h"



// fs_stat

int fs_stat(const char *path, struct fs_stat *buf) {
    // check if path and buf are null then exit if they are
    if (path == NULL || buf == NULL) {
        return -1; 
    } 
    // check if path name is valid then exit if not
    ppInfo info;
    int res = parsePath((char *)path, &info);
    if (res < 0 || info.index < 0) return -1;
    
    File *curr = &info.parent[info.index];

    // fill in the struct
    buf->st_size = curr->size;  // file/directory size
    buf->st_createtime = curr->date_created;  // creation time
    buf->st_accesstime = curr->date_modified;  // last accessed time
    buf->st_modtime = curr->date_modified;  // last modified time
    /*
    buf->st_isDirectory = currentDirectory->isDirectory;  // check if it is a directory
    buf->st_location = currentDirectory->location;  // location of the file or directory
    */
    
    return 0;
}


// fs_delete

int fs_delete(char *filename) {
     // check if the filename given is a file and if not exit
     if (!fs_isFile(filename)) {
        printf("Invalid file name.\n");
        return -1;
    }
    // check if parsePath is valid 
    ppInfo info;
    int res = parsePath(filename, &info);
    if (res < 0 || info.index < 0) {
        printf("Invalid file path.\n");
        return -1;
    }
    // go through file system 

    File *target = &info.parent[info.index];
    target->free = 0;
    target->size = 0;
    target->date_modified = time(NULL);
    target->location = -1;
    LBAwrite(info.parent, 1, currentDirectoryStartBlock);
    
    
    return 0;

}

// fs_rmdir

int fs_rmdir(const char *pathname) {
    // check if the pathname given is a directory and if not exit
    if (!fs_isDir(( char *)pathname)) {
        printf("Invalid directory name.\n");
        return -1;
    }
    ppInfo info;
    int res = parsePath((char *)pathname, &info);
    if (res < 0 || info.index < 0) {
        printf("Invalid directory path.\n");
        return -1;
    }
     // go through file system
    File *target = &info.parent[info.index];
    target->free = 0;
    target->size = 0;
    target->date_modified = time(NULL);
    target->location = -1;

    LBAwrite(info.parent, 1, currentDirectoryStartBlock); 

    printf("Directory deleted");
     
    
    
    return 0;
}


