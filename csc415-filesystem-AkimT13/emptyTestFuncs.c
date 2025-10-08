/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov, Devarsh Hirpara
* Student IDs:: 922761746, 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: emptyTestFuncs.c
*
* Description:: Implementing fs_closedir() and fs_opendir() functions 
*				from mfs.h file.
*
**************************************************************/
#include "mfs.h"
#include "parsePath.h"
#include "globals.h"

int fs_closedir(fdDir *dirp){
    if (dirp) {
        free(dirp->di);
        free(dirp->directory);
        free(dirp);
    }
    return 0;
}

fdDir* fs_opendir(const char *pathname){
    ppInfo info;
    // use parsePath to get information about the directory
    int res = parsePath((char *)pathname, &info);
    if (res < 0) return NULL;
    File *target;
    if (info.index == -2) {
        // root directory

        target = &root[0];
    } else {
        target = &info.parent[info.index];
    }

    // Check if the target is actually a directory
    if (!target->isDirectory) return NULL;

    // Load the contents of the directory into memory
    File *loaded = loadDirectory(target->location);
    if (!loaded) return NULL;
    // Allocate memory for the fdDir structure which represents the open directory
    fdDir *dir = malloc(sizeof(fdDir));
    if (!dir) {
        free(loaded);
        return NULL;
    }

    
    dir->dirEntryPosition = 0;
    dir->directory = loaded;
    dir->di = malloc(sizeof(struct fs_diriteminfo));

    return dir;
}




