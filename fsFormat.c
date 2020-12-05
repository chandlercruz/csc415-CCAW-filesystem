/**************************************************************
* Class:  CSC-415
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613, 917048657, 916260714, 918810235
* Project: Basic File System
*
* File: fsFormat.c
*
* Description: The main method formats the file system's volume onto a disk for persistence
*
**************************************************************/

#include "mfs.h"

int main (int argc, char* argv[]) {
  if(argc<4) {
    printf("No arguments, syntax: fsFormat volumeName volumeSize blockSize\n");
    return 0;
  }
  char volumeName[MAX_FILENAME_SIZE];
  uint64_t volumeSize;
  uint64_t blockSize;
  strcpy(volumeName, argv[1]);
  volumeSize = atoll(argv[2]);
  blockSize = atoll(argv[3]);
  createVolume(volumeName, volumeSize, blockSize);
  openVolume(volumeName);
  closeVolume();
  return 0;
}
