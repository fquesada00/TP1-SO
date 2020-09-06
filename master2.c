//https://linux.die.net/man/3/select
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#define SLAVE_COUNT 5


void killSlaves(pid_t slave_pid[SLAVE_COUNT]);
int main(int argc, char *argv[])
{
    pid_t pid;
    pid_t slave_pid[SLAVE_COUNT];
    sem_t *sem = sem_open("sent", O_CREAT, S_IRWXU, 0);
    int files_to_process = argc - 1, files_processed = 0, argv_idx = 1, max_fd = 0;
    int sender[2];                  //Pipes del Master
    int slave_pipe[SLAVE_COUNT][2]; //File descriptor de los slaves
    fd_set fd_read_pipes;
    if (pipe(sender) < 0)
    { //Validamos errores
        perror("Creating master pipe\n");
        exit(EXIT_FAILURE);
    }
    //Validar que pase minimo un archivo

    for (int i = 0; i < SLAVE_COUNT; ++i)
    { //Mandamos todo el output del main a todos los hijos
        if (pipe(slave_pipe[i]) < 0)
        { //Valido error
            perror("Creating pipe in slave\n");
            exit(EXIT_FAILURE);
        }
        if (slave_pipe[i][0] > max_fd) 
            max_fd = slave_pipe[i][0];
        if ((slave_pid[i] = fork()) < 0) //Si crashea, es aca.
        { //Valido error
            perror("Forking slave\n");
            exit(EXIT_FAILURE);
        }
        else if (slave_pid[i] == 0)
        {
            close(STDOUT_FILENO);
            dup2(sender[STDIN_FILENO], STDIN_FILENO);          //En el hijo tengo que pipear del output del master al input de todos los hijos, para que lo levante uno.
            dup2(slave_pipe[i][STDOUT_FILENO], STDOUT_FILENO); //Lo que me devuelve el hijo lo mando a un pipe de ESE esclavo.
            close(sender[STDIN_FILENO]);
            close(slave_pipe[i][STDOUT_FILENO]);
            execl("./slave", "slave", NULL); //¿Aca no le tendriamos que mandar los primeros N files?
            perror("Could not execute slave\n");
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0 ; i < SLAVE_COUNT ; i++){
        printf("%d -", slave_pid[i]);
    }
    printf("\n");
    FD_ZERO(&fd_read_pipes);
    while (files_processed < files_to_process)
    {
        char argv_buffer[255] = {0};
        strcpy(argv_buffer, argv[argv_idx++]);
        strcat(argv_buffer, "\n");
        write(sender[1], argv_buffer, strlen(argv_buffer));
        files_processed++;
        printf("%s\n", argv_buffer);
        sem_post(sem); //Le avisamos a algun hijo que nos saque del pipe el archivo que mandamos.
        FD_ZERO(&fd_read_pipes); //preguntar
        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            FD_SET(slave_pipe[i][0], &fd_read_pipes);
        }
        if (select(max_fd + 1, &fd_read_pipes, NULL, NULL, NULL) < 0)
        { //El problema de esperar 0 segundos es que arruinamos concurrencia, si esperamos infinito y terminaron todos ya solo corre 1 proceso
            perror("Select failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < SLAVE_COUNT; ++i)
        {
            if (FD_ISSET(slave_pipe[i][0], &fd_read_pipes))
            { //¿Que pasa si ningun hijo termino todavia? Lee basura o no lee?
                char buff[1024] = {0};
                if (read(slave_pipe[i][0], buff, 1024))
                {
                    write(STDOUT_FILENO, buff, 1024);
                    /*if (files_processed < files_to_process)
                    {
                        char argv_buffer[255] = {0};
                        strcpy(argv_buffer, argv[argv_idx++]);
                        strcat(argv_buffer, "\n");
                        write(sender[1], argv_buffer, strlen(argv_buffer));
                        printf("reciclo hijo %s\n", argv_buffer);
                        files_processed++;
                    }*/
                }
            }
        }
       

        //https://pubs.opengroup.org/onlinepubs/007908775/xsh/select.html#:~:text=The%20select()%20function%20tests,descriptors%20are%20ready%20to%20read.
    }
    killSlaves(slave_pid);
    sem_close(sem);
    sem_unlink("sent");
}


void killSlaves(pid_t slave_pid[SLAVE_COUNT]){
    for(int i = 0; i < SLAVE_COUNT;i++) {
        kill(slave_pid[i], SIGINT);
    }
    return;
}