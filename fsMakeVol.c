/**************************************************************
* Class:  CSC-415
* Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 (Aaron), 917048657 (Chandler), 916260714 (Wesley), 918810235 (Chaoyi)
* Project: Basic File System 
*
* File: b_io.c
*
* Description: Creates the volume for the file system and holds
* 		free-space bit vector and the inodes on disk (for persistence)
**************************************************************/

#include "fsMakeVol.h"

//is the VCB initialized or not
int initialized = 0;

//Variables of volume control block
char header[11] = "FileSystem.";
uint64_t volumeSize;
uint64_t blockSize;
uint64_t diskBlocks;
uint32_t vcbBlock;
uint32_t totalVCBBlocks;
uint32_t inodeBlock;
uint32_t inodes;
uint32_t inodeBlocks;
uint32_t freeMapSize;

//pointer to volume control block
fs_VCB* vcbPointer;

uint64_t ceilDiv(uint64_t a, uint64_t b) {
  return (a + b - 1) / b;
}

int allocateVCB(fs_VCB** vcb_p) {
  *vcb_p = calloc(totalVCBBlocks, blockSize);
  return totalVCBBlocks;
}

uint64_t readFileSystem(void* buffer, uint64_t blockCount, uint64_t blockPosition) {
  if(!initialized) {
    printf("System not initialized.\n");
    return 0;
  }
  if(blockPosition + blockCount > vcbPointer->diskBlocks) {
    printf("Invalid block range.\n");
    return 0;
  }
  LBAread(buffer, blockCount, blockPosition);
  return blockCount;
}

uint64_t writeFileSystem(void* buffer, uint64_t blockCount, uint64_t blockPosition) {
  if(!initialized) {
    printf("System not initialized.\n");
    return 0;
  }
  if(blockPosition + blockCount > vcbPointer->diskBlocks) {
    printf("Invalid block range.\n");
    return 0;
  }
  LBAwrite(buffer, blockCount, blockPosition);
  for(int i=0; i<blockCount; i++) {
    setBitmap(vcbPointer->freeMap, blockPosition + i);
  }
  writeVCB();
  return blockCount;
}

void freeFileSystem(void* buffer, uint64_t blockCount, uint64_t blockPosition) {
  if(!initialized) {
    printf("System not initialized.\n");
    return;
  }
  if(blockPosition + blockCount > vcbPointer->diskBlocks) {
    printf("Invalid block range.\n");
    return;
  }
  for(int i=0; i<blockCount; i++) {
    clearBitmap(vcbPointer->freeMap, blockPosition + i);
  }
  writeVCB();
}

//gets next free block
uint64_t getNextFreeBlock(){
  for (int index = 0; index < diskBlocks; index++)
  {
    if(findBitmap(vcbPointer->freeMap, index) == 0) {
      return index; //returns index in the VolumeSpaceArray
    }
  }
 return -1;
}

uint64_t readVCB() {
  if(!initialized) {
    printf("readVCB: System not initialized.\n");
    return 0;
  }

  //reads Volume Control Block from disk (persistence)
  uint64_t blocksRead = LBAread(vcbPointer, totalVCBBlocks, VCB_START_BLOCK);
  printf("Read VCB in %d blocks starting at block %d.\n", totalVCBBlocks, VCB_START_BLOCK);
  return blocksRead;
}

uint64_t writeVCB() {
  if(!initialized) {
    printf("System not initialized.\n");
    return 0;
  }

  // Write openVCB_p to disk
  uint64_t blocksWritten = LBAwrite(vcbPointer, totalVCBBlocks, VCB_START_BLOCK);
  printf("Write VCB in %d blocks starting at block %d.\n", totalVCBBlocks, VCB_START_BLOCK);
  return blocksWritten;
}

fs_VCB* getVCB() {
  return vcbPointer;
}

void initializeVCB() {
  if(!initialized) {
    printf("System not initialized.\n");
    return;
  }
  printf("Initializing VCB......\n");
 
  sprintf(vcbPointer->header, "%s", header);
 
  //information on volume sizes and block locations
  vcbPointer->volumeSize = volumeSize;
  vcbPointer->blockSize = blockSize;
  vcbPointer->diskBlocks = diskBlocks;
  vcbPointer->vcbBlock = VCB_START_BLOCK;
  vcbPointer->totalVCBBlocks = totalVCBBlocks;
  vcbPointer->inodeBlock = inodeBlock;
  vcbPointer->inodes = inodes;
  vcbPointer->inodeBlocks = inodeBlocks;
  printf("Total Inode Blocks %ld", vcbPointer->inodeBlocks);

  vcbPointer->freeMapSize = freeMapSize;

  //Initialize freeBlockMap to 0
  for(int i=0; i<freeMapSize; i++) {
    vcbPointer->freeMap[i] = 0;
  }

  //Set bits in freeMap
  for(int i=0; i<inodeBlock+inodeBlocks; i++) {
    setBitmap(vcbPointer->freeMap, i);
  }
  printVCB();
  writeVCB();
}

