include ./Makefile.inc

#Dependencias 
all: app slave vista

app: aplicacion.c
	gcc $(CFLAGS) aplicacion.c libTp1.c -o aplicacion $(LIBSFLAGS)

slave: slave.c
	gcc $(CFLAGS) slave.c -o slave $(LIBSFLAGS)

vista: vista.c
	gcc $(CFLAGS) vista.c libTp1.c -o vista $(LIBSFLAGS)

clean: 
	rm aplicacion slave vista

#Pedido explicito sin importar su modificacion
.PHONY: all app slave vista clean
