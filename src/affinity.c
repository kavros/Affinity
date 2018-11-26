//B145772
#include "../include/affinity.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
void GetStolenChunk(chunk* stolenChunk)
{
    
    int nthreads = omp_get_num_threads(); 
    int maxRemainingIterations =-1;
    int targetThread = -1;    
    int remainingIterations = -1;
    
    
    stolenChunk->start = -1;
    stolenChunk->end = -1;
    

    
    for(int i=0; i < nthreads; i++ )         // search in order to find
                                             // the most loaded local sets.
    {
        omp_set_lock(&(array[i].lock));      // before we access  each position
                                             // inside the array with the local sets
                                             // we need to acquire the appropriate lock.
        
        
        remainingIterations = array[i].hi-array[i].lo;  // calculates the remaining
                                                        //iterations of thread i.
        
        
        if(remainingIterations > maxRemainingIterations)
        {
            if(targetThread != -1)                              // at the first iteration
                                                                // the maximum remaining iterations
                                                                // is the remaining
                                                                // iterations so we don't release
                                                                // the lock until we find another
                                                                // thread with more remaining iterations.
            {
                omp_unset_lock(&(array[targetThread].lock));
            }
            maxRemainingIterations=remainingIterations;
            targetThread =i;
        }
        else
        {
            omp_unset_lock(&(array[i].lock));                   // unlock the thread's i local
                                                                // before move to the next iteration
            

        }
    }
    if(maxRemainingIterations  > 0)                             // if a thread find a stolen chunk
                                                                // assign the chunk to stolenChunk 
                                                                // variable, update local set of the target thread
                                                                // and release the lock.
    {               
        int chunkSize = (array[targetThread].hi-array[targetThread].lo)/nthreads;
        if(chunkSize == 0) chunkSize=1;
        stolenChunk->start = array[targetThread].lo;
        stolenChunk->end = stolenChunk->start +chunkSize;
        array[targetThread].lo = stolenChunk->end ;
        
        omp_unset_lock(&(array[targetThread].lock));
    }
    else if(maxRemainingIterations == 0)
    {
        omp_unset_lock(&(array[targetThread].lock));            //release lock of the target thread if there are no iterations to execute
    }
            
    
}

void RunChunk(int loopid, chunk nextChunk)
{
    //runs given chunk
     switch (loopid) 
    { 
        case 1: loop1chunk(nextChunk.start,nextChunk.end); break;
        case 2: loop2chunk(nextChunk.start,nextChunk.end); break;
    }
}

void StealChunks(int loopid)
{
    chunk stolenChunk;
    
    while(true)                             
    {
        GetStolenChunk(&stolenChunk);       // returns the stolen chunk.
        
        if(stolenChunk.start == -1)         // exit loop when there are no more 
        {                                   // no more iterations in each thread
            break;                          // local set.
        }
        
        RunChunk(loopid,stolenChunk);       // run the stolen chunk.
    }
    
}

void GetNextChunk(int myid,chunk* nextChunk)
{

    int nthreads = omp_get_num_threads(); 
    
    int start=-1;
    int chunkSize = -1;
    
    omp_set_lock(&(array[myid].lock));                     // locks local set
    
    start =  array[myid].lo ;                              
    
    chunkSize = (array[myid].hi-array[myid].lo)/nthreads;  // Every thread executes
                                                           // chunks of iterations 
                                                           // whose size is a fraction 1/p of the
                                                           // remaining iterations its local set
    
    if(chunkSize == 0) chunkSize=1;                         // if the remaining 
                                                            // iterations are less
                                                            // than the number of threads
                                                            // then the chunk size is 1
                                                            // until lo value reach value of hi.
    
    array[myid].lo = array[myid].lo + chunkSize;            // updates lo value 
                                                            // of the current local set.
    
    omp_unset_lock(&(array[myid].lock));                    // unlocks local set.
    
    
    
    nextChunk->start = start;                               // determines the start 
                                                            //of the next chunk
    
    nextChunk->end = start+chunkSize;                       //determines the end of the next chunk
    
    //printf("myid %d chunkSize  = %d lo = %d hi=%d \n ",myid,chunkSize,nextChunk->start,nextChunk->end);
    
    
}


void AllocateArray()
{    
    int nthreads = omp_get_num_threads(); 
    omp_set_lock(&allocationLock);                     //lock to prevent other threads
                                                       //go in this section of code
                                                       //while the allocation is in progress
    
    if(isArrayAlocated == false)                      // only the first thread 
                                                      // which acquires the lock
                                                      // allocate the array
    {
        
        array =(localSet*) malloc(nthreads * sizeof(localSet)); //array allocation
        //printf("Allocation an array of size %d\n",nthreads);
        for(int i=0; i < nthreads;i++)
        {
            omp_init_lock(&(array[i].lock));        // initialize locks of the array.
        }
        isArrayAlocated = true;
    }
    
    omp_unset_lock(&allocationLock);                    //unlock
    
    
}
void InitArray()
{    
  
    #pragma omp single                                      //  in  this section we
                                                            // run the initialization
                                                            // of the data structure
                                                            // only from one thread
    {
        int nthreads = omp_get_num_threads(); 
        int ipt = (int) ceil((double)N/(double)nthreads); 
        for(int i = 0; i < nthreads; i++)
        {
            int lo = i*ipt;
            int hi = (i+1)*ipt;
            if (hi > N) hi = N; 
            array[i].lo = lo;
            array[i].hi = hi;
        }
  
    }

}


