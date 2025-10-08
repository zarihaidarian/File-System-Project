/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Zari Haidarian
* Student IDs:: 917423031
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: initRoot.h
*
* Description:: Header file for structs and functions necessary 
*               to initialize root directory 
*
**************************************************************/

#ifndef _INITROOT_H
#define _INITROOT_H

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "fsLow.h"


typedef struct File {
    char name[30]; // file name
    char permissions[30]; // file permissions
    int isDirectory; // is it a directory or file entry
    int location; // file location
	int free; // is it in a free state 
    time_t date_created; // date file is created
    time_t date_modified; // date file is modified 
    size_t size; // size of file
}File;

extern File* rootDirectory;
int initRootDirectory(uint64_t blockSize);
void freeRootDirectory();
int loadRootDirectory(int startBlock);
File * loadDirectory(int location);
int findInDirectory(char *name, File *directory);
#endif 
