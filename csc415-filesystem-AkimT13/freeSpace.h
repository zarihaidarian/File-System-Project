/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name::  Devarsh Hirpara
* Student IDs:: 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: freeSpace.h
*
* Description:: Header file containing several functions to implement.
*
**************************************************************/
#ifndef FREESPACE_H
#define FREESPACE_H

#include <stdio.h>
#include <stdint.h> 
#include "mfs.h"

int initFreeSpaceBitMap(uint64_t blocks, uint64_t blockSize);

int allocBlock();

int releaseBlock(int blockNum);


int allocBlocks(int count);


void releaseBlocks(int start, int count);

void freeFSM();

int loadFSM(int FSMLocation);




#endif