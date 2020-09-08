#include "process.h"

//Confiamos que tenemos el tamaño, la tenemos que abrir y setear el puntero
//Recibimos por parametro el nombre y size
int main(int argc, char *argv[])
{
    char shm_name[BUFF_SIZE] = {0};
    int shm_size;
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
        throwError("Disable buffering");
    
    //Si no recibimos nada lo leemos por entrada entanda
    if (argc == 1)
    {
        char *line = NULL;
        size_t size;
        if (getline(&line, &size, stdin) == -1)
            throwError("Getting line");
        
        char *tok = strtok(line, " ");
        strcpy(shm_name, tok);
        tok = strtok(NULL, " ");
        shm_size = atoi(tok);
        free(line);
    }
    else
    {
        strcpy(shm_name, argv[1]);
        shm_size = atoi(argv[2]);
    }
    
    printf("%s %d",shm_name,shm_size);
    if (shm_size <= 0)
    {
        throwError("Reading memory size");
    }
    
    sem_t *sem_read = sem_open("sem_read", O_CREAT, S_IRWXU, 0);
    if (sem_read == SEM_FAILED)
    {
        throwError("Reading Semaphore");
    }

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd < 0)
    {
        throwError("Shared memory");
    }
    void *shm_ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        throwError("Shared mapping");
    }
    
    char *shm_char = (char *)shm_ptr;
    
    do{
        //Espero a que la aplicacion me habilite el read
        if (sem_wait(sem_read) < 0)
            throwError("Semaphore wait reading vista");
        //Imprimo a salida estandar y aumento el indice
        if (*shm_char == '\\')
            break;
        printf("%s\n", shm_char);
        shm_char += strlen(shm_char) + 1;
        //Aviso que ya puede escribir
    } while (1);
    
    if (sem_close(sem_read))
        throwError("Reading closing Semaphore");
    if (munmap(shm_ptr, shm_size))
        throwError("Unmapping Failure");
    if (close(shm_fd))
        throwError("Closing File Descriptor");
    
    return 0;
}
