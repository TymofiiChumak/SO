#include "charBlocks.h"


void creationArrayD(int arraySize, int blockSize){
    char** newArray;
    newArray = (char**)calloc(arraySize,sizeof(char*));
    arrayB = newArray;
}
void creationArrayS(int arraySize, int blockSize){
    char *newArray[100000];
    for (int i = 0 ; i < arraySize ; i++){
        newArray[i] = NULL;
    }
    arrayB = newArray;
}


void deleteArrayD(int arraySize){
    for (int i = 0 ; i < arraySize ; i++){
        free(arrayB[i]);
    }
    free(arrayB);
}

void deleteArrayS(int arraySize){
    arrayB = NULL;
}

void addBlockD(int arraySize, int blockSize, int number){
    arrayB = realloc(arrayB,(arraySize + number) * sizeof(char *));
    for (int i = arraySize; i < arraySize + number; i++){
        arrayB[i] = calloc(blockSize,sizeof(char));
    }
}

void deleteBlockD(int arraySize, int number){
    for (int i = arraySize - number; i < arraySize ; i++){
        free(arrayB[i]);
    }
    arrayB = realloc(arrayB,(arraySize - number) * sizeof(char*));
}

void addBlockS(int arraySize, int blockSize, int number){
    for (int i = arraySize; i < arraySize + number; i++){
        char newBlock[blockSize];
        arrayB[i] = newBlock;
    }
}

void deleteBlockS(int arraySize, int number){
    for (int i = arraySize - number; i < arraySize ; i++){
        arrayB[i] = NULL;
    }
}


int getSumOfSymbols(char* block, int blockSize){
    if (block == NULL) return -1;
    int sum = 0;
    for (int i = 0 ; i < blockSize ; i ++){
        sum += block[i];
    }
    return sum;
}

char* getBlockWithSimilarSum(int arraySize, int blockSize, int index){
    if (index >= arraySize) return NULL;
    int value = getSumOfSymbols(arrayB[index],blockSize);
    char* result = NULL;
    int minDelta = INT_MAX;
    for (int i = 0 ; i < arraySize ; i ++){
        if (i != index && arrayB[i] != NULL){
            int delta = abs(value - getSumOfSymbols(arrayB[i],blockSize));
            if (delta < minDelta) {
                result = arrayB[i];
                minDelta = delta;
            }
        }
    }
    return result;
}

void fillBlock(char* block, int blockSize){
    srand(time(NULL));
    for (int i = 0 ; i < blockSize ; i ++){
        block[i] = (char)(rand() % 26 + 65);
    }
}