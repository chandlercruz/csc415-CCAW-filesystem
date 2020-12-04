/**************************************************************
* Class:  CSC-415-02 Summer 2020
* Name: 
* Student ID: 
* Project: Basic File System 
*
* File: mfs.c
*
* Description: This file defines the methods to initialize, keep track of, and change our file system's inodes.
*
**************************************************************/

#include "mfs.h"

mfs_DIR* inodes;

// calculate element number in inodes array
size_t numberOfElement = sizeof(inodes)/sizeof(inodes[0]);

void mfs_init() {
  printf("Initializing\n");

  uint64_t totalBytes = getVCB()->inodeBlocks * getVCB()->blockSize;
  printf("Total Inode Blocks is %ld, blockSize is %ld\n", getVCB()->inodeBlocks, getVCB()->blockSize);
  printf("Allocating %ld bytes.\n", totalBytes);

  inodes = calloc(getVCB()->inodeBlocks, getVCB()->blockSize);
  printf("Inodes allocated at %p.\n", inodes);

  uint64_t blocksRead = LBAread(inodes, getVCB()->inodeBlocks, getVCB()->inodeBlock);
  printf("Loaded %ld blocks into cache.\n", blocksRead);
  
  if(blocksRead != getVCB()->inodeBlocks) {
    printf("exist inodes do not loaded into cache.\n");
    mfs_close();
    exit(0);
  }

  mfs_setcwd("/root");

  printf(" ");
}

void writeInodes() {
  printf("Writing Inode......");
  LBAwrite(inodes, getVCB()->inodeBlocks, getVCB()->inodeBlock);
  printf(" ");
}

char inodeTypeNames[3][64] = { "I_FILE", "I_DIR", "I_UNUSED" };

char* getInodeTypeName(char* buf, InodeType type) {
  strcpy(buf, inodeTypeNames[type]);
  return buf;
}

//holding current directory path
char currentDirectoryPath[MAX_FILEPATH_SIZE];
char currentDirectoryPathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int currentDirectoryPathArraySize = 0;

//hold each level of path
char requestedFilePath[MAX_FILEPATH_SIZE];
char requestedFilePathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int requestedFilePathArraySize = 0;

void parseFilePath(const char *pathname) { 
  printf("Parsing the File Path........\n");
  
  printf("Input: %s\n", pathname);

  //Clear previous value and count
  requestedFilePath[0] = '\0';
  requestedFilePathArraySize = 0;

  //copy of pathname.
  char _pathname[MAX_FILEPATH_SIZE];
  strcpy(_pathname, pathname);

  //Set up tokenizer
  char* savePointer;
  char* token = strtok_r(_pathname, "/", &savePointer);

  //Categorize pathname
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

  //Discard '.' , '..'
  if(isSelfRelative || isParentRelative) {
    token = strtok_r(0, "/", &savePointer);
  }

  while(token && requestedFilePathArraySize < MAX_DIRECTORY_DEPTH) {
    strcpy(requestedFilePathArray[requestedFilePathArraySize], token);
    sprintf(requestedFilePath, "%s/%s", requestedFilePath, token);
    printf("\t%s\n", token);
    requestedFilePathArraySize++;
    token = strtok_r(0, "/", &savePointer);
  }

  printf("Output: %s\n", requestedFilePath);
  printf(" ");
}

//Added to test parseFileName
void printFilePath() {
  for(int i=0; i<requestedFilePathArraySize; i++) {
    if(i<requestedFilePathArraySize-1) {
      printf("Directory %d is %s\n", i, requestedFilePathArray[i]);
    } else {
      printf("Filename is %s\n", requestedFilePathArray[i]);
    }

  }
}

mfs_DIR* getInode(const char *pathname){
  printf("Getting Inode......");
  // Loop and find requested node, return that node return Null if does not exists
  printf("Searching path: '%s'\n", pathname);
  for (size_t i = 0; i < getVCB()->inodes; i++) {
    //printf("\tInode path: '%s'\n", inodes[i].path);
    if (strcmp(inodes[i].path, pathname) == 0) {
      printf("\tInode path: '%s'\n", inodes[i].path);
      printf(" ");
      return &inodes[i];
    }
  }
  printf("Inode path '%s' does not exist.\n", pathname);
  printf(" ");

  return NULL;

}

