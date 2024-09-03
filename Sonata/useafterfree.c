#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *allocation;
    allocation = (char *) (malloc(256));
    free(allocation);

    allocation[0] = 'B';
    
    return 0;
}
