#include <stdlib.h>
#include <stdio.h>

int main() {
    int *x = malloc(4*sizeof(int));
    printf("x pointer: %p\n",x);
    *x = 1;
    printf("value at x: %d\n", *x);

    // pointer arithmetic outside of x's range
    int *y = x + 128;
    // c will have no issues with this
    // in CHERI, we can print the pointer address, but attempting to read or write the value there will trap

    printf("y pointer: %p\n",y); // works
    *y = 2; // won't work in cheri, works in c
    printf("value at y: %d\n", *y); // won't work in cheri, works in c
  
    return 0;
}
