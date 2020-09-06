CC= gcc
CFLAGS = -Wall -g -lpthread -lrt
DEPS: master.h slave.h
OBJ: master.o slave.o

%.o: %.c $(DEPS)
        $(CC) -c -o $@ $< $(CFLAGS)
all: $(OBJ)
        $(CC) -o $@ $^ $(CFLAGS)

.PHONY clean
clean: $(OBJ)
        rm *.o

#El % es como un grep, es decir %.o me da master.o y slave.o
#El DEPS es una macro que le indica de que dependen esos archivos,
#osea el cuando en el make all aparece un .o se fija que depende de sus .c y sus .h
#si esos cambiaron los recompila
#EL CC y el CFLAGS tambien son macros especiales que le indican el compilador
#y le indican los Flags
#Cuando hacemos un make all, se fija los objetos de la macro OBJ y si cambiaron los recompila
#Finalmente el $@ es la parte izquierda del : (es decir all) y el $^ son los objetos.
