/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 6 - Task 2d
 *      Implement a parallel version of your serial 
 * code in C using Message Passing Interface (MPI).
 * The root process will prompt the user for the n 
 * value (n = 10,000,000 (i.e., ts).).
 * The specified n value will then be disseminated to 
 * other MPI processes to calculate the prime numbers. 
 * Each process (including root process) computes the 
 * prime number (based on the equal or varied workload 
 * distribution per node) and writes the computed prime 
 * number into text files.
 *      The name of the text files should include the 
 * node rank value (e.g. process_0.txt, process_1.txt, 
 * process_2.txt, etc.). Execute the compiled program 
 * using at least four MP
 * 
 * CODE BELOW FROM WEEK 3 1C
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
#include <mpi.h>


#define THREAD_COUNT 4

// Function prototypes
void file_append(int out, int thread);
void file_clear(int thread);
int is_prime(int input_integer);
void *ThreadFunc(void *pArg); // POSIX thread function format

// Declaration of global variables
int n, prime_boolean;
pthread_t thread_id[THREAD_COUNT]; 
int thread_number[THREAD_COUNT];

int main(int argc, char **argv){

    int n_integer_boolean;
    struct timeval start, middle, stop;
	double time_taken; 


    int my_rank, size;
    gettimeofday(&middle, NULL);

    MPI_Init(&argc, &argv);     // Initialise MPI and get the required constants (size and current rank)
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


    if (my_rank == 0)
    {
        printf("Enter an integer: \n");
        fflush(stdout);
        scanf("%d", &n);
        gettimeofday(&start, NULL);

        // Cast numeric character into integer
        n = (int)n;

        for (int ranks = 0; ranks < size; ranks++){ // Create and empty all the files we output to

            file_clear(ranks);
        }
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Send the data and output it
	// Parallel implementation
    int chunk = n/size;     // Calculate the bounds for each process (e.g.0 - 25M, 25M - 50M, etc.)
    int min = chunk * my_rank;
    int max_chunk = min + chunk;
    for(int i = min; i < max_chunk; i++){ // Perform the operations
        if (is_prime(i)){
            file_append(i, my_rank);
        }
    }
    if (my_rank == 0)
    {
        gettimeofday(&stop, NULL);
        int max, comp;
        max = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
        comp = (stop.tv_sec - middle.tv_sec) * 1000000 + stop.tv_usec - middle.tv_usec;
        // printf("Time taken: %lu microseconds\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
        // printf("Comp time taken: %lu microseconds\n", (stop.tv_sec - middle.tv_sec) * 1000000 + stop.tv_usec - middle.tv_usec);
        printf("Time in seconds: %f\n", max*1e-6);
    
    }
	MPI_Finalize();     // Finish up with MPI



    return 0;
}

//File output:
void file_append (int out, int thread){
	FILE *output;

    char * outputFileBuffer;                                                            // Declare buffer for name of output file
    outputFileBuffer = malloc(sizeof(char) * 256);                                      // Allocate space for declared buffer
    strcpy(outputFileBuffer, "");                                                       // Ensure output buffer is empty
    snprintf(outputFileBuffer, sizeof(char) * 256, "task_2_output_%d.txt", thread);     // Store output file name into buffer and parameterise for thread
    output = fopen(outputFileBuffer, "a");                                              // Append to target file

	if(output == NULL){
		printf("ERROR");
		exit(1);
	}

	fprintf(output, "%d\n", out);
	fclose(output);

}

//File clear:
void file_clear (int thread){
	FILE *output;
    
    char * outputFileBuffer;                                                            // Declare buffer for name of output file
    outputFileBuffer = malloc(sizeof(char) * 256);                                      // Allocate space for declared buffer
    strcpy(outputFileBuffer, "");                                                       // Ensure output buffer is empty
    snprintf(outputFileBuffer, sizeof(char) * 256, "task_2_output_%d.txt", thread);     // Store output file name into buffer and parameterise for thread
    output = fopen(outputFileBuffer, "w");                                              // Append to target file

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

