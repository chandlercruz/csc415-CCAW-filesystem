/**************************************************************
* Class:  CSC-415-02 Summer 2020
* Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 917048657 916260714 918810235
* Project: Basic File System 
*
* File: mfs.c
*
* Description: This file defines the methods to initialize, keep track of, and change our file system's inodes.
*
**************************************************************/
#include "mfs.h"

mfs_DIR* inodes;

//set number of inodes in array
size_t NumberOfElementsInInodesArray = sizeof(inodes)/sizeof(inodes[0]); 

void mfs_init() {
  printf("Initializing\n");

  uint64_t totalBytes = getVCB()->totalInodeBlocks * getVCB()->blockSize;
  printf("totalInodeBlocks %ld, blockSize %ld\n", getVCB()->totalInodeBlocks, getVCB()->blockSize);
  printf("Allocating %ld bytes for inodes.\n", totalBytes);

  inodes = calloc(getVCB()->totalInodeBlocks, getVCB()->blockSize);
  printf("Inodes allocated at %p.\n", inodes);

  uint64_t blocksRead = LBAread(inodes, getVCB()->totalInodeBlocks, getVCB()->inodeStartBlock);
  printf("Loaded %ld blocks of inodes into cache.\n", blocksRead);
  
  if(blocksRead != getVCB()->totalInodeBlocks) {
    printf("exist inodes do not loaded into cache.\n");
    mfs_close();
    exit(0);
  }
  mfs_setcwd("/root");
  printf(" ");
}

void writeInodes() {
  printf("Writing Inode......\n");
  LBAwrite(inodes, getVCB()->totalInodeBlocks, getVCB()->inodeStartBlock);
  printf(" ");
}
char inodeTypeNames[3][64] = { "I_FILE", "I_DIR", "I_UNUSED" };
char* getInodeTypeName(char* buf, InodeType type) {
  strcpy(buf, inodeTypeNames[type]);
  return buf;
}

//holds the directory path
char currentDirectoryPath[MAX_FILEPATH_SIZE];
char currentDirectoryPathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int currentDirectoryPathArraySize = 0;
//holds each part of the path
char requestedFilePath[MAX_FILEPATH_SIZE];
char requestedFilePathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int requestedFilePathArraySize = 0;

void parseFilePath(const char *pathname) { 
  printf("Parsing the File Path........\n");
  
  printf("Input: %s\n", pathname);

  //Clears previous values
  requestedFilePath[0] = '\0';
  requestedFilePathArraySize = 0;

  //Copies the path
  char _pathname[MAX_FILEPATH_SIZE];
  strcpy(_pathname, pathname);

  //Delimeter for token
  char* savePointer;
  char* token = strtok_r(_pathname, "/", &savePointer);

  //Sorting paths
  int isAbsolute = pathname[0] == '/';
  int isSelfRelative = !strcmp(token, ".");
  int isParentRelative = !strcmp(token, "..");

  if(token && !isAbsolute) {
    int maxLevel = isParentRelative ? currentDirectoryPathArraySize - 1 : currentDirectoryPathArraySize;
    for(int i=0; i<maxLevel; i++) {
      strcpy(requestedFilePathArray[i], currentDirectoryPathArray[i]);
      sprintf(requestedFilePath, "%s/%s", requestedFilePath, currentDirectoryPathArray[i]);
      requestedFilePathArraySize++;
    }
  }

  //Drop hidden files
  if(isSelfRelative || isParentRelative) {
    token = strtok_r(0, "/", &savePointer);
  }
  while(token && requestedFilePathArraySize < MAX_DIRECTORY_DEPTH) {

    strcpy(requestedFilePathArray[requestedFilePathArraySize], token);
    sprintf(requestedFilePath, "%s/%s", requestedFilePath, token);

    /* Printf for debug. */
    printf("\t%s\n", token);
    
    requestedFilePathArraySize++;
    token = strtok_r(0, "/", &savePointer);

  }
  printf("Output: %s\n", requestedFilePath);
  printf(" ");

}

void printFilePath() {
  for(int i=0; i<requestedFilePathArraySize; i++) {

    if(i<requestedFilePathArraySize-1) {
      printf("Directory %d: %s\n", i, requestedFilePathArray[i]);
    } else {
      printf("Filename: %s\n", requestedFilePathArray[i]);
    }

  }
}

