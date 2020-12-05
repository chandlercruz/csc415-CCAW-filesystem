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
  printf("PRINTING INODE\n");
  printf("ID: %ld\n", inode->id);
  printf("TYPE: %s\n", getInodeTypeName(inodeTypenameBuffer, inode->type));
  printf("NAME: %s\n", inode->name);
  printf("PATH: %s\n", inode->path);
  printf("PARENT: %s\n", inode->parent);
  printf("INODE CHILDREN: ");
  for(int i=0; i < inode->numChildren; i++) {   //print inode children
    printf("%s ", inode->children[i]);
  }
  printf("\n");
  printf("# OF CHILDREN: %d\n", inode->numChildren);

  //print inode block pointers
  printf("DIRECTBLOCKPOINTERS: ");
  for(int i=0; i < inode->numDirectBlockPointers; i++) {
    printf("%d ", inode->directBlockPointers[i]);
  }
  printf("\n# OF DIRECTBLOCKPOINTERS: %d\n", inode->numDirectBlockPointers);
  printf("SIZEINBLOCKS: %ld\n", inode->sizeInBlocks);
  printf("SIZEINBYTES: %ld\n", inode->sizeInBytes);
  printf("LAST ACCESS TIME: %lld\n", (long long) inode->lastAccessTime);
  printf("LAST MOD. TIME: %lld\n", (long long) inode->lastModificationTime);
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
