/**************************************************************
* Class:  CSC-415
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613, 917048657, 916260714, 918810235
* Project: Basic File System
*
* File: fileExplorer.c
*
* Description: Opens the file system's volume and prints the inode
*
**************************************************************/

#include "fsMakeVol.h"
#include "mfs.h"

char inodeTypenameBuffer[24];

void printInode(fs_DIR* inode) {
  printf("--------------------------Printing Inode------------------------\n");
  printf("id: %ld\n", inode->id);
  printf("type: %s\n", getInodeTypeName(inodeTypenameBuffer, inode->type));
  printf("name: %s\n", inode->name);
  printf("path: %s\n", inode->path);
  printf("parent: %s\n", inode->parent);
  
  //print inode children
  printf("children: ");
  for(int i=0; i < inode->numChildren; i++) {
    printf("%s ", inode->children[i]);
  }
  printf("\n");

  printf("numChildren: %d\n", inode->numChildren);

  //print inode block pointers
  printf("directBlockPointers: ");
  for(int i=0; i < inode->numDirectBlockPointers; i++) {
    printf("%d ", inode->directBlockPointers[i]);
  }
  printf("\nnumDirectBlockPointers: %d\n", inode->numDirectBlockPointers);
  printf("sizeInBlocks: %ld\n", inode->sizeInBlocks);
  printf("sizeInBytes: %ld\n", inode->sizeInBytes);
  printf("lastAccessTime: %lld\n", (long long) inode->lastAccessTime);
  printf("lastModificationTime: %lld\n", (long long) inode->lastModificationTime);
}

int main(int argc, char* argv[]) {
 if(argc<2) {
    printf("Missing arguments, syntax: fileExplorer volumeName\n");
    return 0;
  }
  char volumeName[MAX_FILENAME_SIZE];
  strcpy(volumeName, argv[1]);
  openVolume(volumeName);
  fs_init();

  for(int i=0; i<getVCB()->totalInodes; i++) {
    fs_DIR* inode = getInodeByID(i);
    printInode(inode);
  }
  fs_close();
  closeVolume();

}
