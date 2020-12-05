/**************************************************************
* Class:  CSC-415
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613, 917048657, 916260714, 918810235
* Project: Basic File System
*
* File: b_io.c
* 
* Description: This file allocates the file system's volume
*		and holds the free-space bit vector and inodes
* 		on disk (for persistence)
*
**************************************************************/

#include "fsMakeVol.h"

int initialized = 0; //is the VCB initialized or not

//Variables of volume control block
char header[10] = "FileSystem";
uint64_t volumeSize;
uint64_t blockSize;
uint64_t diskSizeBlocks;
uint32_t vcbStartBlock;
uint32_t totalVCBBlocks;
uint32_t inodeStartBlock;
uint32_t totalInodes;
uint32_t totalInodeBlocks;
uint32_t freeMapSize;

fs_VCB* openVCB_p; //pointer to volume control block

uint64_t ceilDiv(uint64_t a, uint64_t b) {
  return (a + b - 1) / b;
}

int allocateVCB(fs_VCB** vcb_p) {
  *vcb_p = calloc(totalVCBBlocks, blockSize);
  return totalVCBBlocks;
}

uint64_t fsRead(void* buf, uint64_t blockCount, uint64_t blockPosition) {
  if(!initialized) {
    printf("File system isn't initialized.\n");
    return 0;
  }
  if(blockPosition + blockCount > openVCB_p->diskSizeBlocks) {
    printf("Invalid block range.\n");
    return 0;
  }
  LBAread(buf, blockCount, blockPosition);
  return blockCount;
}

uint64_t fsWrite(void* buf, uint64_t blockCount, uint64_t blockPosition) {
  if(!initialized) {
    printf("File System isn't initialized.\n");
    return 0;
  }
  if(blockPosition + blockCount > openVCB_p->diskSizeBlocks) {
    printf("Invalid block range.\n");
    return 0;
  }
  LBAwrite(buf, blockCount, blockPosition);
  for(int i=0; i<blockCount; i++) {
    setBitMap(openVCB_p->freeMap, blockPosition + i);
  }
  writeVCB();
  return blockCount;
}

void fsFree(void* buf, uint64_t blockCount, uint64_t blockPosition) {
  if(!initialized) {
    printf("File System isn't initialized.\n");
    return;
  }
  if(blockPosition + blockCount > openVCB_p->diskSizeBlocks) {
    printf("Invalid block range.\n");
    return;
  }
  for(int i=0; i<blockCount; i++) {
    clearBitMap(openVCB_p->freeMap, blockPosition + i);
  }
  writeVCB();
}

//gets next free block
uint64_t getFreeBlock(){
  for (int index = 0; index < diskSizeBlocks; index++)
  {
    if(findBitMap(openVCB_p->freeMap, index) == 0) {
      return index; //returns index in the VolumeSpaceArray
    }
  }
 return -1;
}

uint64_t readVCB() {
  if(!initialized) {
    printf("File System isn't initialized.\n");
    return 0;
  }

  //reads Volume Control Block from disk (persistence)
  uint64_t blocksRead = LBAread(openVCB_p, totalVCBBlocks, VCB_START_BLOCK);
  printf("Read VCB in %d blocks starting at block %d.\n", totalVCBBlocks, VCB_START_BLOCK);
  return blocksRead;
}

uint64_t writeVCB() {
  if(!initialized) {
    printf("File System isn't initialized.\n");
    return 0;
  }

  //Write openVCB_p to disk
  uint64_t blocksWritten = LBAwrite(openVCB_p, totalVCBBlocks, VCB_START_BLOCK);
  printf("Wrote VCB in %d blocks starting at block %d.\n", totalVCBBlocks, VCB_START_BLOCK);
  return blocksWritten;
}

fs_VCB* getVCB() {
  return openVCB_p;
}

void initializeVCB() {
  if(!initialized) {
    printf("System isn't initialized.\n");
    return;
  }
  printf("INIT VCB...\n");
 
  sprintf(openVCB_p->header, "%s", header); 
 
  //information on VCB's volume sizes and block locations
  openVCB_p->volumeSize = volumeSize;
  openVCB_p->blockSize = blockSize;
  openVCB_p->diskSizeBlocks = diskSizeBlocks;
  openVCB_p->vcbStartBlock = VCB_START_BLOCK;
  openVCB_p->totalVCBBlocks = totalVCBBlocks;
  openVCB_p->inodeStartBlock = inodeStartBlock;
  openVCB_p->totalInodes = totalInodes;
  openVCB_p->totalInodeBlocks = totalInodeBlocks;
  printf("TOTALINODEBLOCKS %ld", openVCB_p->totalInodeBlocks);
  openVCB_p->freeMapSize = freeMapSize;

  //initialize freeMap
  for(int i=0; i<freeMapSize; i++) {
    openVCB_p->freeMap[i] = 0;
  }

  //set the bits in freeMap
  for(int i=0; i<inodeStartBlock+totalInodeBlocks; i++) {
    setBitMap(openVCB_p->freeMap, i);
  }
  printVCB();
  writeVCB();
}

