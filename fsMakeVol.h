/**************************************************************
* Class:  CSC-415
* Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 (Aaron), 917048657 (Chandler), 916260714 (Wesley), 918810235 (Chaoyi)
* Project: Basic File System 
*
* File: fsMakeVol.h
*
* Description: header file that holds methods and routines used to set up volume
*              for file system
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

#define DATA_BLOCKS_PER_INODE	4	//Number of data blocks to allocate per on inode.
#define VCB_START_BLOCK		0

/* Struct for the VCB of the disk. Use allocateVCB to allocate
 * the proper amount of memory.
 */

#ifndef _MAKEVOL
#define _MAKEVOL

 //structure to hold data
typedef struct {
  char header[16];
  uint64_t volumeSize;
  uint64_t blockSize;
  uint64_t diskSizeBlocks;
  uint64_t vcbStartBlock;
  uint64_t totalVCBBlocks;
  uint64_t inodeStartBlock;
  uint64_t totalInodes;
  uint64_t totalInodeBlocks;
  uint64_t freeMapSize;
  uint32_t freeMap[];
} mfs_VCB;
#endif

//function for rounding up integer division
uint64_t ceilDiv(uint64_t, uint64_t);

//allocates space for mfs_VCB
int allocateVCB(mfs_VCB**);

//read blocks
uint64_t fsRead(void*, uint64_t, uint64_t);

//write block to disk
uint64_t fsWrite(void*, uint64_t, uint64_t);

//frees block
void fsFree(void*, uint64_t, uint64_t);

//checks if there is enough blocks
int checkIfStorageIsAvalibale(int numberOfRequestedBlocks);

//return first free block
uint64_t getFreeBlock();

//reads from the VCB
uint64_t readVCB();

//write the VCB to disk
uint64_t writeVCB();
mfs_VCB* getVCB();

//printing the VCB
void printVCB();

// creates new volume with fileName, volumeSize and blockSize.
int createVolume(char*, uint64_t, uint64_t);

//open volume
void openVolume(char*);
//close volume
void closeVolume();

