include ./Makefile.inc


all: master slave vista

app: master2.c
	gcc $(CFLAGS) $(LIBS) master2.c -o master

vista: slaves2.c
	gcc $(CFLAGS) $(LIBS) slaves2.c -o slave

vista: vista.c
	gcc $(CFLAGS) $(LIBS) vista.c -o vista 

clean: 
	rm master slave vista

.PHONY: all master slave vista clean