mfs_DIR* getFreeInode(){
  printf("Get Free Inode......\n");
  // Search through inodes, return the first avalable inode,
  // Update inUse of the returned node to 1 (Which means it's in use right now)
  // if there is no free inode return NULL
  mfs_DIR* returnediNode;

  for (size_t i = 0; i < getVCB()->inodes; i++) {
    if (inodes[i].inUse == 0) { // if the inode inUse equals 0, that means it is free so we return it
      inodes[i].inUse = 1; // update the node to be in use before returning it
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
  // returns inode
  mfs_DIR* inode;
  char parentPath[MAX_FILEPATH_SIZE];
  mfs_DIR* parentNode;

  //Obtain current time
  time_t currentTime;
  currentTime = time(NULL);

  //call checkValidityOfPath() return NULL if false
  if (checkValidityOfPath() == 0){
    return NULL;
  }

  //receive the next inode
  inode = getFreeInode();

  //assign the parent to new inode
  getParentPath(parentPath, path);
  parentNode = getInode(parentPath);
  
  //Set properties to inode
  inode->type = type;
  strcpy(inode->name , requestedFilePathArray[requestedFilePathArraySize - 1]);
  sprintf(inode->path, "%s/%s", parentPath, inode->name);
  inode->lastAccessTime = currentTime;
  inode->lastModificationTime = currentTime;

  //set the parent
  if (!setParent(parentNode, inode)) {
    freeInode(inode);
    printf("Error, reverting changes.\n");
    return NULL;
  }
    
  printf("Creating inode successfully.");
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
    printf("Folder '%s' had maximum children.\n", parent->path);
    printf(" ");
    return 0;
  }

  //checking if a child like the one passed, already exists
  if(parentHasChild(parent, child)) {
    printf("Folder '%s' was already exists.\n", child->path);
    return 0;
  }

  //set the rest of the parent and children
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
  printf("Remove Parent......\n");
  
  //Loop through parents list
  for(int i=0; i<parent->numChildren; i++) {
    if(!strcmp(parent->children[i], child->name)) {
      //Clear entry in children parents list
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

char* getParentPath(char* buf ,const char* path){
  printf("Getting Parent Path......\n");

  //find the parent string "path"
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

//checks if the path is valid
int checkValidityOfPath(){
  printf("Checking the validity of Path......\n");
  // Returns 0 if the path is invalid, and 1 for valid.
}

mfs_DIR* getInodeByID(int id) {
  if(0 <= id < getVCB()->inodes) {
    return &inodes[id];
  } else {
    return NULL;
  }
}

int writeBufferToInode(mfs_DIR* inode, char* buffer, size_t bufSizeBytes, uint64_t blockNumber) {
  printf("Writing buffer to inode......\n");
  int freeIndex = -1;
  for(int i=0; i<MAX_DATABLOCK_POINTERS; i++) {
    if(inode->directBlockPointers[i] == INVALID_DATABLOCK_POINTER) {
      freeIndex = i;
      break;
    }
  }
  //If there is no place to put the new dataBlock pointer
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
  printf("mfs close\n");
  free(inodes);
}
 

int mfs_mkdir(const char *pathname, mode_t mode) {
  printf("mfs make dir\n");
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
  printf("mfs rm dir\n");
  mfs_DIR* node = getInode(pathname);
  if(!node) {
    printf("%s does not exist.\n", pathname);
    return -1;
  }
  mfs_DIR* parent = getInode(node->parent);
  if (node->type == I_DIR && node->numChildren == 0){
    removeFromParent(parent,node);
    freeInode(node);
    return 0;
  }
  return -1;
}

mfs_DIR* mfs_opendir(const char *fileName) {
  printf("mfs open dir\n");
  int ret = b_open(fileName, 0);
  if(ret < 0) {
    return NULL;
  }
  return getInode(fileName);
}

int readdirCounter = 0;
struct mfs_dirent directoryEntry;

struct mfs_dirent* mfs_readdir(mfs_DIR *dirp) {
  printf("mfs read dir\n");
  
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
  printf("mfs close dir\n");
  return 0;
}

char * mfs_getcwd(char *buf, size_t size) {
  printf("mfs get cwd\n");
  if(strlen(currentDirectoryPath) > size) {
    errno = ERANGE;
    return NULL;
  }
  strcpy(buf, currentDirectoryPath);
  return buf;
}

int mfs_setcwd(char *buf) {
  printf("mfs set cwd\n");
  
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

//Initial mfs_isFile and mfs_isDir
int mfs_isFile(char * path) {
  printf("isFile\n");
  mfs_DIR* inode = getInode(path);
  return inode ? inode->type == I_FILE : 0;
}

int mfs_isDir(char * path) {
  printf("isDir\n");
  mfs_DIR* inode = getInode(path);
  return inode ? inode->type == I_DIR : 0;
}

int mfs_delete(char* filePath) {
  printf("mfs delete\n");
  //Get inode
  mfs_DIR* fileNode = getInode(filePath); 
  //Get parent
  mfs_DIR* parentNode = getInode(fileNode->parent);
  //Remove child from parent
  removeFromParent(parentNode, fileNode);
  freeInode(fileNode);
  return 0;
}

//mfs_DIR needs rework to contain these fields.
int mfs_stat(const char *path, struct mfs_stat *buf) {
  printf("mfs stat\n");
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
