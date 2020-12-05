/**************************************************************
* Class:  CSC-415
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613, 917048657, 916260714, 918810235
* Project: Basic File System
*
* File: b_io.c
*
* Description: Buffered I/O- this file has the open, read, write, and close methods for
* 		our file system's functionality
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20

uint64_t bufSize;

typedef enum  {NoWRITE,WRITE} fileMode;

typedef struct b_fcb {
	int linuxFd;		//file descriptor
	char * buf;		//file buffer
	int index;		//position in the buffer
	int blockIndex;		//index of the block in FCB
	int buflen;		//how many bytes are in buffer
	fileMode mode; 
	fs_DIR* inode; 
	int eof;
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS]; //FCB array that holds the files

int startup = 0;	//has been initialized or not

//initializes the file system
void b_init ()
{
	bufSize = getVCB()->blockSize; //sets buffer size equal to block size of the file system

	//initialize fcbArray
	for (int i = 0; i < MAXFCBS; i++)
	{ 
		fcbArray[i].linuxFd = -1;
		fcbArray[i].mode = NoWRITE;
	}
	startup = 1;
}

//retrieves free file in fcbArray
int b_getFCB ()
{
	for (int i = 0; i < MAXFCBS; i++)
		{
			if (fcbArray[i].linuxFd == -1)
			{
				fcbArray[i].linuxFd = -2;
				return i;
			}
		}
	return (-1); 
}

// opens a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
int b_open (char * filename, int flags)
	{
	int fd;
	int returnFd;
	
	printf("b_open\n");

	if (startup == 0) b_init();  
	returnFd = b_getFCB();	//gets file descriptor
	
	//check if there was available fcb returned
	b_fcb* fcb;
	if(returnFd < 0) {
		return -1;
	}

	fcb = &fcbArray[returnFd];

	fs_DIR* inode = getInode(filename);
	if(!inode) {
		
		printf("b_open: %s does not yet exist.\n", filename);

		if(flags & O_CREAT) {

			printf("Creating %s\n", filename);
			inode = createInode(I_FILE, filename);
			char parentpath[MAX_FILENAME_SIZE];
			getParentPath(parentpath, filename);
			fs_DIR* parent = getInode(parentpath);
			setParent(parent, inode);
			writeInodes();
		} else {
			//file doesn't exist and O_CREAT isn't set
			return -1;
		}
	}
	fcb->inode = inode;

	// allocate buffer, change to bufSize+1, and change memcpy to strcpy
	fcb->buf = malloc (bufSize+1);
	if (fcb->buf  == NULL)
		{
		close (fd);
		fcb->linuxFd = -1;
		return -1;
		}	
	fcb->buflen = 0; 
	fcb->index = 0;	
	printf("b_open: Opened file '%s' with fd %d\n", filename, fd);
	return (returnFd);
	}

//writes to a buffer
int b_write (int fd, char * buffer, int count)
{
	if (startup == 0)
	{
		b_init();
	}

	//check if fd is between 0 and MAXFCBS-1
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1);
	}

	if (fcbArray[fd].linuxFd == -1)
	{
		return -1;
	}

	b_fcb* fcb = &fcbArray[fd];
	int freeSpace = bufSize - fcb->index;

	printf("b_write: count=%d, fcb->index=%d, freeSpace=%d\n", count, fcb->index, freeSpace);

	fcb->mode = WRITE;

	//calculate the amount of bytes to put at the end of the buffer
	int copyLength = freeSpace > count ? count : freeSpace;
	int secondCopyLength = count - copyLength;

	printf("Copying first segment to fcb->buf+%d: %d bytes\n", fcb->index, copyLength);
	memcpy(fcb->buf+fcb->index, buffer, copyLength);
	fcb->index += copyLength;
	fcb->index %= bufSize;

	if(secondCopyLength != 0)
	{
		printf("Writing buffer to fd.\n"); 
		uint64_t indexOfBlock = getFreeBlock();

		if (indexOfBlock == -1){
			printf("There is not enough free space!");
			return 0;
		} else {
			printf("\n\nFCB buff:\n\n%s", fcb->buf);
			writeBufInode(fcb->inode, fcb->buf, copyLength + secondCopyLength, indexOfBlock);
		}
		fcb->index = 0;
		printf("Copying second segment to fcb->buf+%d: %d bytes\n", fcb->index, secondCopyLength);
		memcpy(fcb->buf+fcb->index, buffer+copyLength, secondCopyLength);
		fcb->index += secondCopyLength;
		fcb->index %= bufSize;
	}
	return copyLength + secondCopyLength;
}

// reads data into the fcb
int b_read (int fd, char * buffer, int count) {
  struct b_fcb* fcb = &fcbArray[fd];
  int bytesRemaining =fcb->buflen-fcb->index;
	printf("b_read: index = %d\n", fcb->index);
	printf("b_read: buflen = %d\n", fcb->buflen);
  if(bytesRemaining>count)
  {
    printf("Existing: Copying to %ld from %ld for %ld bytes.\n", buffer, fcb->buf+fcb->index, count);
    memcpy(buffer, fcb->buf+fcb->index, count);
    fcb->index += count;
    return count;
  } else {
    printf("Tail: Copying to %ld from %ld for %ld bytes.\n", buffer, fcb->buf+fcb->index, bytesRemaining);
    memcpy(buffer, fcb->buf+fcb->index, bytesRemaining);
    if(fcb->eof)
    {
	printf("EOF reached. Exiting.\n");
	fcb->index += bytesRemaining;
	return bytesRemaining;
    }
    if(fcb->blockIndex > fcb->inode->numDirectBlockPointers - 1)
    {
	printf("Block Index out-of-bounds.\n");
	return 0;
    }
    int blockNumber = fcb->inode->directBlockPointers[fcb->blockIndex]; 
    LBAread(fcb->buf, 1, blockNumber);
    fcb->blockIndex++;
    printf("Read new data.\n");
    printf("*********************************************************\n");
    printf("%s\n", fcb->buf);
    printf("*********************************************************\n");
    fcb->index = 0;
    int newBufferSize = fcb->buflen = strlen(fcb->buf);
	printf("%d bytes read.\n", newBufferSize);
    if(newBufferSize<bufSize)
    {
	fcb->eof = 1;
    }
    int remainderOfCount = count - bytesRemaining;
    int secondSegmentCount = newBufferSize>remainderOfCount?remainderOfCount:newBufferSize;
    printf("Second: Copying to %ld from %ld for %ld bytes.\n", buffer+bytesRemaining, fcb->buf+fcb->index, secondSegmentCount);
    memcpy(buffer+bytesRemaining, fcb->buf+fcb->index, secondSegmentCount);
    fcb->index+=secondSegmentCount;
    return bytesRemaining + secondSegmentCount;
  }
}
	
//closes file	
void b_close (int fd)
{
	b_fcb* fcb = &fcbArray[fd];
	printf("Closing file %d.\n", fd);
	if (fcb->mode == WRITE && fcb->index > 0)
	{
		printf("File was in write mode.\n");
		uint64_t indexOfBlock = getFreeBlock();
		if (indexOfBlock == -1) {
			printf("There is not enough free space!");
			return;
		} else {
			printf("Writing remaining bytes.\n");
			writeBufInode(fcb->inode, fcb->buf, fcb->index, indexOfBlock);
		}
	
	}
	close (fcb->linuxFd);
	free (fcb->buf);
	fcb->buf = NULL;
	fcb->linuxFd = -1;
}
