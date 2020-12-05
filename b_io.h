/**************************************************************
* Class:  CSC-415
* Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613, 917048657, 916260714, 918810235
* Project: Basic File System 
*
* File: b_io.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

int b_open (char * filename, int flags);
int b_read (int fd, char * buffer, int count);
int b_write (int fd, char * buffer, int count);
int b_seek (int fd, off_t offset, int whence);
void b_close (int fd);

#endif

