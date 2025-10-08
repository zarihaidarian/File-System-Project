/**************************************************************
* Class::  CSC-415-05 Spring 2025
* Name:: Akim Tarasov, Gerious Heishan, Devarsh Hirpara
* Student IDs:: 922761746, 922559089, 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: freeSpace.c
*
* Description::This code helps keep track of free space on a disk, so the file system knows which parts of the disk are available for saving files. 
* It ensures the system can store and remove files efficiently. 
* It uses a map where each bit represents a block on the disk—0 means the block is free, and 1 means it's used.
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <string.h>
#include "freeSpace.h"
#include "fsLow.h"


// so it can be used in other functions
unsigned char *freeSpaceMap = NULL;


int initFreeSpace(uint64_t blocks, uint64_t blockSize){ 
    freeSpaceMap = (unsigned char *)calloc(5, blockSize);
    if(freeSpaceMap == NULL){
        return -1;
    }

    unsigned char bitmask = 0b11111100;

    freeSpaceMap[0] |= bitmask;

    uint64_t writeRes = LBAwrite(freeSpaceMap, 5,1);
    if(writeRes == -1){
        printf("LBA write failed in freeSpace init");
        free(freeSpaceMap);
        return -1;
    }

    return 1;





}

int allocBlocks(int count){
    int totalBits = 512 * 5 * 8; 
    int currBlock = 0;  // Start at block 0
    int curr = 0;
    int startBlock = -1;

    while (currBlock < totalBits) {
        // 
        int byteIndex = currBlock / 8;
        int bitIndex = currBlock % 8;

        // Make a mask to check the bit
        unsigned char bitmask = (1 << bitIndex);
        unsigned char currByte = freeSpaceMap[byteIndex];

        // checks oif bit is 0
        if ((currByte & bitmask) == 0) {
            if (curr == 0) {
                startBlock = currBlock;
            }

            curr++;

            // found enough contiguous 
            if (curr == count) {
                // Mark all of them as used
                int k = 0;
                while (k < count) {
                    int bitIndex2 = (startBlock + k) / 8;
                    int bitPos = (startBlock + k) % 8;
                    freeSpaceMap[bitIndex2] |= (1 << bitPos);
                    k++;
                }

                // Save changes to disk
                uint64_t writeRes = LBAwrite(freeSpaceMap, 5,1);
                if(writeRes == -1){
                    printf("LBA write failed in alloc blocks");
                    free(freeSpaceMap);
                    return -1;
                }
                

                return startBlock;
            }
        } else {
            // Not free — reset
            curr = 0;
            startBlock = -1;
        }

        currBlock++;
    }

    // 
    printf("Not enough consecutive free blocks.\n");
    return -1;


}

int allocBlock() {
    int totalBits = 5 * 512 * 8; // Total number of bits in the freeSpaceMap (5 blocks × 512 bytes × 8 bits)

    for (int i = 0; i < totalBits; i++) {
        int byteIndex = i / 8;
        int bitPos = i % 8;

        if ((freeSpaceMap[byteIndex] & (1 << bitPos)) == 0) {
            freeSpaceMap[byteIndex] |= (1 << bitPos);

            
            uint64_t writeRes = LBAwrite(freeSpaceMap, 5, 1);
            if (writeRes == -1) {
                printf("LBA write failed in allocBlock\n");
                return -1;
            }
            return i; // Return the allocated block index
        }
    }
    // No free block found
    return -1;
}


void freeFSM(){
    free(freeSpaceMap);
}

int loadFSM(int FSMLocation){
    printf("loading fsm \n");

    // get space for fsm
    freeSpaceMap = (unsigned char *) malloc(5 * 512);
    if(freeSpaceMap==NULL){
        printf("[loadFSM] allocating memory for fsm failed");
        return -1;

    }
    // read into the buffer from disk
    printf("Loading free space map into memory");
    int res = LBAread(freeSpaceMap,5,1);
    if(res < 5){
        printf("Reading fsm from disk failed");
        return -1;

    }
    return 0;
    




}

void releaseBlocks(int start, int count) {

    // use bitwise AND to flip 1's of a paticular byte into 0's
    for (int i = 0; i < count; i++) {
        int block = start + i;
        int byteIndex = block / 8;
        int bitPos = block % 8;
        freeSpaceMap[byteIndex] &= ~(1 << bitPos);
    }
    uint64_t writeRes = LBAwrite(freeSpaceMap, 5, 1);
    if (writeRes == -1) {
        printf("LBA write failed in releaseBlocks\n");
    }
}
