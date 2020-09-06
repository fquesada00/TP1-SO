all : master2.o slaves2.o
        gcc -o TP1 master2.o slaves2.o
master2.o : master2.c
        gcc master2.c -Wall -g -lpthread -lrt -c
slaves2.o : slaves2.c
        gcc slaves2.c -Wall -g -lpthread -lrt -c
clean : 
        rm master.o slave.o 

