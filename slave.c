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
    /*int d;
    sem_getvalue(sent,&d);
    d+='0';
    char c[2]={0};
    c[0]=d;
    write(STDOUT_FILENO,c,2);*/
    sem_wait(sent); 
    //write(STDOUT_FILENO,"Hola\n",5);
    size_t size;
    ssize_t length = getline(&line,&size , stdin);
    line[length-1]=0;
    sprintf(command, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e  \".*SATISFIABLE\"", line);
    FILE *file = NULL;
    printf("run %s\n",command);
    file = popen(command, "r");
    char buf[255] = {0};
    fread((void *)buf, 1, 255, file);
    puts(buf);

    if (file != NULL)
        pclose(file);
    sem_close(sent);
    sem_unlink("sent");
    return 0;
}
//padre manda n --> enviados = n
//hijo termina --> hace sem post, y hace sem wait de un semaforo