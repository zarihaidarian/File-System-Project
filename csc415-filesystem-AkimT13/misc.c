/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov
* Student IDs:: 922761746  
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: misc.c
*
* Description:: Implementing fs_isFile() function.
*
**************************************************************/
#include "parsePath.h"
#include "globals.h"

int fs_isFile(char *filename) {
    ppInfo parsePathInfo;
    int res = parsePath(filename, &parsePathInfo);
    if (res < 0 || parsePathInfo.index == -1){
        return 0;
    }
    File *target = &parsePathInfo.parent[parsePathInfo.index];
    return target->isDirectory == 0;
}