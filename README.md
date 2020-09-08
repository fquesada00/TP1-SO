# TP1-SO
# Compilación:
Docker:
Para compilar y ejecutar el programa se recomienda 
utilizar la imagen de docker provista por la cátedra
para hacer eso (suponiendo que tiene configurado docker):
	docker pull agodio/itba-so:1.0
Donde luego deberá instalar el minisat con el comando 
	sudo apt-get install minisat
Alternativamente se creó una nueva imagen de docker a partir
de la del usuario agodio en la cual se incluye el minisat 
con el propósito de evitar su instalación por cada ejecución de la imagen
	docker pull fquesada00/itba-so-minisat:2.0
Makefile
	Para poder compilar los archivos utilice el comando en el directorio TP1-SO
	make all
	Si desea re-compilar todo ejecute
	make clean all
# Ejecución:
Para ejecutarlo se ofrecen tres formas diferentes
Ejecutarlo en una sola terminal con ejecutando el programa aplicación
	USER$: ./aplicacion <archivos.cnf>
Ejecutarlo pipeando el output del programa aplicación al programa vista
	USER$: ./aplicacion <archivos.cnf> | ./vista
Ejecutarlo en dos terminales distintas donde el proceso aplicación se ejecuta en background (segundo plano)
el proceso vista en foreground (primer plano)
	Terminal 1: USER$: ./aplicacion <archivos.cnf> &
	Terminal 2: USER$: ./vista <Nombre_memoria>  <Tamaño_memoria>
Los valores de ambos argumentos pueden encontrarse 
ejecutando la forma 1. o 2. por STDOUT