mfs_DIR* getInode(const char *pathname){
  printf("Getting Inode......\n");
  printf("Searching path: '%s'\n", pathname);
  for (size_t i = 0; i < getVCB()->totalInodes; i++) {
    printf("\tInode path: '%s'\n", inodes[i].path);
    if (strcmp(inodes[i].path, pathname) == 0) {
      printf("----------------------------------------------------------------\n");
      return &inodes[i];
    }
  }
  printf("Inode with path '%s' does not exist.\n", pathname);
  printf(" ");
  return NULL;
}

mfs_DIR* getFreeInode(){
  printf("Get Free Inode......\n");
  mfs_DIR* returnediNode;
  for (size_t i = 0; i < getVCB()->totalInodes; i++) {
      // if 0, return to be used
    if (inodes[i].inUse == 0) {
      inodes[i].inUse = 1;
      returnediNode = &inodes[i];
      printf(" ");   
      return returnediNode;
    }
  }
  printf(" ");
  return NULL;
}

mfs_DIR* createInode(InodeType type, const char* path){
  printf("Creating Inode......\n");
  mfs_DIR* inode;
  char parentPath[MAX_FILEPATH_SIZE];
  mfs_DIR* parentNode;
  //Get time
  time_t currentTime;
  currentTime = time(NULL);
  if (checkValidityOfPath() == 0){
    printf(" ");
    return NULL;
  }
  //to reciev next inode
  inode = getFreeInode();
  //assign the parent to the new inode
  getParentPath(parentPath, path);
  parentNode = getInode(parentPath);
  
  //Propperties are set
  inode->type = type;
  strcpy(inode->name , requestedFilePathArray[requestedFilePathArraySize - 1]);
  sprintf(inode->path, "%s/%s", parentPath, inode->name);
  inode->lastAccessTime = currentTime;
  inode->lastModificationTime = currentTime;

  //Assign the parent
  if (!setParent(parentNode, inode)) {
    freeInode(inode);
    printf("Error, reverting changes.\n");
    return NULL;
  }
    
  printf("Creating inode successful. '%s'.\n", path);       
  return inode;

}

int parentHasChild(mfs_DIR* parent, mfs_DIR* child) {
  for( int i = 0; i < parent->numChildren; i++ ) {
    if(!strcmp(parent->children[i], child->name)) {
      return 1;
    }
  }
  return 0;
}

//set the parent and child inodes to each other
int setParent(mfs_DIR* parent, mfs_DIR* child){
  printf("Set Parent......\n");
  //checking if the parent can take more children
  if(parent->numChildren == MAX_NUMBER_OF_CHILDREN) {
    printf("Folder '%s' has maximum children.\n", parent->path);
    printf(" ");
    return 0;
  }
  //Checking if a child already exists
  if(parentHasChild(parent, child)) {
    printf("Folder '%s' already exists.\n", child->path);
    return 0;
  }
  //Set rest of children to parent
  strcpy(parent->children[parent->numChildren], child->name);
  parent->numChildren++;
  parent->lastAccessTime = time(0);
  parent->lastModificationTime = time(0);
  parent->sizeInBlocks += child->sizeInBlocks;
  parent->sizeInBytes += child->sizeInBytes;

  strcpy(child->parent, parent->path);
  sprintf(child->path, "%s/%s", parent->path, child->name);

  printf("Set parent of '%s' to '%s'.\n", child->path, child->parent); 
  printf(" ");
  return 1;
}

int removeFromParent(mfs_DIR* parent, mfs_DIR* child) {
  printf("Remove Parent......");
  //Loop through parents list
  for(int i=0; i<parent->numChildren; i++) {
    if(!strcmp(parent->children[i], child->name)) {
      strcpy(parent->children[i], "");
      parent->numChildren--;
      parent->sizeInBlocks -= child->sizeInBlocks;
      parent->sizeInBytes -= child->sizeInBytes;
      return 1;
    }
  }

  printf("Could not find child '%s' in parent '%s'.\n", child->name, parent->path);
  return 0;

}

