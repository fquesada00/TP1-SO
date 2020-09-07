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
    char shm_name[255] = {0};
    int shm_size;
    setvbuf(stdin, NULL, _IONBF, 0);
    //Si no recibimos nada lo leemos por entrada entanda
    if (argc == 1)
    {
        char * line = NULL;
        size_t size;
        getline(&line,&size,stdin);
        char * tok = strtok(line," ");
        strcpy(shm_name,tok);
        tok = strtok(NULL," ");
        shm_size = atoi(tok);
        free(line);
    }
    else
    {
        strcpy(shm_name, argv[1]);
        shm_size = atoi(argv[2]);
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

    int shm_fd = shm_open("shm_mem", O_RDWR, 0666);
    if (shm_fd < 0)
    {
        throwError("Shared memory failure");
    }
    void *shm_ptr = mmap(0, 25245, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        throwError("Shared mapping failure");
    }
    char *shm_char = (char *)shm_ptr;
    do
    {
        //Espero a que la aplicacion me habilite el read
        if (sem_wait(sem_read) < 0)
            throwError("sem wait read vista");
        //Imprimo a salida estandar y aumento el indice
        if(*shm_char == '\\')
            break;
        printf("%s\n", shm_char);
        shm_char += strlen(shm_char) + 1;
        //Aviso que ya puede escribir
    } while (1);
    if (sem_close(sem_write))
        throwError("Writing closing Semaphore failure");
    if (munmap(shm_ptr, shm_size))
        throwError("Unmapping Failure");
    if (close(shm_fd))
        throwError("Closing File Descriptor");
    return 0;
}