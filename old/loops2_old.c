#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define N 729
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

typedef struct task
{
	int lo;
	int hi;
	int threadId;
}task;
task* loop1Tasks = NULL;
task* loop2Tasks = NULL;
omp_lock_t lock;

bool isLoop1TasksInitialized = false;
bool isLoop2TasksInitialized = false;
omp_lock_t lock2;


int main(int argc, char *argv[]) { 

	double start1,start2,end1,end2;
	int r;

	init1(); 

	//init locks
	omp_init_lock(&lock2);
	omp_init_lock(&lock);
	
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

#pragma omp parallel default(none) shared(loopid,loop1Tasks,loop2Tasks) 
	{
		int myid  = omp_get_thread_num();
		int nthreads = omp_get_num_threads(); 
		int ipt = (int) ceil((double)N/(double)nthreads); 
		int lo = myid*ipt;
		int hi = (myid+1)*ipt;
		if (hi > N) hi = N; 
		

		//AllocateTasksArray(loopid);
		//InitTasksArray(loopid,hi,lo);
		switch (loopid) { 
			case 1: loop1chunk(lo,hi); break;
			case 2: loop2chunk(lo,hi); break;
		} 

		//StealTask(loopid,nthreads);
	}
}

task* GetTasksArray(int loopid)
{
	task* tasks;
	if(loopid == 1)
	{
		tasks = loop1Tasks; 
	}
	else if(loopid == 2)
	{
		tasks = loop2Tasks;
	}
	else
	{
		assert(0);
	}
	return tasks;
}

void InitTasksArray(int loopid,int hi,int lo)
{
	task* tasks = GetTasksArray(loopid);
	int myid  = omp_get_thread_num();
	tasks[myid].threadId = myid;

	tasks[myid].lo = lo;
	tasks[myid].hi = hi;
	//printf("thread  %d have %d iterations\n",myid,hi-lo);

}

void AllocateTasksArray(int loopid)
{
	int nthreads = omp_get_num_threads();
	int myid  = omp_get_thread_num();
	int ipt = (int) ceil((double)N/(double)nthreads); 
	int lo = myid*ipt;
	int hi = (myid+1)*ipt;
	if (hi > N) hi = N; 
	
	omp_set_lock(&lock2);	

	if(loopid == 1 && isLoop1TasksInitialized== false)
	{
		if(loop1Tasks == NULL)
		{
			//printf("Initialization step for loop 1 tasks\n");
			loop1Tasks = (task*) malloc(sizeof(task)*nthreads);
		}
		isLoop1TasksInitialized = true;		
	}
	else if(loopid==2 && isLoop2TasksInitialized == false)
	{
		if(loop2Tasks == NULL)
		{
			//printf("Initialization step for loop 2 tasks\n");
			loop2Tasks = (task*) malloc(sizeof(task)*nthreads);
		}
		isLoop2TasksInitialized = true;
	}

	omp_unset_lock(&lock2);
}

void DeallocateTasksArray()
{

}


bool GetNextIteration( int loopid)
{
	int myid = omp_get_thread_num();
	bool IsNextIterationMine = false;
	task* tasks = GetTasksArray(loopid);
	int nthreads = omp_get_num_threads();


	omp_set_lock(&lock);
	for(int i=0; i < nthreads; i++)
	{
		if(tasks[i].threadId == myid )
		{
			tasks[i].lo =tasks[i].lo+1;
			IsNextIterationMine = true;
		}
	}
	/*if(tasks[myid].threadId == myid)
	{
		tasks[myid].lo = tasks[myid].lo+1;
		IsNextIterationMine = true;
	}*/
	omp_unset_lock(&lock);
	return IsNextIterationMine;

}


void StealTask(int loopid,int totalThreads )
{
	task* tasks = GetTasksArray(loopid);
	int myid  = omp_get_thread_num();
	int maxRemainigIterations = -1;
	int targetThread =-1;
	int currRemainingIterations = 0;

	
	//printf("\n thread %d  try to steal iterations from ",myid);
	
	for(int i =0; i < totalThreads; i++)
	{	
		currRemainingIterations = tasks[i].hi -tasks[i].lo;

		if( currRemainingIterations > maxRemainigIterations)
		{
			maxRemainigIterations =currRemainingIterations;
			targetThread = tasks[i].threadId;
		}

	}

	if(maxRemainigIterations > 0 )
	{
		//printf(" thread %d which have %d remaining iterations",targetThread ,maxRemainigIterations);
		
		omp_set_lock(&lock);
		tasks[targetThread].threadId = myid; // here we steal task(iterations) from target thread.
		omp_unset_lock(&lock);	

		if(loopid == 1)
		{
			//printf(" \n start thread %d from %d to %d",myid ,tasks[targetThread].lo,tasks[targetThread].hi);
			loop1chunk(tasks[targetThread].lo,tasks[targetThread].hi);
		}
		else if(loopid == 2)
		{
			loop2chunk(tasks[targetThread].lo,tasks[targetThread].hi);
		}
		else
		{
			assert(0);
		}


	}else
	{
		printf("\n");
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


