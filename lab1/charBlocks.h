#ifndef CHARBLOCKS_H_
#define CHARBLOCKS_H_
#include <stdlib.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>


char** arrayB;

void creationArrayD(int arraySize, int blockSize);

void creationArrayS(int arraySize, int blockSize);

void deleteArrayD(int arraySize);

void deleteArrayS(int arraySize);

void addBlockD(int arraySize, int blockSize, int number);

void deleteBlockD(int arraySize, int number);

void addBlockS(int arraySize, int blockSize, int number);

void deleteBlockS(int arraySize, int number);

int getSumOfSymbols(char* block, int blockSize);

char* getBlockWithSimilarSum(int arraySize, int blockSize, int index);

void fillBlock(char* block, int blockSize);

#endif