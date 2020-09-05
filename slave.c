#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
int main(int argc, char const *argv[]){// ./slave ./formulas/hole8.cnf ./formulas/hole*...
    char minisat[255]={0};
    strcat(minisat,"minisat ");
    int n = strlen(minisat);
    FILE * file =NULL;
    for(int i = 1; i < argc;i++)
    {
        strcat(minisat, argv[i]);
        strcat(minisat," | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e  \".*SATISFIABLE\"");
        file = popen(minisat,"r");
    
        char buf[255]={0};
        fread((void *)buf,1,255,file);
        puts(buf);
        minisat[n]=0;
    }
   // mandar_señal_padre
   // semaforo espero hasta que me entregue un nuevo archivo el problema de los semaforos es que no podemos identificar la hijo 
   // retomo_
   if(file != NULL)
        pclose(file);
    kill(getppid(),10);
    return 0;

}
//padre manda n --> enviados = n
//hijo termina --> hace sem post, y hace sem wait de un semaforo