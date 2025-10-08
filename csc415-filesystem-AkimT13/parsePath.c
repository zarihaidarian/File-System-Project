/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: Akim Tarasov 
* Student IDs:: 9227161746  
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: parsePath.c
*
* Description:: Breaks down the pathname and locates the target within the filesystem.
* It also populates a ppInfo struct containting important data shared accorss al fs functions
*
**************************************************************/
#include "parsePath.h"
#include "initRoot.h"
#include "globals.h"

int parsePath(char *pathname, ppInfo *ppinfo) {

   
    File *parent;
    File *startParent;
    int parentBlock;

    
    // Determine the starting directory based on whether the path is absolute or relative
    if (pathname[0] == '/') {
        parent = root;
        parentBlock = vcb->rootLocation;
    } else {
        parent = currentDirectory;
        parentBlock = currentDirectoryStartBlock;
    }

    startParent = parent;

    // copy the pathname so we can modify it
    char *pathCopy = strdup(pathname);
    if (!pathCopy)
        return -1;


    // tokenize path by /
    char *saveptr;
    char *token1 = strtok_r(pathCopy, "/", &saveptr);

    if (token1 == NULL) {
        // If it is the root or if it is empty
        if (pathname[0] == '/') {
            ppinfo->parent = parent;
            ppinfo->index = -2;
            ppinfo->parentStartBlock = vcb->rootLocation;
            free(pathCopy);
            return 0;
        } else {
            free(pathCopy);
            return -1;
        }
    }


    // continue tokenizing the path
    while (1) {
        // locate token1 in the directory
        int idx = findInDirectory(token1, parent);
        

        
        if (idx == -1) {
            // Not found — return -2 for mkdir/rmdir usage
            ppinfo->parent = parent;
            ppinfo->index = -1;
            ppinfo->parentStartBlock = parentBlock;
            ppinfo->lastElementName = strdup(token1);
            free(pathCopy);
            return -2;
        }

        // check if more elements exist
        char *token2 = strtok_r(NULL, "/", &saveptr);

        if (token2 == NULL) {
            // Final component reached
            ppinfo->parent = parent;
            ppinfo->index = idx;
            ppinfo->parentStartBlock = parentBlock;
            ppinfo->lastElementName = strdup(token1);
            free(pathCopy);
            return 0;
        }

        // Descend into next directory
        if (!parent[idx].isDirectory) {
            free(pathCopy);
            return -1;
        }

        File *nextParent = loadDirectory(parent[idx].location);
        if (parent != startParent) {
            free(parent);
        }

        parent = nextParent;
        parentBlock = parent[idx].location;
        token1 = token2;
    }


    // shouldnt reach here since the while condition is always true
    free(pathCopy);
    return -1;
    
}