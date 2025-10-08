/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov
* Student IDs:: 922761746  
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: vcbStruct.h
*
* Description:: C file containing VolumeControlBlock struct.
*
**************************************************************/
// vcbStruct.h
#ifndef VCB_STRUCT_H
#define VCB_STRUCT_H

#include <stdint.h>

typedef struct VolumeControlBlock {
    uint32_t numOfBlocks;
    uint32_t freeBlocks;
    uint32_t freeSpaceMapStart;
    uint32_t freeSpaceMapSize;
    uint32_t rootLocation;
    uint64_t signature;
} VolumeControlBlock;

#endif
