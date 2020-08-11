/* FIT3143 - Parallel Computing */
/* Lab 2 - Task 7 (I/O) */
/* Pair 4 */

/* Task 7 */
/* Can you think of a way to count the number of words in a text file? */
#include <stdio.h>

int main()
{
    int c, nw = 0, nl = 0;
    while ( (c = getchar()) != EOF )
    {
        if (c == ' ') nc++; // Assume that the file includes a ' ' character between each word
        if (c == '\n') nl++;
    }
    printf("Number of words = %d, number of lines = %d\n", nw, nl);
    return(0);
}