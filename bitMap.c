/**************************************************************
* Class:  CSC-415
* Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 (Aaron), 917048657 (Chandler), 916260714 (Wesley), 918810235 (Chaoyi)
* Project: Basic File System 
*
* File: bitMap.c
*
* Description: Has the  methods that modify the free-space bit vector
*
**************************************************************/
#include "bitMap.h"

void setBitmap(int A[], int k)
{
    A[k / 32] |= 1 << (k % 32);
}

void clearBitmap(int A[], int k)
{
    A[k / 32] &= ~(1 << (k % 32));
}

int findBitmap(int A[], int k)
{
    return ((A[k / 32] & (1 << (k % 32))) != 0);
}