//method will return NULL if the parent's path cannot be found, otherwise sets the passed buffer to the path and returns it
char* getParentPath(char* buf ,const char* path){
  printf("Getting Parent Path......");
  //find the parent string
  parseFilePath(path);

  char parentPath[MAX_FILEPATH_SIZE] = "";
  //Loop until the second to last element
  for(int i=0; i<requestedFilePathArraySize - 1; i++) {
    strcat(parentPath, "/");
    strcat(parentPath, requestedFilePathArray[i]);
  }
  strcpy(buf, parentPath);
  printf("Input: %s, Parent Path: %s\n", path, buf);
  return buf;
}

//Checks path validity
int checkValidityOfPath(char* buf){
    printf("Checking the validity of Path......\n");
    if (buf == NULL) {
        printf("The validity of the Path is valid. \n");
        return 1;
    }
    else {
        printf("The validity of the Path is invalid. \n");
        return 0;
    }
    //Returns 0 if invalid and 1 for valid.
}

mfs_DIR* getInodeByID(int id) {
  if(0 <= id < getVCB()->totalInodes) {
    return &inodes[id];
  } else {
    return NULL;
  }
}

int writeBufferToInode(mfs_DIR* inode, char* buffer, size_t bufSizeBytes, uint64_t blockNumber) {
  printf("Writing buffer to inode......\n\n");
  int freeIndex = -1;
  for(int i=0; i<MAX_DATABLOCK_POINTERS; i++) {
    if(inode->directBlockPointers[i] == INVALID_DATABLOCK_POINTER) {
      //If there is no place for new dataBlock
      freeIndex = i;
      break;
    }
  }
  //If there is no place for new dataBlock
  if(freeIndex == -1) {
    return 0;
  }
  //Write buffered data to disk
  LBAwrite(buffer, 1, blockNumber);

  //Record the block number in the inode
  inode->directBlockPointers[freeIndex] = blockNumber;
  setBitmap(getVCB()->freeMap, blockNumber);
  writeVCB();

  inode->numDirectBlockPointers++;
  inode->sizeInBlocks++;
  inode->sizeInBytes += bufSizeBytes;
  inode->lastAccessTime = time(0);
  inode->lastModificationTime = time(0);
  writeInodes();
  return 1;

}

void freeInode(mfs_DIR* node){
  printf("Freeing inode: '%s'\n", node->path);
  node->inUse = 0;
  node->type = I_UNUSED;
  node->name[0] = NULL;
  node->path[0] = NULL;
  node->parent[0] = NULL;
  node->sizeInBlocks = 0;
  node->sizeInBytes = 0;
  node->lastAccessTime = 0;
  node->lastModificationTime = 0;

  //free the data blocks
  if(node->type == I_FILE){
    for (size_t i = 0; i < node->numDirectBlockPointers; i++) {
      int blockPointer = node->directBlockPointers[i];
      clearBitmap(getVCB()->freeMap, blockPointer);
    }
  }
  writeInodes();
}

void mfs_close() {
  printf("fs close");
  free(inodes);
}
 

int mfs_mkdir(const char *pathname, mode_t mode) { // return 0 for sucsess and -1 if not
  printf("fs make dir\n");
  // Parses file name
  char parentPath[256] = "";
  parseFilePath(pathname);

  for (size_t i = 0; i < requestedFilePathArraySize - 1; i++) {
     strcat(parentPath, "/");
     strcat(parentPath, requestedFilePathArray[i]);
  }
  
  mfs_DIR* parent = getInode(parentPath);
  if (parent) {
    for (size_t i = 0; i < parent->numChildren; i++){
      if(strcmp(parent->children[i], requestedFilePathArray[requestedFilePathArraySize - 1])){
          printf("Folder already exists!\n");
          return -1;
      }
    }
  } else {
    printf("Parent '%s' does not exist!\n", parentPath);
    return -1;
  }
  

  if( createInode(I_DIR, pathname)){
    writeInodes();
    return 0;
  }
  printf("Failed to make directory '%s'.\n", pathname);
  return -1;
}

