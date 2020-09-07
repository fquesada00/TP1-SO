#include <stdio.h>
#include <stdlib.h>

void throwError(char * string)
{
    perror(string);
    exit(EXIT_FAILURE);
}