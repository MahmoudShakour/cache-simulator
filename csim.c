#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

struct access{
    char type[5];
    unsigned int address;
    int size;
};

void parseArguments(int argc, char *argv[], int *numberOfSetBits, int *numberOfLines, int *numberOfBytes, char *fileName)
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
            *numberOfBytes = atoi(optarg);
        case 't':
            strcpy(fileName, optarg);
        }
    }
}

void parseOperations(char *fileName,struct access *operations,int * numberOfOperations)
{
    FILE *file;
    file = fopen(fileName, "r");
    char type[5];
    unsigned int address;
    int size,i=0;
    while(fscanf(file, " %s %x,%d", type, &address,&size)>0){
        strcpy(operations[i].type,type);
        operations[i].address= address;
        operations[i].size= size;
        i++;
    }
    *numberOfOperations=i;
}

int main(int argc, char *argv[])
{

    int numberOfSetBits, numberOfLines, numberOfBytes,numberOfOperations;
    char fileName[30];
    struct access operations[20];

    parseArguments(argc, argv, &numberOfSetBits, &numberOfLines, &numberOfBytes, fileName);

    printf("%d %d %d %s\n", numberOfSetBits, numberOfLines, numberOfBytes, fileName);

    parseOperations(fileName,operations,&numberOfOperations);

    for(int i=0;i<numberOfOperations;i++){
        printf(" %s %x,%d\n",operations[i].type,operations[i].address,operations[i].size);
    }


    // printSummary(0, 0, 0);
    return 0;
}
