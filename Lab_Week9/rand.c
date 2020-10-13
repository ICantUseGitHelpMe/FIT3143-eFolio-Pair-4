#include <stdio.h>

int main()
{
    int n, lastDigit;

    /* Input number from user */
    printf("Enter any number: ");
    scanf("%d", &n);

    /* Get the last digit */
    lastDigit = n % 100;

    printf("Last digit = %d", lastDigit);

    return 0;
}