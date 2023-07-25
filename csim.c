#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

const int numberOfAddressBits = 64;

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
}

void getAddressInfo(unsigned long address, int numberOfSetBits, int numberOfBlockBits, unsigned long *tag, unsigned long *set, unsigned long *blockOffset)
{
    for(int i=0;i<64;i++){
        if(i<numberOfBlockBits){
            (*blockOffset)|=((1LL<<i)&address);
        }
        else if((i>=numberOfBlockBits)&&(i<numberOfBlockBits+numberOfSetBits)){
            (*set)|=((1LL<<i)&address);
        }
        else{
            (*tag)|=((1LL<<i)&address);
        }
    }
    (*set)>>=numberOfBlockBits;
    (*tag)>>=(numberOfBlockBits+numberOfSetBits);
}

int main(int argc, char *argv[])
{

    int numberOfSetBits, numberOfLines, numberOfBlockBits, numberOfOperations;
    char fileName[30];
    Access operations[20];

    parseArguments(argc, argv, &numberOfSetBits, &numberOfLines, &numberOfBlockBits, fileName);

    printf("%d %d %d %s\n", numberOfSetBits, numberOfLines, numberOfBlockBits, fileName);

    parseOperations(fileName, operations, &numberOfOperations);

    for (int i = 0; i < numberOfOperations; i++)
    {
        printf(" %s %lu,%d\n", operations[i].type, operations[i].address, operations[i].size);
        
    }

    Line cache[1 << numberOfSetBits][numberOfLines];

    for(int i=0;i<1<<numberOfSetBits;i++){
        for(int j=0;j<numberOfLines;j++){
            cache[i][j].valid=0;
            printf("%d\n",cache[i][j].valid);
        }
    }

    for (int i = 0; i < numberOfOperations; i++)
    {
        unsigned long tag=0,set=0,blockOffset=0;
        printf("%lu\n",operations[i].address);
        getAddressInfo(operations[i].address,numberOfSetBits,numberOfBlockBits,&tag,&set,&blockOffset);
        printf("%lu %lu %lu\n",tag,set,blockOffset);
        
    }

    // printSummary(0, 0, 0);
    return 0;
}