void initializeInodes() {
  if(!initialized) {
    printf("initializeInodes: System not initialized.\n");
    return;
  }

  printf("Initializing inodes......\n");
  printf("Total disk blocks: %ld, total inodes: %d, total inode blocks: %d\n", diskBlocks, inodes, inodeBlocks);

  //allocates and initializes inodes
  fs_DIR* inodes = calloc(inodeBlocks, blockSize);
  inodes[0].id = 0;
  inodes[0].inUse = 1;
  inodes[0].type = I_DIR;
  strcpy(inodes[0].name, "root");
  strcpy(inodes[0].path, "/root");
  inodes[0].lastAccessTime = time(0);
  inodes[0].lastModificationTime = time(0);
  inodes[0].numDirectBlockPointers = 0;

  for(int i = 1; i<inodes; i++) {
    inodes[i].id = i;
    inodes[i].inUse = 0;
    inodes[i].type = I_UNUSED;
    strcpy(inodes[i].parent, "");
    strcpy(inodes[i].name, "");
    inodes[i].lastAccessTime = 0;
    inodes[i].lastModificationTime = 0;
    
    // set direct block pointers to -1
    for(int j=0; j<MAX_DATABLOCK_POINTERS; j++) {
      inodes[i].directBlockPointers[j] = INVALID_DATABLOCK_POINTER;
    }
    inodes[i].numDirectBlockPointers = 0;
    
  }
  //write inodes to disk (persistence
  char* char_p = (char*) inodes;
  LBAwrite(char_p, inodeBlocks, inodeBlock);
  printf("Wrote %d inodes of size %ld bytes each starting at block %d.\n", inodes, sizeof(fs_DIR), inodeBlock);
  free(inodes);
}

void printVCB() {
  int size = vcbPointer->totalVCBBlocks*(vcbPointer->blockSize);
  int width = 16;
  char* char_p = (char*)vcbPointer;
  char ascii[width+1];
  sprintf(ascii, "%s", "................");
  printf("Printing VCB......\n");
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
  printf("VCB Size is %d bytes\n", size);
}

void init(uint64_t thisVolumeSize, uint64_t thisBlockSize) {
  printf("");
  printf("Initialize Information......\n");
  printf("volume Size is %ld\n", volumeSize = thisVolumeSize);
  printf("blockSize is %ld\n", blockSize = thisBlockSize);
  printf("diskBlocks is%ld\n", diskBlocks = ceilDiv(volumeSize, blockSize));
  printf("freeMapSize is %d\n", freeMapSize = diskBlocks <= sizeof(uint32_t) * 8 ? 1 : diskBlocks / sizeof(uint32_t) / 8);
  printf("totalVCBBlocks is %d\n", totalVCBBlocks = ceilDiv(sizeof(fs_VCB) + sizeof(uint32_t[freeMapSize]), blockSize));
  printf("totalInodes is %d\n", inodes = (diskBlocks - inodeBlock) / (DATA_BLOCKS_PER_INODE + ceilDiv(sizeof(fs_DIR), blockSize)));
  printf("totalInodeBlocks is %d\n", inodeBlocks = ceilDiv(inodes * sizeof(fs_DIR), blockSize));
  printf("inodeBlock is %d\n", inodeBlock = VCB_START_BLOCK + totalVCBBlocks);
  printf("inodeSizeBytes is %ld\n", sizeof(fs_DIR));
  printf("inodeSizeBlocks is %ld\n", ceilDiv(sizeof(fs_DIR), blockSize));

  //allocate Volume Control Block in memory
  int size = allocateVCB(&vcbPointer);
  printf("VCB allocated in %d blocks.\n", size);

  initialized = 1;
  printf("Finishing Init.....\n");
}

int createVolume(char* volumeName, uint64_t thisVolumeSize, uint64_t thisBlockSize) {
  printf("Creating volume......\n");
  if(access(volumeName, F_OK) != -1) {
    printf("Volume already exists.\n", volumeName);
    return -3;
  }

  uint64_t existingVolumeSize = thisVolumeSize;
  uint64_t existingBlockSize = thisBlockSize;

  //initialize volume
  int retVal = startPartitionSystem (volumeName, &existingVolumeSize, &existingBlockSize);

  printf("Opened %s, Volume Size is %llu;  BlockSize is %llu; Return %d\n", volumeName, (ull_t)existingVolumeSize, (ull_t)existingBlockSize, retVal);

  //format disk
  if(!retVal) {
    init(thisVolumeSize, thisBlockSize);
    initializeVCB();
    initializeInodes();
  }

  closeVolume();
  return retVal;
}

void openVolume(char* volumeName) {
  printf("Opening the Volume......\n");
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
    printf("Volume was already be opened.\n", volumeName);
  }
}

void closeVolume() {
  printf("Closing  the Volume......\n");
  if(initialized) {
    closePartitionSystem();
    free(vcbPointer);
    initialized = 0;
  } else {
    printf("Do not have open volume yet.\n");
  }
}
