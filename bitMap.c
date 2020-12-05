/**************************************************************
* Class:  CSC-415
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613, 917048657, 916260714, 918810235
* Project: Basic File System
*
* File: bitMap.c
*
* Description: This file the methods that modify the free-space bit vector
*
**************************************************************/

#include "bitMap.h"

void setBitMap(int A[], int k)
{
    A[k / 32] |= 1 << (k % 32);
}

void clearBitMap(int A[], int k)
{
    A[k / 32] &= ~(1 << (k % 32));
}

int findBitMap(int A[], int k)
{
    return ((A[k / 32] & (1 << (k % 32))) != 0);
}


