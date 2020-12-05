/**************************************************************
* Class:  CSC-415-02 Summer 2020
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 (Aaron), 917048657 (Chandler), 916260714 (Wesley), 918810235 (Chaoyi)
* Project: Basic File System - PentaFS
*
* File: b_io.h
*
* Description: The header file of the buffered I/O module that contains the prototypes of the methods.
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

int b_open(char *filename, int flags);
int b_read(int fd, char *buffer, int count);
int b_write(int fd, char *buffer, int count);
void b_close(int fd);

#endif
