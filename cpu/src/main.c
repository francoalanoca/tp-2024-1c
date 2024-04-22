#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"


char *path_config;


int main(int argc, char* argv[]) {
    decir_hola("CPU");

    path_config = argv[1];

	int socket_memoria;
	char* ip;



     printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;

    }
    
	log_info(logger_cpu, "empieza el programa");
    crear_servidor_dispatch();
    log_info(logger_cpu, "se creo el servidor");
   
     socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);
       
     if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con kernel");
    }
    else {
        log_info(logger_cpu, "Error en handshake con kernel");
        return EXIT_FAILURE;
    }

     
   


	


	terminar_programa(socket_memoria, logger_cpu, cfg_cpu);
    return 0;
}


