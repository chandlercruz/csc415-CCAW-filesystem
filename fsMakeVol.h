/**************************************************************
* Class:  CSC-415
* Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 917048657 916260714 918810235
* Project: Basic File System 
*
* File: fsMakeVol.h
*
* Description: header file that holds the set of routines used to create a volume
*              for Penta File System.
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include "fsLow.h"
#include "bitMap.h"
#include "mfs.h"

//Number of data blocks to allocate per on inode.
#define DATA_BLOCKS_PER_INODE	4
#define VCB_START_BLOCK		0
#ifndef _MAKEVOL
#define _MAKEVOL

//structure to hold all the data
typedef struct {
  char header[16];
  uint64_t volumeSize;
  uint64_t blockSize;
  uint64_t diskBlocks;
  uint64_t vcbBlock;
  uint64_t totalVCBBlocks;
  uint64_t inodeBlock;
  uint64_t inodes;
  uint64_t inodeBlocks;
  uint64_t freeMapSize;
  uint32_t freeMap[];
} fs_VCB;
#endif

//Utility function for rounding up integer division.
uint64_t ceilDiv(uint64_t, uint64_t);

//Allocates space for an fs_VCB
int allocateVCB(fs_VCB**);

//Read block
uint64_t readFileSystem(void*, uint64_t, uint64_t);

//Write block to disk
uint64_t writeFileSystem(void*, uint64_t, uint64_t);

//Free block
void freeFileSystem(void*, uint64_t, uint64_t);

//Checks if there is enough block
int checkIfStorageIsAvalibale(int numberOfRequestedBlocks);

//Return first free block
uint64_t getNextFreeBlock();

//Read the VCB
uint64_t readVCB();

//Write the VCB to disk
uint64_t writeVCB();

fs_VCB* getVCB();

//printing the VCB in hex and ASCII
void printVCB();

/*******************************************************************************
 * Creates new volume with fileName, volumeSize and blockSize.
 * Initializing VCB and inodes.
 * Returns
 * 0 if Volume successfully created .
 * 1 if File exists
 * 2 if Insufficient space
 * 3 if Volume already exists.
 * 4 if System does not initialized.
 ********************************************************************************/
int createVolume(char*, uint64_t, uint64_t);

//open volume
void openVolume(char*);
//close
void closeVolume();
