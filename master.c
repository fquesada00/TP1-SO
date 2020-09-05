#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#define SLAVE_COUNT 5
#define INIT_FILES 1
int main(int argc, char const *argv[])
{
    
    sem_unlink("sent");
    sem_t *sent = sem_open("sent", O_CREAT, S_IRWXU, 0);

    int sender[2];
    if (pipe(sender) < 0)
    {
        perror("Error creating pipe");
        exit(EXIT_FAILURE);
    }

    int max_fd = 0;

    fd_set fds_read;
    FD_ZERO(&fds_read);
    int slave_pipe[SLAVE_COUNT][2];
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        if (pipe(slave_pipe[i]) < 0)
        {
            perror("Error creating pipe");
            exit(EXIT_FAILURE);
        }
        if (slave_pipe[i][0] > max_fd)
            max_fd = slave_pipe[i][0];
        FD_SET(slave_pipe[i][0], &fds_read);
        if (fork() == 0)
        {
            dup2(slave_pipe[i][STDOUT_FILENO], STDOUT_FILENO);
            dup2(sender[STDIN_FILENO], STDIN_FILENO);
            close(sender[0]);
            close(sender[1]);
            for (int j = 0; j < i; j++)
            {
                close(slave_pipe[j][0]);
                close(slave_pipe[j][1]);
            }
            execl("./slave", "slave", NULL);
            perror("Exec error");
            exit(EXIT_FAILURE);
        }
    }
    FILE * result = fopen("resultado", "w+");
    if(result == NULL){
        perror("resultado FILE");
        exit(EXIT_FAILURE);
    }
    for (int i = 1 ; argv[i]!=NULL ; i++)
    {
        
        char argv_buffer[255]={0};
        strcpy(argv_buffer, argv[i]);
        strcat(argv_buffer,"\n");
        write(sender[1], argv_buffer, strlen(argv_buffer));
        sem_post(sent); //habilito semaforo
        int result_select;
        if ((result_select = select(max_fd + 1, &fds_read, NULL, NULL, NULL)) < 0)
        { //el primer hijo q salga, lo tomo como rta
            perror("Select fail");
            exit(EXIT_FAILURE);
        }
        //me fijo q hijo fue el q cargo
        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            
            if (FD_ISSET(slave_pipe[i][0], &fds_read))
            {
                char buff[255] = {0};
                read(slave_pipe[i][0], buff, 255);
                fputs(buff,result);
                write(1, buff, 255);
                //FD_CLR(slave_pipe[i][0], &fds_read);                    
            }
        }
        //reinicio pipes
        FD_ZERO(&fds_read);
        for (int i = 0; i < SLAVE_COUNT; i++)
        {
            FD_SET(slave_pipe[i][0], &fds_read);
        }
    }
    /*
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        wait(NULL);
    }
    */
    //fclose(result); descomentar
    sem_close(sent);
    sem_unlink("sent");
    return 0;
}