//Upon success, returns a 0 otherwise returns a -1
int mfs_rmdir(const char *pathname) {
  printf("fs rm dir\n");
  mfs_DIR* node = getInode(pathname);
  if(!node) {
    printf("%s does not exist.\n", pathname);
    return -1;
  }
  mfs_DIR* parent = getInode(node->parent);
  if (node->type == I_DIR && node->numChildren == 0){
    printf("got this far\n");
    removeFromParent(parent,node);
    freeInode(node);
    return 0;
  }
  return -1;
}

mfs_DIR* mfs_opendir(const char *fileName) {
  printf("fs open dir");
  int ret = b_open(fileName, 0);
  if(ret < 0) {
      //return NULL;
    return NULL;
  }
  return getInode(fileName);
}

int readdirCounter = 0;
struct mfs_dirent directoryEntry;

struct mfs_dirent* mfs_readdir(mfs_DIR *dirp) {
  printf("fs read dir\n");
  
  if(readdirCounter == dirp->numChildren) {
    readdirCounter = 0;
    return NULL;
  }
  //Get child inode
  char childPath[MAX_FILEPATH_SIZE];
  sprintf(childPath, "%s/%s", dirp->path, dirp->children[readdirCounter]);
  mfs_DIR* child = getInode(childPath);
  directoryEntry.d_ino = child->id;
  strcpy(directoryEntry.d_name, child->name);

  //Increment the counter to the next child
  readdirCounter++;
  return &directoryEntry;
}

int mfs_closedir(mfs_DIR *dirp) {
  printf("fs close dir\n");
  return 0;
}

char * mfs_getcwd(char *buf, size_t size) {
  printf("fs get cwd\n");
  if(strlen(currentDirectoryPath) > size) {
    errno = ERANGE;
    return NULL;
  }
  strcpy(buf, currentDirectoryPath);
  return buf;
}

int mfs_setcwd(char *buf) {
  printf("fs set cwd\n");
  
  parseFilePath(buf);

  //Check if inode exists
  mfs_DIR* inode = getInode(requestedFilePath);
  if(!inode) {
    printf("Directory '%s' does not exist.\n", requestedFilePath);
    return 1;
  }
  //Clear
  currentDirectoryPath[0] = '\0';
  currentDirectoryPathArraySize = 0;
  //Copy pathname to currentDirectoryPathArray.
  for(int i=0; i<requestedFilePathArraySize; i++) {
    strcpy(currentDirectoryPathArray[i], requestedFilePathArray[i]);
    sprintf(currentDirectoryPath, "%s/%s", currentDirectoryPath, requestedFilePathArray[i]);
    currentDirectoryPathArraySize++;
  }

  printf("Set cwd to '%s'.\n", currentDirectoryPath);
  return 0;

}


void printCurrentDirectoryPath() {
  for(int i=0; i<currentDirectoryPathArraySize; i++) {
    if(i<currentDirectoryPathArraySize-1) {
      printf("Directory %d: %s\n", i, currentDirectoryPathArray[i]);
    } else {
      printf("Filename: %s\n", currentDirectoryPathArray[i]);
    }

  }
}

//Initial fs_isFile and fs_isDir
int mfs_isFile(char * path) {
  printf("isFile initial\n");
  mfs_DIR* inode = getInode(path);
  return inode ? inode->type == I_FILE : 0;
}

int mfs_isDir(char * path) {
  printf("mfs-isDir\n");
  mfs_DIR* inode = getInode(path);
  return inode ? inode->type == I_DIR : 0;
}

int mfs_delete(char* filePath) {
  printf("fs delete\n");
  //Get inode
  mfs_DIR* fileNode = getInode(filePath); 
  //Get parent
  mfs_DIR* parentNode = getInode(fileNode->parent);
  //Remove child from parent
  removeFromParent(parentNode, fileNode);
  //Clear properties on child inode so it is not found in search & Set inuse to false.
  freeInode(fileNode);
  return 0;
}

//fs_DIR needs rework to contain these fields.
int mfs_stat(const char *path, struct mfs_stat *buf) {
  printf("fs stat\n");
  mfs_DIR* inode = getInode(path);
  if(inode) {
    buf->st_size = 999;
    buf->st_blksize = getVCB()->blockSize;
    buf->st_blocks = 2;
    buf->st_accesstime = 1;
    buf->st_modtime = 1;
    buf->st_createtime = 1;
    return 1;
  }
  return 0;
}

