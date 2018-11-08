#include "../include/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//AllocateLocalSets(4);
//InitLocalSets(4,64);
/*chunk* next = GetNextChunk(0);

while(next != NULL)
{
    printf("%d, %d\n",next->hi,next->lo);
    next = GetNextChunk(0);

}*/
/*chunk* chunk = StealChunk();
for(int i =0; i < 7; i ++)
{
    printf("%d %d \n",chunk->lo,chunk->hi);
    chunk = StealChunk();
}*/


//printList(4);
//init locks




void AllocateLocalSets(int nThreads)
{
    printf("Initialization\n");
    
    //initialize lock
    omp_init_lock(&lock);
    totalPositions = nThreads;
    
    
    //allocates array with local sets
    array =(localSet*) malloc(sizeof(localSet) *nThreads);
    
    if(array == NULL)
    {
        printf("Failed to allocate space for queue.\n");
        exit(-1);
    }
    //set all queues empty
    for(int i=0; i < nThreads; i++)
    {
        array[i].head = NULL;

    }
}
void InitLocalSets(int nThreads,int N)
{
    //int totalIter=0;
    for(int i=0; i < nThreads; i++)
    {
        int ipt = (int) ceil((double)N/(double)nThreads); 
        int lo = i*ipt;
        int hi = (i+1)*ipt;
        if (hi > N) hi = N; 
        array[i].remainingChunks = hi -lo;
        
        //printf("thread num %d \n",i);
        int l = hi-lo;          
        int c = l/nThreads;     
        int r = l ;             
        int start =lo;
        int end   =lo+c;
        //totalIter=totalIter+(end-start);
        while( 1 )
        {
            //printf("c =%d\n",c);
            //printf("lo=%d , hi = %d \n",start,end );
            AddChunk(i,start,end);
            r = r-c;            //16-4 ,12-4
            if(r < nThreads)
            {
                for(int j =0 ; j < r; j++)
                {
                    c =1;
                    start =start+1;
                    end=end+c;
                    
                    AddChunk(i,start,end);
                    //printf("lo=%d , hi = %d \n",start,end );
                    //totalIter=totalIter+(end-start);
                }
                break;
            }
            c = r/nThreads;
            
            start=end;
            end=end+c;
            //totalIter=totalIter+(end-start);
        }
        //printf("%d\n",totalIter);
       
    }
    
}
void AddChunk(int pos,int lo,int hi)
{
    localSet* queue = &array[pos];
    //printf("lo = %d,hi=%d\n",lo,hi);
    if(queue->head == NULL)
    {
        chunk* newChunk = (chunk*) malloc(sizeof(chunk));
        newChunk->next = NULL;
        newChunk->hi = hi;
        newChunk->lo = lo;
        
        queue->head = newChunk;

        queue->head->next = NULL;
    }
    else
    {
        chunk* newChunk = (chunk*) malloc(sizeof(chunk));
        newChunk->next = NULL;
        newChunk->hi = hi;
        newChunk->lo = lo;
        
        chunk* curr = queue->head->next;
        chunk* prev = queue->head;
        while(curr)
        {
            prev = curr;
            curr = curr->next;
        }
        prev->next = newChunk;

    }
}

chunk* GetNextChunk(int threadId)
{
    localSet* queue = &(array[threadId]);
    chunk* next=NULL;
    
    if(queue->head != NULL)
    {
        next = queue->head;
        queue->head = queue->head->next;
        queue->remainingChunks = queue->remainingChunks -1;
    }
    
    return next;
}

chunk* StealChunk()
{
    int maxRemainingChunks = -1;
    int targetThread = -1;
    chunk* chunk = NULL;
    for(int i = 0; i < totalPositions; i++)
    {
        localSet* queue = &array[i];
        if(queue->remainingChunks > maxRemainingChunks )
        {
            maxRemainingChunks = queue->remainingChunks;
            targetThread = i;
        }
    }
    chunk = GetNextChunk(targetThread);
    return chunk;
}

void printList()
{
    for(int i=0; i < totalPositions; i++)
    {   
        
        localSet* queue = &array[i];
        printf("thread i %d, r = %d\n",i,queue->remainingChunks);
        chunk* curr = queue->head;
        while(curr)
        {
            printf("lo = %d, hi = %d\n",curr->lo,curr->hi);
            curr = curr->next;
        }
     
    }
}

