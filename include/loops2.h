//B145772

#ifndef LOOPS2_H
#define LOOPS2_H

#define N 729
#define reps 1000

double a[N][N], b[N][N], c[N];
int jmax[N];  


void init1(void);
void init2(void);
void runloop(int); 
void loop1chunk(int, int);
void loop2chunk(int, int);
void valid1(void);
void valid2(void);

#endif /* LOOPS2_H */

