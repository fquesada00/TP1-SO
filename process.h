#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>

#define SHM_MEM_SIZE 255
#define BUFF_SIZE 512

void throwError(char *string);

#endif
