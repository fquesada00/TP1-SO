#include "process.h"

//Confiamos que tenemos el tamaño, la tenemos que abrir y setear el puntero
//Recibimos por parametro el nombre y size
void throwError(char *string)
{
    perror(string);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    char shm_name[255]={0};
    if (argc == 1)
    {
        read(STDIN_FILENO, shm_name, 255);
    }else{
        strcpy(shm_name, argv[1]);
    }

    sem_t *sem_read = sem_open("sem_read", O_CREAT, S_IRWXU, 0);
    if (sem_read == SEM_FAILED)
    {
        throwError("Could not open Reading Semaphore");
    }

    sem_t *sem_write = sem_open("sem_write", O_CREAT, S_IRWXU, 0);
    if (sem_write == SEM_FAILED)
    {
        throwError("Could not open Writing Semaphore");
    }

    int shm_fd = shm_open(shm_name, O_RDWR, 0666); //Aca le pasamos un modo pero no sabemos que es.
    if (shm_fd < 0)
    {
        throwError("Shared memory failure");
    }
    //./master cnf/* > out.txt
    void *shm_ptr = mmap(0, SHM_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        throwError("Shared mapping failure");
    }
    //Queremos que lea si esta habilitado
    char *shm_char = (char *)shm_ptr;
    printf("llegue\n");
    if (sem_wait(sem_read))
        throwError("Reading Semaphore wait failed"); 
    while ((*shm_char) != '\\')                     
    {

        printf("%s", shm_char);// ¿ Aca ya tenemos formateado lo qe pdie la consigna? Si no podriamos hacer una funcion que formatee y imprima
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
    if(munmap(shm_ptr, SHM_MEM_SIZE))
        throwError("Unmapping Failure");   
    if(close(shm_fd))
        throwError("Closing File Descriptor");
    return 0;
}