/* FIT3143 - Parallel Computing */
/* Lab 2 - Task 5 (Conditionals) */
/* Pair 4 */

/* Task 5 */
/* You should write your own C codes that demonstrate the use of the different types
of conditionals discussed in this section. */

#include <stdio.h>

int main(void)
{
    /* Assign and initialize integer for demonstrations*/
    int integer5a, integer5b;
    
    /* Demonstrating IF construct by using if statement to determine whether a number
    is greater than or less than 50 */
    printf("(5a) Demonstrating the use of the IF construct: \n");
    printf("-----\n");
    printf("Enter an integer: \n");
    scanf("%i", &integer5a);
    if (integer5a > 50){
        printf("This number is greater than 50.\n");
    }
    else{
        printf("This number is not greather than 50. \n");
    }

    printf("=====-----=====\n");

    /* Demonstrating SWITCH construct by using if statement to print the input number 
    as word as long as it is between 11 and 19 */
    printf("(5b) Demonstrating the use of the SWITCH construct: \n");
    printf("-----\n");
    printf("Enter an integer between (including) 11 and 19: \n");
    scanf("%i", &integer5b);    
    switch(integer5b){
        case 11:
            printf("eleven");
            break;
        case 12:
            printf("twelve");
            break;
        case 13:
            printf("thirteen");
            break;
        case 14:
            printf("fourteen");
            break;
        case 15:
            printf("fifteen");
            break;
        case 16:
            printf("sixteen");
            break;
        case 17:
            printf("seventeen");
            break;
        case 18:
            printf("eighteen");
            break;
        case 19:
            printf("nineteen");
            break;
        default:
            printf("Please follow instructions!");
            break;
    }
        
    return 0;        
}