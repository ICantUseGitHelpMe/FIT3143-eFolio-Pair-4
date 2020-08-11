#include <stdio.h>

#define COUNT_MAX 5
int main() {
    int count = 1;
    while (count % COUNT_MAX != 0) {
        printf("%d\n", count);  // print digits from 1 to 49
        count = count + 1;
    }

    for (int i = COUNT_MAX; i > 0; i--) {
        printf("%d\n", i);  // print digits from 50 to 1
    }
}
