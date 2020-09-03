/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 5 - Task 5b
 *      Implement a parallel version of this algorithm 
 * using Message Passing Interface (MPI). The root rank 
 * will prompt the user for the N value. The specified 
 * N value will then be disseminated to other processors 
 * to calculate the value of Pi based. Apply a data 
 * parallel design based on the value of N and the number
 * of MPI processes. Measure the overall time taken 
 * (i.e., tp) for large values of N (e.g., N = 100,000,000) 
 * and compute the actual speed up (if any).
 *      You may want to use these MPI routines in your 
 * solution:
 *      MPI_Bcast and MPI_Reduce
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
static long N = 100000000;
int main(int argc, char* argv[])
{
    int i;
    double sum = 0.0;
    double piVal;
    struct timespec start, end;
    double time_taken;
    // Get current clock time.
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(i = 0; i < N; i++){
        sum += 4.0 / (1 + pow((2.0 * i + 1.0)/(2.0 * N), 2));
    }
    piVal = sum / (double)N;
    // Get the clock current time again
    // Subtract end from start to get the CPU time used.
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    printf("Calculated Pi value (Serial-AlgoI) = %12.9f\n", piVal);
    printf("Overall time (s): %lf\n", time_taken); // ts
    return 0;
}