/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Zari Haidarian, Akim Tarasov, Diego Antunez, Devarsh Hirpara
* Student IDs:: 917423031, 922761746, 922061514, 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
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
#include "globals.h"
#include "vcbStruct.h"

#define VCB_SIGNATURE 0x436F6E657949736C 



// pointer to hold the vcb in memory during system operation
struct VolumeControlBlock *vcb;

extern int initFreeSpace(uint64_t numberOfBlocks, uint64_t blockSize);
extern int initRootDirectory(uint64_t blockSize);

void persistVCB() {
    if (LBAwrite(vcb, 1, 0) != 1) {
        printf("Failed to write vcb to disk\n");
    } else {
        //printf("VCB written to disk\n");
    }
}


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	//allocate memory to hold 1 block for vcb
	
	
	vcb = (struct VolumeControlBlock *) malloc(blockSize);
	if (vcb == NULL) {
		printf("Failed to allocate memory for VCB\n");
		return -1;
	}
	// read exactly 1 block from the block address 0 into vcb
	if (LBAread(vcb, 1, 0) != 1) {
		printf("Failed to read VCB from disk.\n");
		free(vcb);
		return -1;
	}
	
	// if the signature doesnt match, assume volume is uninitialized and format it
	if (vcb->signature != VCB_SIGNATURE) {

	// For uint64_t
    //printf("Signature (uint64_t): %lu vs expected %lu\n", (unsigned long)vcb->signature, VCB_SIGNATURE);
	// For unsigned long
    //printf("Signature (unsigned long): %lu vs expected %lu\n", vcb->signature, VCB_SIGNATURE);


		
		printf("Volume not initialized. Formatting needed\n");
	// set vcb values	
	vcb->signature = VCB_SIGNATURE;
	vcb->numOfBlocks = numberOfBlocks;
	
	//initialize free space 
	int res = initFreeSpace(numberOfBlocks, blockSize);
	if (res == -1) {
		printf("Error initializng free space map\n");
		free(vcb);
		return -1;
	
	}
	// store where bitmap starts
	vcb->freeSpaceMapStart = res;
	// store bitmap size 
	vcb->freeSpaceMapSize = 5; 
	
	//initialize root directory
	int rootStart = initRootDirectory(blockSize);
	if (rootStart == -1) {
		fprintf(stderr, "Error initializing root directory\n");
		free(vcb);
		return -1;
	}
	// store where root directory starts
	vcb->rootLocation = rootStart; 
	loadRootDirectory(rootStart);
	// calculate and store remaining free blocks
	vcb->freeBlocks = numberOfBlocks - 1 - 5 - 6;
	// write initialized vcb struct to disk at block 0
	if (LBAwrite(vcb, 1, 0) != 1) {
		printf("Failed to write VCB to disk\n");
		free(vcb);
		return -1;
	}
	printf("Formatting complete. Volume initialized\n");
	} else {

	// if signature matches
	// For uint64_t
    //printf("Signature (uint64_t): %lu vs expected %lu\n", (unsigned long)vcb->signature, VCB_SIGNATURE);


	// For unsigned long
    //printf("Signature (unsigned long): %lu vs expected %lu\n", vcb->signature, VCB_SIGNATURE);
	
	
	printf("Volume already initialized\n");
	loadRootDirectory(vcb->rootLocation);
	int fsmRes = loadFSM(vcb->freeSpaceMapStart);
	if(fsmRes < 0){
		printf("[initFileSystem] something went wrong with loading the freespacemap,volume already formatted");
		return -1;
	}
	
	
	}
	return 0;
	}
	
	
	
	
	
void exitFileSystem ()
	{

	if (currentDirectory != NULL) {
        LBAwrite(currentDirectory, 6, currentDirectoryStartBlock);
    }
	persistVCB();
	freeRootDirectory();
	freeFSM();
	free(vcb);
	printf ("System exiting\n");
	}
