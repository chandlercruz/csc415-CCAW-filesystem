/**************************************************************
* Class:  CSC-415-02 Summer 2020
* Name: Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
* Student ID: 916913613 (Aaron), 917048657 (Chandler), 916260714 (Wesley), 918810235 (Chaoyi)
* Project: Basic File System - PentaFS
*
* File: bitMap.c
*
* Description: This file defines the methods that change our free-space bit vector.
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


