
#ifndef AFFINITY_H
#define AFFINITY_H
#include <omp.h> 
#include <stdbool.h>
#include "loops2.h"

typedef struct localSet
{
    int lo;
    int hi;
    omp_lock_t lock;
}localSet;

typedef struct chunk
{
    int start;
    int end;
}chunk;


localSet* array;
bool isArrayAlocated;
bool isArrayInitialized;
omp_lock_t allocationLock;
omp_lock_t initializationLock;


void GetNextChunk(int threadId,chunk* nextChunk);
void InitArray();
void StealChunks(int loopid);
void AllocateArray();
void RunChunk(int loopid, chunk nextChunk);
void GetStolenChunk(chunk* stolenChunk);


#endif /* AFFINITY_H */

