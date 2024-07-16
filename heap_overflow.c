#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int length = 256;
    char *allocation;
    allocation = (char*)malloc(length);
    allocation[length] = 'J';


    printf("%c \n", allocation[length]);
    return 0;
}
