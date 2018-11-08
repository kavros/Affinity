#ifndef QUEUE_H
#define QUEUE_H
#include <omp.h> 
#include<stdbool.h>

typedef struct chunk
{
    int lo,hi;
    struct chunk* next;
    
}chunk;

typedef struct localSet
{
    chunk* head;
    int remainingChunks;
}localSet;

localSet* array;
omp_lock_t lock;

int totalPositions;

void AllocateLocalSets(int nThreads);
void InitLocalSets(int nThreads,int iterations);
void AddChunk(int pos,int lo,int hi);

chunk* GetNextChunk(int threadId);
chunk* StealChunk();


//void DeleteChunk(int threadId,int lo,int hi);
//void DealocateLocalSet();


#endif /* QUEUE_H */

