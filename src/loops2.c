#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define N 729
//#define reps 1000
#define reps 1000
#include <omp.h> 
#include <assert.h>



double a[N][N], b[N][N], c[N];
int jmax[N];  


void init1(void);
void init2(void);
void runloop(int); 
void loop1chunk(int, int);
void loop2chunk(int, int);
void valid1(void);
void valid2(void);

typedef struct localSet
{
    int lo;
    int hi;
    omp_lock_t lock;
}localSet;

localSet* array;
bool isArrayAlocated;

typedef struct chunk
{
    int start;
    int end;
}chunk;

omp_lock_t allocationLock;
//chunk nextChunk;

void GetNextChunk(int threadId,chunk* nextChunk);
void InitArray();
void StealChunks(int loopid);
void AllocateArray();
void RunChunk(int loopid, chunk nextChunk);
void GetStolenChunk(chunk* stolenChunk);



int main(int argc, char *argv[]) { 

    double start1,start2,end1,end2;
    int r;

    
    init1(); 

    isArrayAlocated = false;
    omp_init_lock(&allocationLock);

    start1 = omp_get_wtime(); 

    for (r=0; r<reps; r++){ 
            runloop(1);
    } 

    end1  = omp_get_wtime();  

    valid1(); 

    printf("Total time for %d reps of loop 1 = %f\n",reps, (float)(end1-start1)); 


    init2(); 

    start2 = omp_get_wtime(); 

    for (r=0; r<reps; r++){ 
            runloop(2);
    } 

    end2  = omp_get_wtime(); 

    valid2(); 

    printf("Total time for %d reps of loop 2 = %f\n",reps, (float)(end2-start2)); 
    
} 

void init1(void){
    int i,j; 

    for (i=0; i<N; i++){ 
            for (j=0; j<N; j++){ 
                    a[i][j] = 0.0; 
                    b[i][j] = 3.142*(i+j); 
            }
    }

}

void init2(void){ 
    int i,j, expr; 

    for (i=0; i<N; i++){ 
            expr =  i%( 3*(i/30) + 1); 
            if ( expr == 0) { 
                    jmax[i] = N;
            }
            else {
                    jmax[i] = 1; 
            }
            c[i] = 0.0;
    }

    for (i=0; i<N; i++){ 
            for (j=0; j<N; j++){ 
                    b[i][j] = (double) (i*j+1) / (double) (N*N); 
            }
    }

} 


void runloop(int loopid)  {

#pragma omp parallel default(none) shared(loopid,isArrayAlocated,array) 
    {
        int myid  = omp_get_thread_num();
        int nthreads = omp_get_num_threads(); 
        int ipt = (int) ceil((double)N/(double)nthreads); 
        int lo = myid*ipt;
        int hi = (myid+1)*ipt;
        if (hi > N) hi = N; 
        
        AllocateArray();
        InitArray();
        
        chunk nextChunk;
        
        
        while(true )
        {
            GetNextChunk(myid,&nextChunk);
            if(nextChunk.start == hi) break;
            
            switch (loopid) 
            { 
                case 1: loop1chunk(nextChunk.start,nextChunk.end); break;
                case 2: loop2chunk(nextChunk.start,nextChunk.end); break;
            }
        }
        
        StealChunks(loopid);
    }
    
    

}

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
        //printf("thread %d acquires lock of thread %d\n",omp_get_thread_num(),i);
        remainingIterations = array[i].hi-array[i].lo;
        
        
        if(remainingIterations > maxRemainingIterations)
        {
            if(targetThread != -1)
            {
                omp_unset_lock(&(array[targetThread].lock));
                //printf("thread %d release lock of thread %d\n",omp_get_thread_num(),targetThread);
            }
            maxRemainingIterations=remainingIterations;
            targetThread =i;
        }
        else
        {
            omp_unset_lock(&(array[i].lock));
            //printf("thread %d release lock of thread %d\n",omp_get_thread_num(),i);
        }
    }
    if(maxRemainingIterations  > 0)
    {
        //printf("thread %d  steal chunk from thread %d which has %d \n",omp_get_thread_num(),targetThread,maxRemainingIterations);
        
        int chunkSize = (array[targetThread].hi-array[targetThread].lo)/nthreads;
        if(chunkSize == 0) chunkSize=1;
        stolenChunk->start = array[targetThread].lo;
        stolenChunk->end = stolenChunk->start +chunkSize;
        array[targetThread].lo = stolenChunk->end ;
        
        omp_unset_lock(&(array[targetThread].lock));
        //printf("thread %d release lock of thread %d\n",omp_get_thread_num(),targetThread);
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
        array =(localSet*) malloc(nthreads * sizeof(localSet));
        isArrayAlocated = true;
        //printf("Allocation step\n");
    }
    omp_unset_lock(&allocationLock);
    
    
}
void InitArray()
{
    int myid  = omp_get_thread_num();
    int nthreads = omp_get_num_threads(); 
    int ipt = (int) ceil((double)N/(double)nthreads); 
    int lo = myid*ipt;
    int hi = (myid+1)*ipt;
    if (hi > N) hi = N; 
    omp_init_lock(&(array[myid].lock));
    
    array[myid].lo = lo;
    array[myid].hi = hi;
    
}



void loop1chunk(int lo, int hi) { 
    int i,j; 

    for (i=lo; i<hi; i++){ 
            for (j=N-1; j>i; j--){
                    a[i][j] += cos(b[i][j]);
            }

    }

} 



void loop2chunk(int lo, int hi) {
    int i,j,k; 
    double rN2; 

    rN2 = 1.0 / (double) (N*N);  

    for (i=lo; i<hi; i++){ 
            for (j=0; j < jmax[i]; j++){
                    for (k=0; k<j; k++){ 
                            c[i] += (k+1) * log (b[i][j]) * rN2;
                    } 
            }

    }

}

void valid1(void) { 
    int i,j; 
    double suma; 

    suma= 0.0; 
    for (i=0; i<N; i++){ 
            for (j=0; j<N; j++){ 
                    suma += a[i][j];
            }
    }
    printf("Loop 1 check: Sum of a is %lf\n", suma);

} 


void valid2(void) { 
    int i; 
    double sumc; 

    sumc= 0.0; 
    for (i=0; i<N; i++){ 
            sumc += c[i];
    }
    printf("Loop 2 check: Sum of c is %f\n", sumc);
} 


