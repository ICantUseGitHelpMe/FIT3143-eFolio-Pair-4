/* FIT2100 - Operating Systems */
/* Week 1 - Tutorial */
/* Philip Chen 27833275 */
/* Date: 24-07-2018 */

/* Task 5 */
/* Write a C program that generates a random sequence of six numbers by using the rand
function from the stdlib library. Your program should generate a different sequence each time
the program is run. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(void)
{
    /* Declaration of block variables */
    int randomNumberArray[6];
    
    /* Seed generator */
    srand((unsigned) time(NULL));
    
    /* Generate numbers */
    randomNumberArray[0] = truncl(rand());
    randomNumberArray[1] = rand();
    randomNumberArray[2] = rand();
    randomNumberArray[3] = rand();
    randomNumberArray[4] = rand();
    randomNumberArray[5] = rand();
    
    /* Print initial */
    printf("%i %i %i %i %i %i", randomNumberArray[0], randomNumberArray[1], randomNumberArray[2], 
           randomNumberArray[3], randomNumberArray[4], randomNumberArray[5]);
    printf("%d", __INT_MAX__);
    return 0;        
}