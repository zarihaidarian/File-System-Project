/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: 
* Student IDs::  
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: parsePath.h
*
* Description:: Header file for parsePath() function as well as 
*				ppInfo struct.
*
**************************************************************/
#ifndef PARSEPATH_H
#define PARSEPATH_H
#include "initRoot.h"
#include "globals.h"


typedef struct ppInfo
{

    File *parent;
    int index;
    char *lastElementName;
    int parentStartBlock;

} ppInfo;

int parsePath(char * pathname, ppInfo *ppinfo);




#endif
