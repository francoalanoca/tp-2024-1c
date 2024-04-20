#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"

/*void* servidor(void* args) {
    t_log* logger = (t_log*)args;
    // Aquí implementa la lógica del servidor
    // Por ejemplo, aceptar conexiones entrantes y manejarlas
    // Puedes utilizar la función crear_servidor(logger) aquí si ya está implementada
    log_info(logger, "Servidor en funcionamiento");
    // Ejemplo de bucle de escucha (debes implementar esto según tus necesidades)
    crear_servidor(logger);
    return NULL;
}*/

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
    handshake_cliente(cfg_cpu, logger_cpu, socket_memoria);

     
   


	


	terminar_programa(socket_memoria, logger_cpu, cfg_cpu);
    return 0;
}
/*
t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	return nuevo_logger;
}*/

