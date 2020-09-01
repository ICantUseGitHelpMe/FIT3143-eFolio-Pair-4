/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 4 - Task 1
 *      The aim of this task is to develop a gaming 
 * machine using OpenMP workshare programming model. 
 * The machine has a 10-digit display. The following 
 * rule applies to register a win with the machine.
 * At least two of the displayed digits must represent 
 * the same value to register a win. The machine may 
 * generate more than one win after a play. It will 
 * depend on the number of (different) repeated digits 
 * appearing on the display.
 *      The machine is to be controlled by a parallel 
 * software code. The code specifications are as follows.
 *      • The code forks four concurrent threads. With 
 * chunksize = 2.
 *      • Each thread executes an independent pseudorandom 
 * number generator function that produces random numbers 
 * in the range of 1 to 25.
 *      • The threads work on a shared integer array of 
 * size 10 to fill each of the array elements with a random 
 * number.
 *      • The threads join to form the master thread.
 *      • The array elements are checked for matching 
 * entries. A win is counted if two or more occurrences 
 * of a number are found in the array. The total number of 
 * such occurrences is displayed on the terminal screen. 
 * Discuss your solution in relation to parallel code to 
 * count the number of wins.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#define MAX_NUM 25
#define DIGITS 10
int main(){
// Declare Envienvironment variables
    int numbers[DIGITS];                                                // The number array generated
    int occurrences[MAX_NUM];                                           // Initialise the array storing the frequency of each number present
    
    for (int i = 0; i < MAX_NUM; i++){                                  
        occurrences[i] = 0;
    }

    srand((unsigned) time(NULL));                                       // Generate seed for random number generations
    
    #pragma omp parallel for schedule(static, 2)                        // Forking
    for (int i = 0; i < DIGITS; i++){                                   // For each element in the defined number array
        numbers[i] = (rand() % MAX_NUM) + 1;                            // Generate random values between 1 and 10
        printf("Current Thread: %d, Current index: %d, Number: %d\n", omp_get_thread_num(), i, numbers[i]);
    }                                                                   // Implicit Joining
    
    printf("----------------\n");
                                                              
    for (int i = 0; i < DIGITS; i++){
        occurrences[numbers[i] - 1] = occurrences[numbers[i] - 1] + 1;  // Increment the number of occurrences for the number, where index = the number.  
                                                                        // Subtract 1 as the arrays are zero-indexed
        printf("%d ", numbers[i]);                                      // Print the 10-digit display

    }

    int wins = 0;                                                       // Define variable for number of wins
    for (int i = 0; i < MAX_NUM; i++){                                   // For counter i from 0 to 10
        if (occurrences[i] > 1){                                        // If occurrences is greater than 1
            wins++;                                                     // Increment number of wins
        }
    }

    printf("\nThere are %d wins!\n", wins);                             // Print the number of wins

    return 0;

}