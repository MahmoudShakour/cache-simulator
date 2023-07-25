#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

const int numberOfAddressBits = 64;
int time = 1;
typedef struct
{
    char type[5];
    unsigned long address;
    int size;
} Access;

typedef struct
{

    int valid;
    int tag;
    int time_stamp;

} Line;

void parseArguments(int argc, char *argv[], int *numberOfSetBits, int *numberOfLines, int *numberOfBlockBits, char *fileName)
{
    int opt;
    while ((opt = getopt(argc, argv, "E:b:t:s:")) != -1)
    {

        switch (opt)
        {
        case 's':
            *numberOfSetBits = atoi(optarg);
        case 'E':
            *numberOfLines = atoi(optarg);
        case 'b':
            *numberOfBlockBits = atoi(optarg);
        case 't':
            strcpy(fileName, optarg);
        }
    }
}
int getNumberOfOperations(char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");
    int linescount = 0;
    while (!feof(file))
    {
        int ch = fgetc(file);
        if (ch == '\n')
        {
            linescount++;
        }
    }
    fclose(file);
    return linescount;
}

void parseOperations(char *fileName, Access *operations, int *numberOfOperations)
{
    FILE *file;
    file = fopen(fileName, "r");
    char type[5];
    unsigned int address;
    int size, i = 0;
    while (fscanf(file, " %s %x,%d", type, &address, &size) > 0)
    {
        strcpy(operations[i].type, type);
        operations[i].address = address;
        operations[i].size = size;
        i++;
    }
    *numberOfOperations = i;
    fclose(file);
}

void getAddressInfo(unsigned long address, int numberOfSetBits, int numberOfBlockBits, unsigned long *tag, unsigned long *set, unsigned long *blockOffset)
{
    for (int i = 0; i < 64; i++)
    {
        if (i < numberOfBlockBits)
        {
            (*blockOffset) |= ((1LL << i) & address);
        }
        else if ((i >= numberOfBlockBits) && (i < numberOfBlockBits + numberOfSetBits))
        {
            (*set) |= ((1LL << i) & address);
        }
        else
        {
            (*tag) |= ((1LL << i) & address);
        }
    }
    (*set) >>= numberOfBlockBits;
    (*tag) >>= (numberOfBlockBits + numberOfSetBits);
}

int isHit(int numberOfLines, int set, int tag, Line **cache)
{
    for (int j = 0; j < numberOfLines; j++)
    {
        if ((cache[set][j].valid == 1) && (cache[set][j].tag == tag))
        {
            cache[set][j].time_stamp = time++;
            return 1;
        }
    }
    return 0;
}

int processColdMiss(Line **cache, unsigned long set, unsigned long tag, int numberOfLines)
{
    for (int i = 0; i < numberOfLines; i++)
    {
        if (cache[set][i].valid == 0)
        {
            cache[set][i].valid = 1;
            cache[set][i].tag = tag;
            cache[set][i].time_stamp = time++;
            return 1;
        }
    }
    return 0;
}

void replaceWithLRU(Line **cache, unsigned long set, unsigned long tag, int numberOfLines)
{
    int last = 1e9, index = -1;
    for (int i = 0; i < numberOfLines; i++)
    {
        if (last > cache[set][i].time_stamp)
        {
            last = cache[set][i].time_stamp;
            index = i;
        }
    }
    cache[set][index].tag = tag;
    cache[set][index].time_stamp = time++;
}

void clearCache(Line **cache, unsigned long numberOfSetBits, unsigned long numberOfLines)
{
    for (int i = 0; i < 1 << numberOfSetBits; i++)
    {
        for (int j = 0; j < numberOfLines; j++)
        {
            cache[i][j].valid = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    int hitsCount = 0, missesCount = 0, evictionsCount = 0;
    int numberOfSetBits, numberOfLines, numberOfBlockBits, numberOfOperations;
    char fileName[30];

    parseArguments(argc, argv, &numberOfSetBits, &numberOfLines, &numberOfBlockBits, fileName);
    numberOfOperations = getNumberOfOperations(fileName);

    Access* operations=(Access*)malloc((numberOfOperations+5)*sizeof(Access));
    parseOperations(fileName, operations, &numberOfOperations);

    Line **cache = (Line **)malloc((1 << numberOfSetBits) * sizeof(Line *));
    for (int i = 0; i < (1 << numberOfSetBits); i++)
    {
        cache[i] = (Line *)malloc(numberOfLines * sizeof(Line));
    }

    clearCache(cache, numberOfSetBits, numberOfLines);

    for (int i = 0; i < numberOfOperations; i++)
    {
        if (strcmp(operations[i].type, "I") == 0)
            continue;

        unsigned long tag = 0, set = 0, blockOffset = 0;
        getAddressInfo(operations[i].address, numberOfSetBits, numberOfBlockBits, &tag, &set, &blockOffset);

        if (isHit(numberOfLines, set, tag, cache))
        {
            hitsCount++;
        }
        else
        {
            if (processColdMiss(cache, set, tag, numberOfLines))
            {
                missesCount++;
            }
            else
            {
                replaceWithLRU(cache, set, tag, numberOfLines);
                missesCount++;
                evictionsCount++;
            }
        }
        if (strcmp(operations[i].type, "M") == 0)
        {
            hitsCount++;
        }
    }

    printSummary(hitsCount, missesCount, evictionsCount);
    return 0;
}
