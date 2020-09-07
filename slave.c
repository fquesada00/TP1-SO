#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>

void throwError(char *string);
void processCNF(char *nameOfFile, char result[]);

int main(int argc, char * argv[])
{
    //Procesamos los primeros N archivos 
    for(int i = 1 ; i < argc ; i++)
    {
        char line[512] = {0};
        processCNF(argv[i],line);
        if(!write(STDOUT_FILENO, line, strlen(line)))
            throwError("Could not write");
    }

    //Procesamos los archivos que nos van mandando desde master
    while (1)
    {
        char *line = NULL;
        size_t size;
        ssize_t length = getline(&line, &size, stdin);
        line[length - 1] = 0;
        char result[512] = {0};
        processCNF(line, result);
        free(line);
        if(!write(STDOUT_FILENO, result, strlen(result)))
            throwError("Could not write");
    }
    return 0;
}

//Funcion que se comunica con el minisat y carga en result el resultado, formateado como pide el enunciado.
void processCNF(char *nameOfFile, char result[]) 
{
    char command[255] = {0};
    char buff[255] = {0};

    sprintf(command, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e  \".*SATISFIABLE\"", nameOfFile);
    FILE *file = NULL;
    file = popen(command, "r");
    if(file == NULL)
    {
        throwError("Could not Open file");
    }
    fread((void *)buff, 1, 255, file); 
    strcat(buff,"\n");

    sprintf(result, "Name of File: %s\nProcessed by child with PID: %d\n",nameOfFile, getpid()); 
    strcat(result, buff);
    
    pclose(file);
    return;
}

void throwError(char *string)
{
    perror(string);
    exit(EXIT_FAILURE);
}