void initializeInodes() {
 	if(!initialized) {
  		printf("System isn' initialized.\n");
  	  	return;
  	}

  	printf("INIT INODES..\n");

  	//allocates and initializes inodes
  	fs_DIR* inodes = calloc(totalInodeBlocks, blockSize);
  	inodes[0].id = 0;
  	inodes[0].inUse = 1;
  	inodes[0].type = I_DIR;
  	strcpy(inodes[0].name, "root");
  	strcpy(inodes[0].path, "/root");
	inodes[0].lastModificationTime = time(0);
  	inodes[0].lastAccessTime = time(0);
  	inodes[0].numDirectBlockPointers = 0;

  	for(int i = 1; i<totalInodes; i++) {
    		inodes[i].id = i;
    		inodes[i].inUse = 0;
    		inodes[i].type = I_UNUSED;
    		strcpy(inodes[i].parent, "");
    		strcpy(inodes[i].name, "");
    		inodes[i].lastAccessTime = 0;
    		inodes[i].lastModificationTime = 0;
    
    //set direct block pointers to -1
    	for(int j=0; j<MAX_DATABLOCK_POINTERS; j++) {
      		inodes[i].directBlockPointers[j] = INVALID_DATABLOCK_POINTER;
    	}
    	inodes[i].numDirectBlockPointers = 0;
  }

  //write inodes to disk (persistence)
  char* char_p = (char*) inodes;
  LBAwrite(char_p, totalInodeBlocks, inodeStartBlock);
  free(inodes);
}

void printVCB() {
  int size = openVCB_p->totalVCBBlocks*(openVCB_p->blockSize);
  int width = 16;
  char* char_p = (char*)openVCB_p;
  char ascii[width+1];
  sprintf(ascii, "%s", "................");
  printf("PRINTING VolumeControlBlock..\n");
  for(int i = 0; i<size; i++) {
    printf("%02x ", char_p[i] & 0xff);
    if(char_p[i]) {
      ascii[i%width] = char_p[i];
    }
    if((i+1)%width==0&&i>0) {
      ascii[i%width+1] = '\0';
      printf("%s\n", ascii);
      sprintf(ascii, "%s", "................");
    } else if (i==size-1) {
      for(int j=0; j<width-(i%(width-1)); j++) {
        printf("   ");
      }
      ascii[i%width+1] = '\0';
      printf("%s\n", ascii);
      sprintf(ascii, "%s", "................");
    }
  }
  printf("VolumeControlBlock Size: %d bytes\n", size);
}

void init(uint64_t _volumeSize, uint64_t _blockSize) {
  printf("INIT\n");
  printf("volumeSize: %ld\n", volumeSize = _volumeSize);
  printf("blockSize: %ld\n", blockSize = _blockSize);
  printf("diskSizeBlocks: %ld\n", diskSizeBlocks = ceilDiv(volumeSize, blockSize));
  printf("freeMapSize: %d\n", freeMapSize = diskSizeBlocks <= sizeof(uint32_t) * 8 ? 1 : diskSizeBlocks / sizeof(uint32_t) / 8);
  printf("totalVCBBlocks: %d\n", totalVCBBlocks = ceilDiv(sizeof(fs_VCB) + sizeof(uint32_t[freeMapSize]), blockSize));
  printf("inodeStartBlock: %d\n", inodeStartBlock = VCB_START_BLOCK + totalVCBBlocks);
  printf("totalInodes: %d\n", totalInodes = (diskSizeBlocks - inodeStartBlock) / (DATA_BLOCKS_PER_INODE + ceilDiv(sizeof(fs_DIR), blockSize)));
  printf("totalInodeBlocks: %d\n", totalInodeBlocks = ceilDiv(totalInodes * sizeof(fs_DIR), blockSize));
  printf("inodeSizeBytes: %ld\n", sizeof(fs_DIR));
  printf("inodeSizeBlocks: %ld\n", ceilDiv(sizeof(fs_DIR), blockSize));

  //allocate VolumeControlBlock in memory
  int vcbSize = allocateVCB(&openVCB_p);
  printf("VolumeControlBlock allocated in %d blocks.\n", vcbSize);
  initialized = 1; //setting initialized flag to true
}

int createVolume(char* volumeName, uint64_t _volumeSize, uint64_t _blockSize) {
  printf("CREATING VOLUME...\n");
  if(access(volumeName, F_OK) != -1) {
    printf("Volume already exists.\n");
    return -3;
  }

  uint64_t existingVolumeSize = _volumeSize;
  uint64_t existingBlockSize = _blockSize;

  //initialize volume
  int retVal = startPartitionSystem (volumeName, &existingVolumeSize, &existingBlockSize);

  printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", volumeName, (ull_t)existingVolumeSize, (ull_t)existingBlockSize, retVal);

  //format disk
  if(!retVal) {
    init(_volumeSize, _blockSize);
    initializeVCB();
    initializeInodes();
  }
  closeVolume();
  return retVal;
}

void openVolume(char* volumeName) {
  printf("OPENING VOLUME...\n");
  if(!initialized) {
    uint64_t existingVolumeSize;
    uint64_t existingBlockSize;
    int retVal =  startPartitionSystem(volumeName, &existingVolumeSize, &existingBlockSize);
    if(!retVal) {
      init(existingVolumeSize, existingBlockSize);
      readVCB();
      printVCB();
    }
  } else {
    printf("Volume was already opened\n");
  }
}

void closeVolume() {
  printf("CLOSING VOLUME...\n");
  if(initialized) {
    closePartitionSystem();
    free(openVCB_p);
    initialized = 0;
  } else {
    printf("Volume is not open yet, cannot be closed.\n");
  }
}
