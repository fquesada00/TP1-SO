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
#define INITIAL_ARGS 2

void killSlaves(pid_t slave_pid[SLAVE_COUNT]);


int main(int argc, char *argv[])
{
    pid_t pid;
    pid_t slave_pid[SLAVE_COUNT];
    
    sem_t *sem = sem_open("sent", O_CREAT, S_IRWXU, 0);
    int files_to_process = argc - 1, files_processed = 0, max_fd = 0,argv_idx = 1;
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
        {                                //Valido error
            perror("Forking slave\n");
            exit(EXIT_FAILURE);
        }
        else if (slave_pid[i] == 0)
        {
            close(STDOUT_FILENO);
            dup2(sender[STDIN_FILENO], STDIN_FILENO);       
            dup2(slave_pipe[i][STDOUT_FILENO], STDOUT_FILENO); //Lo que me devuelve el hijo lo mando a un pipe de ESE esclavo.
            close(sender[STDIN_FILENO]);
            close(slave_pipe[i][STDOUT_FILENO]);
            char * argv_slave[INITIAL_ARGS+2];
            argv_slave[0] = "slave";
            for(int i = 1; i <= INITIAL_ARGS ;i++) {
                if(argv_idx <= argc){
                    argv_slave[i] = argv[argv_idx++];
                }
            }
            argv_slave[INITIAL_ARGS+2] = NULL;
            execv("./slave", argv_slave); 
            perror("Could not execute slave\n");
            exit(EXIT_FAILURE);
        }
        if(argv_idx + INITIAL_ARGS <= argc)
            argv_idx += INITIAL_ARGS;
        else
            argv_idx = argc;
    }


    int initial_slave_count[SLAVE_COUNT] = {0};
    
    while (files_processed < files_to_process)
    {
        int select_read;
        FD_ZERO(&fd_read_pipes); 
        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            FD_SET(slave_pipe[i][0], &fd_read_pipes);
        }
        if ( (select_read = select(max_fd + 1, &fd_read_pipes, NULL, NULL, NULL)) < 0)
        {
            perror("Select");
            exit(EXIT_FAILURE);
        }
        
        int iter = 0;
        for (int i = 0; i < SLAVE_COUNT; ++i) 
        {
            if (FD_ISSET(slave_pipe[i][0], &fd_read_pipes))
            {
                char buff[1024] = {0};
                if(initial_slave_count[i] < INITIAL_ARGS)
                { 
                    //write(STDOUT_FILENO,"Llegue a el initial slave\n",26);
                    read(slave_pipe[i][0], buff, 1024);
                    write(STDOUT_FILENO, buff, 1024);
                    files_processed++;
                    initial_slave_count[i]++;
                }
                else 
                {                 
                    printf("el hijo %d esta listo en iteracion %d con %d select\n", slave_pid[i],++iter, select_read);
                    read(slave_pipe[i][0], buff, 1024);
                    write(STDOUT_FILENO, buff, 1024);
                    files_processed++;
                    if (files_processed < files_to_process) 
                    {
                        char argv_buffer[255] = {0};
                        strcpy(argv_buffer, argv[argv_idx++]);
                        strcat(argv_buffer, "\n");
                        write(sender[1], argv_buffer, strlen(argv_buffer));
                        sem_post(sem);
                        printf("llegue\n");
                    }
                }
            }
        }

       
        

        

        //https://pubs.opengroup.org/onlinepubs/007908775/xsh/select.html#:~:text=The%20select()%20function%20tests,descriptors%20are%20ready%20to%20read.
    }

    killSlaves(slave_pid);
    sem_close(sem);
    sem_unlink("sent");
}

void killSlaves(pid_t slave_pid[SLAVE_COUNT])
{
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        kill(slave_pid[i], SIGINT);
    }
    return;
}
