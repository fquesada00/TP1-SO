include Makefile.inc

#Dependencia general
all: master slave vista

master: master.c
        $(GCC) $(CFLAGS) $(LIBS) master2.c -o master

slave: slaves2.c        
        $(GCC) $(CFLAGS) $(LIBS) slaves2.c -o slave

vista: vista.c
        $(GCC) $(CFLAGS) $(LIBS) vista.c -o vista

clean: 
        rm master slave vista

#Regla general independiente de su ejecucion/peticion
.PHONY: all master slave vista clean
