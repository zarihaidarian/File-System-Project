/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov, Devarsh Hirpara
* Student IDs:: 922761746, 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: globals.h
*
* Description:: Header file containing several global variables.
*
**************************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H

#include "initRoot.h"
#include "vcbStruct.h"


typedef struct VolumeControlBlock VolumeControlBlock;
extern struct VolumeControlBlock *vcb;
extern File* currentDirectory;
extern int currentDirectoryStartBlock;
extern File* root;
extern char currentPath[256];
void persistVCB();


#endif
