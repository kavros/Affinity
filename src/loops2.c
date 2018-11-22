#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h> 
#include <assert.h>
#include "../include/affinity.h"
#include "../include/loops2.h"


int main(int argc, char *argv[]) { 
    
    double start1,start2,end1,end2;
    int r;
    
    
    init1(); 
    
    isArrayAlocated = false;
    omp_init_lock(&allocationLock);
    isArrayInitialized =false;
    omp_init_lock(&initializationLock);
    
    start1 = omp_get_wtime(); 
    
    for (r=0; r<reps; r++){ 
        runloop(1);
        isArrayInitialized =false;                  // we need to initialize the 
                                                    // array with the local sets
                                                    // in every new repetition
    } 
    
    end1  = omp_get_wtime();  
    
    valid1(); 
    
    printf("Total time for %d reps of loop 1 = %f\n",reps, (float)(end1-start1)); 
    
    
    init2(); 
    
    start2 = omp_get_wtime(); 
    
    for (r=0; r<reps; r++){ 
        isArrayInitialized =false;                  // we need to initialize the 
                                                    // array with the local sets
                                                    // in every new repetition
        runloop(2);
    } 
    
    end2  = omp_get_wtime(); 
    
    valid2(); 
    
    printf("Total time for %d reps of loop 2 = %f\n",reps, (float)(end2-start2)); 
    free(array);
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


void runloop(int loopid)  
{
    
#pragma omp parallel default(none) shared(loopid,isArrayAlocated,isArrayInitialized,array) 
    {
        int myid  = omp_get_thread_num();
        int nthreads = omp_get_num_threads(); 
        int ipt = (int) ceil((double)N/(double)nthreads); 
        int hi = (myid+1)*ipt;
        if (hi > N) hi = N; 
        
        AllocateArray();                       // The first thread which calls 
                                               // this function, it will allocate
                                               // the array with the local sets.
        
        InitArray();                           // The first thread which calls 
                                               // this function, it will initialize
                                               // the array with the local sets.
        
        chunk nextChunk;
        while(true )
        {
            GetNextChunk(myid,&nextChunk);
            if(nextChunk.start == hi) break;    // exit loop if there are 
                                                // no more iterations 
                                                // left in its local set 
            switch (loopid) 
            { 
                case 1: loop1chunk(nextChunk.start,nextChunk.end); break;
                case 2: loop2chunk(nextChunk.start,nextChunk.end); break;
            }
        }
        
        StealChunks(loopid);                    // threads which has finished with
                                                // their own local set tries to steal
                                                // chunks until there are no more
                                                // iterations.
    }
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


