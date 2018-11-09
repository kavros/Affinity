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
    
    //printf("thread %d  try to steal chunk from ",omp_get_thread_num());
    
    //omp_set_lock(&lock);
    for(int i=0; i < nthreads; i++ )
    {
        omp_set_lock(&(array[i].lock));
//        printf("thread %d acquires lock of thread %d\n",omp_get_thread_num(),i);
        remainingIterations = array[i].hi-array[i].lo;
        
        
        if(remainingIterations > maxRemainingIterations)
        {
            if(targetThread != -1)
            {
                omp_unset_lock(&(array[targetThread].lock));
//                printf("thread %d release lock of thread %d\n",omp_get_thread_num(),targetThread);
            }
            maxRemainingIterations=remainingIterations;
            targetThread =i;
        }
        else
        {
            omp_unset_lock(&(array[i].lock));
//            printf("thread %d release lock of thread %d\n",omp_get_thread_num(),i);
        }
    }
    if(maxRemainingIterations  > 0)
    {
 //       printf("thread %d  steal chunk from thread %d which has %d \n",omp_get_thread_num(),targetThread,maxRemainingIterations);
        
        int chunkSize = (array[targetThread].hi-array[targetThread].lo)/nthreads;
        if(chunkSize == 0) chunkSize=1;
        stolenChunk->start = array[targetThread].lo;
        stolenChunk->end = stolenChunk->start +chunkSize;
        array[targetThread].lo = stolenChunk->end ;
        
        omp_unset_lock(&(array[targetThread].lock));
//        printf("thread %d release lock of thread %d\n",omp_get_thread_num(),targetThread);
    }
    else if(maxRemainingIterations == 0)
    {
        omp_unset_lock(&(array[targetThread].lock));
    }
    //omp_unset_lock(&lock);
            
    
}

void RunChunk(int loopid, chunk nextChunk)
{
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
        GetStolenChunk(&stolenChunk);
        if(stolenChunk.start == -1)
        {
            break;
        
        }
        RunChunk(loopid,stolenChunk);
    }
    
}

void GetNextChunk(int myid,chunk* nextChunk)
{

    int nthreads = omp_get_num_threads(); 
    
    int start=-1;
    int chunkSize = -1;
    
    //omp_set_lock(&lock);
    omp_set_lock(&(array[myid].lock));
    start =  array[myid].lo ;
    chunkSize = (array[myid].hi-array[myid].lo)/nthreads;
    if(chunkSize == 0) chunkSize=1;
    array[myid].lo = array[myid].lo + chunkSize;        
    omp_unset_lock(&(array[myid].lock));
    //omp_unset_lock(&lock);
    
    
    
    nextChunk->start = start;
    nextChunk->end = start+chunkSize;
    //printf("myid %d chunkSize  = %d lo = %d hi=%d \n ",myid,chunkSize,nextChunk->start,nextChunk->end);
    
    
}


void AllocateArray()
{
    int nthreads = omp_get_num_threads(); 
    omp_set_lock(&allocationLock);
    if(isArrayAlocated == false)
    {
        isArrayAlocated = true;
        array =(localSet*) malloc(nthreads * sizeof(localSet));
        //printf("Allocation an array of size %d\n",nthreads);
        for(int i=0; i < nthreads;i++)
        {
            omp_init_lock(&(array[i].lock));
        }
    }
    
    omp_unset_lock(&allocationLock);
    
    
}
void InitArray()
{
    
    int myid  = omp_get_thread_num();
    int nthreads = omp_get_num_threads(); 
    int ipt = (int) ceil((double)N/(double)nthreads); 
    
    omp_set_lock(&initializationLock);
    
    if(isArrayInitialized  == false)
    {
        isArrayInitialized = true;
        for(int i = 0; i < nthreads; i++)
        {
            int lo = i*ipt;
            int hi = (i+1)*ipt;
            if (hi > N) hi = N; 
            //printf("myid %d lo %d hi %d\n",myid,lo,hi);
            array[i].lo = lo;
            array[i].hi = hi;
        }
    }
    omp_unset_lock(&initializationLock);
    
    /*  
 * 
 *   
    omp_init_lock(&(array[myid].lock));
    
    array[myid].lo = lo;
    array[myid].hi = hi;
    printf("thread %d init\n",myid);
 */ 
}


