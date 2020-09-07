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

//Confiamos que tenemos el tamaño, la tenemos que abrir y setear el puntero
//Recibimos por parametro el nombre y size
void throwError(char *string)
{
    perror(string);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    printf("%d\n",argc);
    if (argc != 3)
    {
        throwError("Wrong Arguments!");
    }

    sem_t *sem_read = sem_open("shm_read", O_CREAT, S_IRWXU, 0);
    if (sem_read == SEM_FAILED)
    {
        throwError("Could not open Reading Semaphore");
    }

    sem_t *sem_write = sem_open("shm_write", O_CREAT, S_IRWXU, 0);
    if (sem_write == SEM_FAILED)
    {
        throwError("Could not open Writing Semaphore");
    }

    int mem_size = atoi(argv[2]);
    if (mem_size == 0)
    {
        throwError("Memory Size Not Valid/Zero");
    }

    char *shm_name = argv[1];
    int shm_fd = shm_open(shm_name, O_RDWR, 0666); //Aca le pasamos un modo pero no sabemos que es.
    if (shm_fd < 0)
    {
        throwError("Shared memory failure");
    }

    void *shm_ptr = mmap(0, mem_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        throwError("Shared mapping failure");
    }
    //Queremos que lea si esta habilitado
    char *shm_char = (char *)shm_ptr; 
    if (sem_wait(sem_read))
        throwError("Reading Semaphore wait failed"); 
    while ((*shm_char) != '\\')                     
    {

        printf("%s\n", shm_char);// ¿ Aca ya tenemos formateado lo qe pdie la consigna? Si no podriamos hacer una funcion que formatee y imprima
        //shm_char += strlen(shm_char) * sizeof(char);
        if (sem_post(sem_write))
            throwError("Writing Semaphore post failed");
        if (sem_wait(sem_read))
            throwError("Reading Semaphore wait failed");//ok yo me meto al meet cuando pueda corto y dsp me uno
    }
    if(sem_post(sem_write))
        throwError("Writing post failed");
    if(sem_close(sem_read))
        throwError("Reading closing Semaphore failure");
    if(sem_close(sem_write))
        throwError("Writing closing Semaphore failure");//
    if(munmap(shm_ptr, mem_size))
        throwError("Unmapping Failure");   
    if(close(shm_fd))
        throwError("Closing File Descriptor");
    return 0;
}