#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/main.h"
#include "../include/planificador.h"
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


//INICIAR SERVIDOR Y CONSOLA INTERACTIVA 

    pthread_t hilo1, hilo2, hilo3;

    pthread_create(&hilo1, NULL, (void*)iniciar_consola_interactiva, NULL);
    pthread_create(&hilo2, NULL, (void*)crearServidor, NULL);
    pthread_create(&hilo3, NULL, (void*)Escuchar_Msj_De_Conexiones, NULL);

    // ESPERAR A QUE LOS HILOS TERINEN (OPCIONAL, EN CASO DE QUE QUIERAS ESPERAR)

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);
    pthread_join(hilo3, NULL);

    return 0;
}

{
//CERRAR PROGRAMA

    cerrar_programa();

}
