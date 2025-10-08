/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov, Devarsh Hirpara
* Student IDs:: 922761746, 922991898
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: globals.c
*
* Description:: Global varibles accessed and modifed by various functions.
*
**************************************************************/
#include "initRoot.h"
#include "globals.h"

File * currentDirectory = NULL;
int currentDirectoryStartBlock = 6;
File* root = NULL;
char currentPath[256] = "/";


