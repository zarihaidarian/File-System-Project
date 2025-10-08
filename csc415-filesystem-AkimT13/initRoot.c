/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Zari Haidarian
* Student IDs:: 917423031
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: initRoot.c
*
* Description:: The C file that initializes the root directory.
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

#define DIR_BLOCKS 8


int initRootDirectory(uint64_t blockSize){
	// allocate space for the directory entry  
	File *myfile = malloc(sizeof(File)*50);
	if(myfile==NULL){
		return -1;
	}
	// mark all the entries as free 
	for(int i=0; i < 50; i++){
		myfile[i].free = 0;
	}

	// request 6 blocks from the freespace system 
	int start = allocBlocks(DIR_BLOCKS); // finish this 
	if(start == -1){
		return -1; 
	}

	// set the first directory entry in root directory 
	myfile[0].free = 1;
	strcpy(myfile[0].name, ".");
	myfile[0].size = sizeof(File) * 50;
	myfile[0].location = start;
	time_t current = time(NULL);
	myfile[0].date_created = current;
	myfile[0].date_modified = current;
	myfile[0].isDirectory = 1;
	// set the first directory entry in root directory 
	myfile[1].free = 1;
	strcpy(myfile[1].name, "..");
	myfile[1].location = start;
	myfile[1].date_created = current;
	myfile[1].date_modified = current;
	myfile[1].isDirectory = 1;
	
	
	
	
	
	root = myfile;
	LBAwrite(myfile, DIR_BLOCKS, start);
    printf("\n\nrunning\n\n");
	return start;

}
// loads the root directory into memory
int loadRootDirectory(int startBlock) {
	// allocates memory for the root
	File* buffer = malloc(sizeof(File) * 50);
	if (!buffer) return -1;
	// reads from disk
	if (LBAread(buffer, DIR_BLOCKS, startBlock) !=DIR_BLOCKS) {
		free(buffer);
		return -1;
	}

	// set global variables
	root = buffer;
	// current starts as root
	currentDirectory = buffer;
	
	currentDirectoryStartBlock = startBlock;
	return 0;
}

int findInDirectory(char * name, File * directory){
	// cycle through the directory, checking if the name matches, skips empty slots
	for (int i = 0; i < 50; i++) {
        if (directory[i].free == 0) continue;
		
        if (strcmp(directory[i].name, name) == 0) {
			
            return i;
        }
    }
	
	return -1;
}

File * loadDirectory(int location){
	// reads a File from disk starting at location into memory
	File* dir = malloc(sizeof(File) * 50);
    if (!dir) return NULL;

    if (LBAread(dir, DIR_BLOCKS, location) != DIR_BLOCKS) {
        free(dir);
        return NULL;
    }

    return dir;
}

void freeRootDirectory(){
	free(root);
}
