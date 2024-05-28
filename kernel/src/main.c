#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <main.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

char *path_config;


int main(char argc, char *argv[]) {
    path_config = argv[1];

    int conexion_memoria;
    int conexion_cpu_dispatch;
    int conexion_consola;
    int conexion_entrada_salida;

    int contador_pid;

//CONFIGURACION

   if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar KERNEL");
        return EXIT_FAILURE;
    }

//CONEXION
    conexion_cpu_dispatch = crear_conexion(logger_kernel, "KERNEL", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_DISPATCH);
    
    log_info(logger_kernel, "Socket de KERNEL : %d\n",conexion_cpu_dispatch);   
    
    conexion_memoria = crear_conexion(logger_kernel, "MEMORIA", cfg_kernel->IP_MEMORIA, cfg_kernel->PUERTO_MEMORIA);
    
    log_info(logger_kernel, "Socket de MEMORIA : %d\n",conexion_memoria);      

//CONSOLA

//INICIAR SERVIDOR
    crearServidor();
 
    iniciar_consola_interactiva();
    
    planificador = inicializar_planificador (obtener_algoritmo_planificador(cfg_kernel-> ALGORITMO_PLANIFICACION), cfg_kernel-> QUANTUM);


    cerrar_programa();
}
