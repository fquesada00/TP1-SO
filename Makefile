include ./Makefile.inc


all: app slave vista

app: aplicacion.c
	gcc $(CFLAGS) aplicacion.c -o aplicacion $(LIBSFLAGS)

slave: slave.c
	gcc $(CFLAGS) slave.c -o slave $(LIBSFLAGS)

vista: vista.c
	gcc $(CFLAGS) vista.c -o vista $(LIBSFLAGS)

clean: 
	rm aplicacion slave vista

.PHONY: all app slave vista clean
