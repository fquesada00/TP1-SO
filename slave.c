#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(void)
{ 
    char command[255] = {0};
    char *line = NULL;
    sem_t * sent = sem_open("sent",O_CREAT,S_IRWXU,0);
    sem_wait(sent); 
    ssize_t length = getline(&line, NULL, stdin);
    line[length]=0;
    sprintf(command, "minisat %s  | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e  \".*SATISFIABLE\"", line);
    FILE *file = NULL;

    file = popen(command, "r");
    char buf[255] = {0};
    fread((void *)buf, 1, 255, file);
    puts(buf);

    if (file != NULL)
        pclose(file);
    return 0;
}
//padre manda n --> enviados = n
//hijo termina --> hace sem post, y hace sem wait de un semaforo