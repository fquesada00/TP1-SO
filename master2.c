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
#include <sys/shm.h>
#include <sys/mman.h>

#define SLAVE_COUNT 5
#define INITIAL_ARGS 5
#define MEM_SIZE 512

void killSlaves(pid_t slave_pid[SLAVE_COUNT]);

int main(int argc, char *argv[])
{
    FILE * result = fopen("result.txt", "w+");
    setvbuf(stdout,NULL,_IONBF,0);
    setvbuf(result,NULL,_IONBF,0);
    char * shared_mem_name = "shr_mem";
    
    int shm_fd = shm_open(shared_mem_name, O_CREAT | O_RDWR, 0666);//a le pasamos un modo pero no sabemos que es.
    ftruncate(shm_fd,MEM_SIZE);
    void * shared_mem = mmap(NULL,MEM_SIZE , PROT_READ | PROT_WRITE , MAP_SHARED, shm_fd, 0);
    if(shared_mem == MAP_FAILED)
    {
        perror("map");
        exit(EXIT_FAILURE);
    }
    pid_t slave_pid[SLAVE_COUNT];
    sem_t *sem_read = sem_open("sem_read",O_CREAT,S_IRWXU,0);
    sem_t *sem_write = sem_open("sem_write",O_CREAT,S_IRWXU,1);
    printf("%s",shared_mem_name);
    fflush(stdout);
    sleep(2);

    int files_to_process = argc - 1, files_processed = 0, max_fd = 0, argv_idx = 1;

    int slave_to_master[SLAVE_COUNT][2];
    int master_to_slave[SLAVE_COUNT][2];
    fd_set fd_read_pipes;

    for (int i = 0; i < SLAVE_COUNT; ++i)
    {
        if (pipe(slave_to_master[i]) < 0)
        {
            perror("Creating pipe in slave\n");
            exit(EXIT_FAILURE);
        }
        if(pipe(master_to_slave[i]) < 0)
        {
            perror("Creating pipe in master\n");
            exit(EXIT_FAILURE);
        }
        //Mantengo el maximo valor de fd
        if (slave_to_master[i][0] > max_fd)
            max_fd = slave_to_master[i][0];

        if ((slave_pid[i] = fork()) < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (slave_pid[i] == 0)
        {
            if (dup2(master_to_slave[i][STDIN_FILENO], STDIN_FILENO) < 0 || dup2(slave_to_master[i][STDOUT_FILENO], STDOUT_FILENO) < 0 || close(master_to_slave[i][STDIN_FILENO]) < 0 || close(slave_to_master[i][STDOUT_FILENO]) < 0)
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
            FD_SET(slave_to_master[i][0], &fd_read_pipes);
        }
        if ((select_read = select(max_fd + 1, &fd_read_pipes, NULL, NULL, NULL)) < 0)
        {
            perror("Select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < SLAVE_COUNT; ++i)
        {
            if (FD_ISSET(slave_to_master[i][0], &fd_read_pipes))
            {
                current_read++;
                char buff[1024] = {0};

                if (read(slave_to_master[i][0], buff, 512) < 0)
                {
                    perror("Read from slave pipe failed");
                    exit(EXIT_FAILURE);
                }
                files_processed++;
                
                //printf("processed = %d\n",files_processed);
                //write(STDOUT_FILENO,buff,strlen(buff));
               // Cambiar STDOUT a mem compartida y Archivo resultados
                /*fputs(buff, result);
                int a;
                sem_getvalue(sem_write, &a);
                fprintf(result, "valor: %d\n",a);*/
                //fflush(result);
                sem_wait(sem_write); //
                //strcpy((char *) shared_mem, buff);
                sprintf((char *)shared_mem,"%s",buff);
                //fprintf(result, "%s", (char *) shared_mem);
                //fputs("hola\n", result);
                //memcpy(shared_mem,buff,strlen(buff));
                fputs(shared_mem, result);
                sem_post(sem_read);
                initial_slave_count[i]++;

                if (initial_slave_count[i]>= INITIAL_ARGS && sent < files_to_process)
                {
                    char argv_buffer[255] = {0};
                    strcpy(argv_buffer, argv[argv_idx++]);
                    strcat(argv_buffer, "\n");
                    if(write(master_to_slave[i][1], argv_buffer, strlen(argv_buffer)) < 0)
                    {
                        perror("Error sending new file");
                        exit(EXIT_FAILURE);
                    }
                    sent++;
                }
            }
        }
    }
    sprintf((char *)shared_mem,"%s","\\");
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        close(master_to_slave[i][0]);
        close(master_to_slave[i][1]);
        close(slave_to_master[i][0]);
        close(slave_to_master[i][1]);

    }
    killSlaves(slave_pid);
    sem_close(sem_write);
    sem_unlink("sem_write");
    sem_close(sem_read);
    sem_unlink("sem_read");
    munmap(shared_mem, shm_fd);
    shm_unlink(shared_mem_name);
    return 0;
}

void killSlaves(pid_t slave_pid[SLAVE_COUNT])
{
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        kill(slave_pid[i], SIGINT);
    }
    return;
} 

