/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 3 - Task 1c
 *      Write a parallel version of your serial code 
 * in C utilizing POSIX Threads. Here, design and 
 * implement a parallel partitioning scheme which 
 * distributed the workload among the threads. Compare 
 * the performance of the serial program (ts) in part 
 * (a) with that of the parallel program (tp) in part 
 * (b) for a fixed number of processors or threads (e.g.
 * p = 4).
 *      Calculate the actual speed up, S(p). Analyze 
 * and compare the actual speed up with the theoretical 
 * speed up for an increasing number of threads and/or n. 
 * You can either tabulate your results or plot a chart 
 * when analyzing the performance of the serial and parallel 
 * programs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#define THREAD_COUNT 4

// Function prototypes
void file_append(int out);
void file_clear();
int is_prime(int input_integer);
void *ThreadFunc(void *pArg); // POSIX thread function format

// Declaration of global variables
int n, prime_boolean;
pthread_t thread_id[THREAD_COUNT]; 
int thread_number[THREAD_COUNT];

void main(void){	
    int n_integer_boolean;
    struct timeval start, middle, stop;
	double time_taken; 

    printf("Enter an integer: \n");
    scanf("%d", &n);
    gettimeofday(&start, NULL);

    // Cast numeric character into integer
    n = (int)n;

    file_clear();

    gettimeofday(&middle, NULL);

    // Fork		
	for (int thread_counter = 0; thread_counter < THREAD_COUNT; thread_counter++)
	{
        thread_number[thread_counter] = thread_counter;
		pthread_create(&thread_id[thread_counter], 0, ThreadFunc, &thread_number[thread_counter]);
	}
	// Join
	for(int thread_counter = 0; thread_counter < THREAD_COUNT; thread_counter++)
	{
        pthread_join(thread_id[thread_counter], NULL);
	}



    gettimeofday(&stop, NULL);
    int max, comp;
    max = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
    comp = (stop.tv_sec - middle.tv_sec) * 1000000 + stop.tv_usec - middle.tv_usec;
    printf("Time taken: %lu microseconds\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
    printf("Comp time taken: %lu microseconds\n", (stop.tv_sec - middle.tv_sec) * 1000000 + stop.tv_usec - middle.tv_usec);
    printf("Time in seconds: %f\n", max*1e-6);
}

//File output:
void file_append (int out){
	FILE *output;
	output = fopen("./task_1_output.txt", "a");

	if(output == NULL){
		printf("ERROR");
		exit(1);
	}

	fprintf(output, "%d\n", out);
	fclose(output);

}

//File clear:
void file_clear (){
	FILE *output;
	output = fopen("./task_1_output.txt", "w");

	if(output == NULL){
		printf("ERROR");
		exit(1);
	}

	fclose(output);

}

// Function for determining whether or not a number is prime
int is_prime(int input_integer){
    // Let input_integer be the integer to be tested for prime property
    int input_root;
    // Find square root of input_integer
    input_root = floor(sqrt(input_integer));
    // Iterate through each integer, i, from 2 to square root of input_integer
    for (int iterative_integer = 2; iterative_integer <= input_root; iterative_integer++) {
        // If the remainder of input_integer divded by i is 0, return 0
        int remainder = input_integer % iterative_integer;
        if (remainder == 0){
            return(0);
        }
    }
    // If the remainder of input_integer divided by all i values is not 0, return 1
    if (input_integer > 1){
        return(1);
    }
    else {
        return(0);
    }
}

// Thread function
void *ThreadFunc(void *pArg)
{
	int i, j;
	int my_rank = *((int*)pArg);
	printf("Thread %d\n", my_rank);
	
	int integers_per_thread = n / THREAD_COUNT; 
	int integers_per_thread_remainder = n % THREAD_COUNT; 
	
	int start_point = my_rank * integers_per_thread; 
	int end_point = start_point + integers_per_thread; 
	if(my_rank == THREAD_COUNT - 1)
		end_point += integers_per_thread_remainder;
	
    for (int iterative_integer = start_point; iterative_integer < end_point; iterative_integer++) {
        prime_boolean = is_prime(iterative_integer);
        if (prime_boolean == 1){
            //printf("%d \n", iterative_integer); // change to write
			file_append(iterative_integer);
        }
    }
	return NULL;
}

