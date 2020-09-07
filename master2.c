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
#define INITIAL_ARGS 5

void killSlaves(pid_t slave_pid[SLAVE_COUNT]);

int main(int argc, char *argv[])
{
    pid_t slave_pid[SLAVE_COUNT];

    sem_t *sem = sem_open("sent", O_CREAT, S_IRWXU, 0);

    int files_to_process = argc - 1, files_processed = 0, max_fd = 0, argv_idx = 1;

    int sender[2];
    int slave_pipe[SLAVE_COUNT][2];
    fd_set fd_read_pipes;
    if (pipe(sender) < 0)
    {
        perror("Creating master pipe\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SLAVE_COUNT; ++i)
    {
        if (pipe(slave_pipe[i]) < 0)
        {
            perror("Creating pipe in slave\n");
            exit(EXIT_FAILURE);
        }
        //Mantengo el maximo valor de fd
        if (slave_pipe[i][0] > max_fd)
            max_fd = slave_pipe[i][0];

        if ((slave_pid[i] = fork()) < 0)
        { //Valido error
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (slave_pid[i] == 0)
        {
            if (dup2(sender[STDIN_FILENO], STDIN_FILENO) < 0 || dup2(slave_pipe[i][STDOUT_FILENO], STDOUT_FILENO) < 0 || close(sender[STDIN_FILENO]) < 0 || close(slave_pipe[i][STDOUT_FILENO]) < 0)
            {
                perror("Error with pipe conection in slave");
                exit(EXIT_FAILURE);
            }
            char *argv_slave[INITIAL_ARGS + 2];
            argv_slave[0] = "slave";
            for (int i = 1; i <= INITIAL_ARGS; i++)
            {
                if (argv_idx <= argc)
                {
                    argv_slave[i] = argv[argv_idx++];
                }
            }
            argv_slave[INITIAL_ARGS + 1] = NULL;
            execv("./slave", argv_slave);
            perror("Could not execute slave\n");
            exit(EXIT_FAILURE);
        }

        if (argv_idx + INITIAL_ARGS <= argc)
            argv_idx += INITIAL_ARGS;
        else
            argv_idx = argc;
    }

    int initial_slave_count[SLAVE_COUNT] = {0};
    int sent = SLAVE_COUNT * INITIAL_ARGS;

    while (files_processed < files_to_process)
    {
        int select_read;
        int current_read = 0;
        FD_ZERO(&fd_read_pipes);

        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            FD_SET(slave_pipe[i][0], &fd_read_pipes);
        }
        if ((select_read = select(max_fd + 1, &fd_read_pipes, NULL, NULL, NULL)) < 0)
        {
            perror("Select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < SLAVE_COUNT && current_read < select_read; ++i)
        {
            if (FD_ISSET(slave_pipe[i][0], &fd_read_pipes))
            {
                current_read++;
                char buff[1024] = {0};

                if (read(slave_pipe[i][0], buff, 512) < 0)
                {
                    perror("Read from slave pipe failed");
                    exit(EXIT_FAILURE);
                }
                files_processed++;

                //Cambiar STDOUT a mem compartida y Archivo resultados
                if (write(STDOUT_FILENO, buff, strlen(buff)) < 0)
                {
                    perror("Write to shared mem failed");
                    exit(EXIT_FAILURE);
                }
                initial_slave_count[i]++;

                if (initial_slave_count[i]>= INITIAL_ARGS && sent < files_to_process)
                {
                    char argv_buffer[255] = {0};
                    strcpy(argv_buffer, argv[argv_idx++]);
                    strcat(argv_buffer, "\n");
                    if(write(sender[1], argv_buffer, strlen(argv_buffer)) < 0)
                    {
                        perror("Error sending new file");
                        exit(EXIT_FAILURE);
                    }
                    sent++;
                    sem_post(sem);
                }
            }
        }
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
