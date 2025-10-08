/**************************************************************
* Class::  CSC-415-02 Spring 2025
* Name:: 
* Student IDs::  
* GitHub-Name:: AkimT13
* Group-Name:: Coney Island
* Project:: Basic File System
*
* File:: workingDirFunctions.c
*
* Description:: Implementing fixPath(), fs_setcwd(),  fs_getcwd(),
*				fs_isDir() and fs_mkdir() functions as well as 
*               fs_diriteminfo struct.
*
**************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mfs.h"
#include "initRoot.h"
#include "parsePath.c"
#include "globals.h"
#include "freeSpace.h"

#define DIR_BLOCKS 8

void fixPath(const char *input, char *output, size_t size)
{
    char *parts[256];
    int partsCount = -1;

    char temp[1024];
    strncpy(temp, input, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';

    char *saveptr;
    char *token = strtok_r(temp, "/", &saveptr);

    // Go through every part separated by slashes
    while (token != NULL)
    {
        if (strcmp(token, ".") == 0)
        {
            // do nothing for "."
        }
        else if (strcmp(token, "..") == 0)
        {
            if (partsCount >= 0)
            {
                partsCount--; // remove last folder
            }
        }
        else
        {
            partsCount++;
            parts[partsCount] = token; // add folder
        }
        token = strtok_r(NULL, "/", &saveptr);
    }

    // Build the final path
    if (partsCount == -1)
    {
        strncpy(output, "/", size);
        output[size - 1] = '\0';
        return;
    }

    output[0] = '\0'; // clear output
    for (int i = 0; i <= partsCount; i++)
    {
        strncat(output, "/", size - strlen(output) - 1);
        strncat(output, parts[i], size - strlen(output) - 1);
    }
}

// function to set current working directory
int fs_setcwd(char *pathname)
{

    // call parsePath to populate ppInfo structure
    ppInfo parsePathInfo;
    int res = parsePath(pathname, &parsePathInfo);

    if (res < 0)
    {
        return -1;
    }
    // -2 is the sentinal value of root
    if (parsePathInfo.index == -2)
    {
        currentDirectory = root;
        currentDirectoryStartBlock = 6;
    }
    else
    {
        File *target = &parsePathInfo.parent[parsePathInfo.index];
        if (!target->isDirectory)
            return -1;

        File *newDir = loadDirectory(target->location);
        if (!newDir)
            return -1;

        if (currentDirectory != root)
        {
            free(currentDirectory);
        }

        currentDirectory = newDir;
        currentDirectoryStartBlock = target->location;
    }

    char tempPath[1024];

    if (pathname[0] == '/')
    {
        // Absolute path, just copy
        strncpy(tempPath, pathname, sizeof(tempPath));
        tempPath[sizeof(tempPath) - 1] = '\0'; // null terminate
    }
    else
    {
        // Relative path, manually append
        strncpy(tempPath, currentPath, sizeof(tempPath));
        tempPath[sizeof(tempPath) - 1] = '\0'; // null terminate

        // Add a slash if needed
        if (tempPath[strlen(tempPath) - 1] != '/')
        {
            strncat(tempPath, "/", sizeof(tempPath) - strlen(tempPath) - 1);
        }

        strncat(tempPath, pathname, sizeof(tempPath) - strlen(tempPath) - 1);
    }
    fixPath(tempPath, currentPath, sizeof(currentPath));
    return 0;
}

// function to get current working directory 
char *fs_getcwd(char *pathname, size_t size)
{
    strncpy(pathname, currentPath, size);
    pathname[size - 1] = '\0';
    return pathname;
}

int fs_isDir(char *pathname)
{
    ppInfo info;
    int result = parsePath(pathname, &info);

    if (result < 0)
    {
        return -1; // could not parse path
    }
    if (info.index == -2)
    {
        return 1; // root is directory
    }
    if (info.index == -1)
    {
        return 0; // path doesnt exist
    }
    File *target = &info.parent[info.index];
    return target->isDirectory ? 1 : 0;
}

// function to make a new directory
int fs_mkdir(const char *pathname, mode_t mode)
{
    // get parent dir and name of new dir
    ppInfo info;
    int res = parsePath((char *)pathname, &info);

    // already exists or couldnt parse
    if (res != -2)
    {
        // If it's not -2, then the directory already exists or failed to parse
        return -1;
    }

    // Extract the parent path and last element
    char pathCopy[1024];
    strncpy(pathCopy, pathname, sizeof(pathCopy));
    pathCopy[sizeof(pathCopy) - 1] = '\0';

    char *lastSlash = strrchr(pathCopy, '/');
    if (lastSlash)
    {
        *lastSlash = '\0';
        if (strlen(pathCopy) == 0)
            strcpy(pathCopy, "/");
    }
    else
    {
        strcpy(pathCopy, ".");
    }

    ppInfo parentInfo;
    if (parsePath(pathCopy, &parentInfo) < 0)
    {
        return -1;
    }
    info.parent = parentInfo.parent;
    info.lastElementName = strdup(lastSlash ? lastSlash + 1 : pathname);
    if (!info.lastElementName)
        return -1;

    // allocate a block for new dir
    int newDirBlock = allocBlocks(DIR_BLOCKS);
    if (newDirBlock < 0)
    {
        return -1; // failed to allocate
    }

    // init the new dir block (50 empty file entries)

    File *newDir = calloc(50, sizeof(File));
    if (!newDir)
    {
        return -1; // mem allocation failed
    }

    time_t now = time(NULL);

    newDir[0].free = 1;
    newDir[0].isDirectory = 1;
    newDir[0].location = newDirBlock;
    strncpy(newDir[0].name, ".", sizeof(newDir[0].name) - 1);
    newDir[0].name[sizeof(newDir[0].name) - 1] = '\0';
    newDir[0].date_created = now;
    newDir[0].date_modified = now;
    newDir[0].size = 0;

    newDir[1].free = 1;
    newDir[1].isDirectory = 1;
    newDir[1].location = currentDirectoryStartBlock;
    strncpy(newDir[1].name, "..", sizeof(newDir[1].name) - 1);
    newDir[1].name[sizeof(newDir[1].name) - 1] = '\0';
    newDir[1].date_created = now;
    newDir[1].date_modified = now;
    newDir[1].size = 0;

    // write the empty dir block to disk
    LBAwrite(newDir, DIR_BLOCKS, newDirBlock);

    // find empty spot in parent and drop the new entry in
    
    
    File *parent = info.parent;
    for (int j = 0; j < 50; j++)
    {
        if (parent[j].free == 0)
        {
            
            parent[j].free = 1;
            parent[j].isDirectory = 1;
            parent[j].location = newDirBlock;
            strncpy(parent[j].name, info.lastElementName, sizeof(parent[j].name) - 1);
            parent[j].name[sizeof(parent[j].name) - 1] = '\0';
            parent[j].date_created = time(NULL);
            parent[j].date_modified = time(NULL);
            parent[j].size = 0;
            
            break;
        }
    }
    
    for (int k = 0; k < 50; k++)
    {
        if (currentDirectory[k].free)
        {
            
        }
    }
    // write updated parent back to disk
    LBAwrite(currentDirectory, DIR_BLOCKS, currentDirectoryStartBlock);
    
           

    vcb->freeBlocks -= DIR_BLOCKS;
    persistVCB();
    free(info.lastElementName);
    free(newDir);
    return 0;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    //check if the directory pointer or return struct is invalid
    // if either is NULL, we cant continue reading entries
    if (dirp == NULL || dirp->di == NULL)
        return NULL;

    // loop through the directory starting from where the last read left off
    // this allows repeated calls to continue scanning the directory incrementally
    while (dirp->dirEntryPosition < 50)
    {
        int pos = dirp->dirEntryPosition;
        File *entry = &dirp->directory[pos];

        //advance the positoin pointer regardless 
        //of whether it returns or skips this entry
        //so it doesnt get stuck rechecking the same index

        dirp->dirEntryPosition++;

        // only consider entires that are not free
        // simulates how filesystems skip over deleted or empty slots

        if (entry->free == 1)
        {
        // copy the file name into the return struct,
        // making sure its NULL terimnated
        //this name will be used by the caller to identify the directory entry
            strncpy(dirp->di->d_name, entry->name, sizeof(dirp->di->d_name) - 1);
            dirp->di->d_name[sizeof(dirp->di->d_name) - 1] = '\0';

            //set the file type to distinguish between files and directories
            dirp->di->fileType = entry->isDirectory ? FT_DIRECTORY : FT_REGFILE;
            
            dirp->di->d_reclen = sizeof(struct fs_diriteminfo);
            // return the filled struct pointer so the caller can access
            // the entry info 
            return dirp->di;
        }
    }
    // nothing left to read
    return NULL;
}

// function to make a file
int makeFile(char *pathname)
{
    //printf("[makeFile] Attempting to create: %s\n", pathname);


    // populate ppInfo struct
    ppInfo info;
    int res = parsePath(pathname, &info);

    if (res != -2)
    {
        printf("[makeFile] File already exists or invalid path. res = %d\n", res);
        return -1;
    }

    // Extract the parent path and final file name
    char pathCopy[1024];
    strncpy(pathCopy, pathname, sizeof(pathCopy));
    pathCopy[sizeof(pathCopy) - 1] = '\0';

    char *lastSlash = strrchr(pathCopy, '/');
    if (lastSlash)
    {
        *lastSlash = '\0';
        if (strlen(pathCopy) == 0)
            strcpy(pathCopy, "/");
    }
    else
    {
        strcpy(pathCopy, ".");
    }

    // Parse the parent path
    ppInfo parentInfo;
    if (parsePath(pathCopy, &parentInfo) < 0)
    {
        printf("[makeFile] Failed to parse parent path\n");
        return -1;
    }

    info.parent = parentInfo.parent;
    info.parentStartBlock = parentInfo.parentStartBlock;

    // Extract final file name
    if (lastSlash)
    {
        info.lastElementName = strdup(lastSlash + 1); // Use part after the last '/'
    }
    else
    {
        info.lastElementName = strdup(pathname); // No / found; use the whole path
    }

    if (!info.lastElementName)
        return -1;

    File *parent = info.parent;

    // Allocate space in the directory array
    for (int i = 0; i < 50; i++)
    {
        if (parent[i].free == 0)
        {
            time_t now = time(NULL);

            // Allocate a block for file data
            int block = allocBlocks(1);
            if (block < 0)
            {
                printf("[makeFile] Failed to allocate block for file\n");
                free(info.lastElementName);
                return -1;
            }

            parent[i].free = 1;
            parent[i].isDirectory = 0;
            parent[i].location = block;
            parent[i].size = 0;
            parent[i].date_created = now;
            parent[i].date_modified = now;

            strncpy(parent[i].name, info.lastElementName, sizeof(parent[i].name) - 1);
            parent[i].name[sizeof(parent[i].name) - 1] = '\0';

            //printf("[makeFile] File '%s' created at dir index %d, block %d\n", parent[i].name, i, block);

            // Write parent directory back to disk
            LBAwrite(parent, DIR_BLOCKS, info.parentStartBlock);
            free(info.lastElementName);
            return 0;
        }
    }

    printf("[makeFile] No space left in parent directory\n");
    free(info.lastElementName);
    return -1;
}
