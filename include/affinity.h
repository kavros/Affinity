//B145772
#ifndef AFFINITY_H
#define AFFINITY_H
#include <omp.h> 
#include <stdbool.h>
#include "loops2.h"

typedef struct localSet
{
    int lo;                     // at the start determines the first iteration of the localSet.
                                // This value changed every time a thread
                                // acquires a new chunk from a localSet.
    
    int hi;                     // determines the last iteration of the localSet.
    
    omp_lock_t lock;            //each thread has to acquire this lock
                                //in order to change the values inside the localSet.
}localSet;

typedef struct chunk            // This struct determines the first
                                // and the last iteration of the chunk.
{
    int start;                  // start iteration of the chunk.
    int end;                    // last iteration of the chunk.
}chunk;


localSet* array;                // An array that represents the local set.
                                // For example, the position 0 of the array represents 
                                // the localSet for the thread 0.
                                // The position 1 represents the localSet for thread 1 etc.

bool isArrayAlocated;           // This variable is a shared between threads.
                                // The thread that acquires the allocation lock first
                                // is responsible to allocate the array with the localSet.

omp_lock_t allocationLock;      // Every thread try to allocate the array with 
                                // the localSets but only the first that acquires
                                // the lock is doing the allocation.


/**
 * Returns the next chunk of the thread.
 * When a thread finish the iterations inside the localSet 
 * this function returns the last value as the start of the chunk
 * after that, the user of the function must not call this function again.
 * @param threadId
 * @param nextChunk
 */
void GetNextChunk(int threadId,chunk* nextChunk);

/**
 * Initialize the array with the localSets.
 */
void InitArray();

/**
 * Steal chunks from the most loaded threads.
 * This function called after each thread finish 
 * iterations and tries to steal chunks until there are 
 * no more iterations remaining in any thread local set.
 * @param loopid
 */
void StealChunks(int loopid);

/**
 * Allocate the array with the localSet only once.
 */
void AllocateArray();

/**
 * Runs the given chunk.
 * @param loopid
 * @param nextChunk
 */
void RunChunk(int loopid, chunk nextChunk);

/**
 * Returns in to the variable stolenChunk the stolen chunk.
 * If there are no remaining iterations in any thread returns
 * -1 at the start variable inside the struct.
 * @param stolenChunk
 */
void GetStolenChunk(chunk* stolenChunk);


#endif /* AFFINITY_H */

