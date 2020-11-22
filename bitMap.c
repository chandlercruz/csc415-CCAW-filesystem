/**************************************************************
* Class:  CSC-415-02 Summer 2020
* Name: 
* Student ID: 
* Project: Basic File System 
*
* File: bitMap.c
*
* Description: This file defines the methods that change our free-space bit vector.
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

