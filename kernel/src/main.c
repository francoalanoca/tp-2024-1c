#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include </home/utnso/tp-2024-1c-Pasaron-cosas/kernel/include/main.h>
#include </home/utnso/tp-2024-1c-Pasaron-cosas/kernel/include/planificador.h>
//#include <main.h>
//#include <planificador.h>


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

char *path_config;

t_planificador* planificador;


int main(char argc, char *argv[]) {
    path_config = argv[1];

    int conexion_consola;
    int conexion_entrada_salida;


    int contador_pid;

//CONFIGURACION

   if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar KERNEL");
        return EXIT_FAILURE;
    }

    crear_listas_recursos();

//INICIAR SERVIDOR y CONSOLA

    crearServidor();
 
    iniciar_consola_interactiva();
    
// ESCUCHAR A LOS MODULOS
 
    Escuchar_Msj_De_Conexiones();
 
//EMPEZAR PLANIFICACIONES


    planificador = inicializar_planificador (obtener_algoritmo_planificador(cfg_kernel-> ALGORITMO_PLANIFICACION), cfg_kernel-> QUANTUM,cfg_kernel->GRADO_MULTIPROGRAMACION);
   // planificador = inicializar_planificador (obtener_algoritmo_planificador(cfg_kernel-> ALGORITMO_PLANIFICACION), cfg_kernel-> QUANTUM);


//CERRAR PROGRAMA

    cerrar_programa();

}
