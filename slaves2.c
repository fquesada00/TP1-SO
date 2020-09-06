#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

void processCNF(char *nameOfFile, char buff[]);
int main(void)
{
    int done = 0;
    sem_t *sent = sem_open("sent", O_CREAT, S_IRWXU, 0);
    while (1)
    {
        char *line = NULL;
        size_t size;
        ssize_t length = getline(&line, &size, stdin);
        line[length - 1] = 0; //No sabemos si termina en cero o no
        sem_wait(sent);
        char buff[1024] = {0};
        processCNF(line, buff);
        write(STDOUT_FILENO, buff, strlen(buff));
    }
    sem_close(sent);
    sem_unlink("sent");
    return 0;
}

void processCNF(char *nameOfFile, char buff[]) //Recibe el path del archivo y devuelve un string
{
    char command[255] = {0};

    sprintf(command, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e  \".*SATISFIABLE\"", nameOfFile);
    FILE *file = NULL;
    sprintf(buff, "run %s\n", command);
    file = popen(command, "r");
    char buf[255] = {0};
    fread((void *)buf, 1, 255, file);
    char buf2[255] = {0};
    sprintf(buf2, "- pid %d - ", getpid()); //Me muestra que proceso lo corrio
    strcat(buff, buf2);
    strcat(buff, buf);
    if (file != NULL)
        pclose(file);
    return; 
}
