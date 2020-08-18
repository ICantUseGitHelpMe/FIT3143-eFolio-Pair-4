/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 27833725
 *                  asdf        12345
 * --------------------------------------------------
 * Lab 3 - Task 1a
 *      Write a serial C program to search for prime 
 * numbers which are less than an integer, n, which 
 * is provided by the user. The expected program output 
 * is a list of all prime numbers found, which is written 
 * into a single text file (e.g., primes.txt).
 *      For instance, if the user inputs n as 10 on the 
 * terminal, the prime numbers written to the text file 
 * are: 2, 3, 5, 7.
 *      Hint: It is known that for a given number, prime 
 * numbers would only exist for values less than or equal 
 * to the square root of the given number. As such, you can 
 * make use of the sqrt() of math.h to optimize your code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Function prototypes
void write_to_file(char *pFilename, int *pMatrix, int inRow, int inCol);
int is_prime(int input_integer);

// create main function
// declare variable
// start timer
// for each integer until n, test for prime property of each integer
// write to file for each prime
// close files
// close timer
void main(void){
	char input_value[100] = "";
    int n, n_integer_boolean, prime_boolean;
    struct timespec start, end, startComp, endComp; 
	double time_taken; 

    // Get current clock time.
	clock_gettime(CLOCK_MONOTONIC, &start);

    printf("Enter an integer: \n");
    scanf("%i", &n);

    prime_boolean = is_prime(n);
    if (prime_boolean == 0){
        printf("Integer is not prime");
    }
    else {
        printf("Integer is prime");
    }
    // Get the clock current time again
	// Subtract end from start to get the CPU time used.
	clock_gettime(CLOCK_MONOTONIC, &endComp); 
	time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9; 
    	time_taken = (time_taken + (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9; 
	printf("Cell product complete - Computational time only(s): %lf\n", time_taken); // portion of the computing time of ts


	// Write to file
	printf("Commence Writing\n");
	WriteToFile("VC.txt", pArrayNum3, row1, col1);
	printf("Write complete\n");
	free(pArrayNum1);
	free(pArrayNum2);
	free(pArrayNum3);
	
	// Get the clock current time again
	// Subtract end from start to get the CPU time used.
	clock_gettime(CLOCK_MONOTONIC, &end); 
	time_taken = (end.tv_sec - start.tv_sec) * 1e9; 
    	time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9; 
	printf("Overall time (Including read, product and write)(s): %lf\n", time_taken);	// ts
}

// Function definition
void write_to_file(char *pFilename, int *pMatrix, int inRow, int inCol)
{
	int i, j;
	FILE *pFile = fopen(pFilename, "w");
	fprintf(pFile, "%d\t%d\n", inRow, inCol);
	for(i = 0; i < inRow; i++){
	    for(j = 0; j < inCol; j++){
		fprintf(pFile, "%d\t", pMatrix[(i * inCol) + j]);
		}
		fprintf(pFile, "\n");
	}
	fclose(pFile);
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
    return(1);
}