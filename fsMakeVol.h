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

//amount of blocks to allocate per inode
#define DATA_BLOCKS_PER_INODE	4
#define VCB_START_BLOCK		0
#ifndef _MAKEVOL
#define _MAKEVOL

//structure to hold data
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

//function for rounding up integer division
uint64_t ceilDiv(uint64_t, uint64_t);

//allocates space for an fs_VCB
int allocateVCB(fs_VCB**);

//read blocks
uint64_t readFileSystem(void*, uint64_t, uint64_t);

//write block to disk
uint64_t writeFileSystem(void*, uint64_t, uint64_t);

//frees block
void freeFileSystem(void*, uint64_t, uint64_t);

//checks if there is enough blocks
int checkIfStorageIsAvalibale(int numberOfRequestedBlocks);

//return first free block
uint64_t getNextFreeBlock();

//reads from the VCB
uint64_t readVCB();

//write the VCB to disk
uint64_t writeVCB();

fs_VCB* getVCB();

//printing the VCB
void printVCB();


// creates new volume with fileName, volumeSize and blockSize.
int createVolume(char*, uint64_t, uint64_t);

//open volume
void openVolume(char*);
//close
void closeVolume();
