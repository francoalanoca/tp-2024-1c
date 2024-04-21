#include "../include/main.h"

char *path_config;
char *nombre_interfaz;
int main(char argc, char *argv[]) {
    nombre_interfaz = argv[1];
    path_config = argv[2];
    int socket_memoria;
    int socket_kernel;

    printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;
    }


    //CONEXION//
    socket_kernel = crear_conexion(logger_entrada_salida, "KERNEL", cfg_entrada_salida->IP_KERNEL, cfg_entrada_salida->PUERTO_KERNEL);

       printf("socket de la kernel : %d\n",socket_kernel);

    socket_memoria = crear_conexion(logger_entrada_salida, "MEMORIA", cfg_entrada_salida->IP_MEMORIA, cfg_entrada_salida->PUERTO_MEMORIA);

    printf("socket de la memoria : %d\n",socket_memoria);   

    
    //INICIAR INTERFACE//
    iniciar_interface(cfg_entrada_salida->TIPO_INTERFAZ, socket_kernel, socket_memoria);

    
    
    //HANDSHAKE//

    if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_entrada_salida, "Correcto en handshake con memoria\n");
    }
    else {
        log_info(logger_entrada_salida, "Error en handshake con memoria\n");
        return EXIT_FAILURE;
    }


    if ( (hacer_handshake (socket_kernel) == HANDSHAKE)){
        log_info(logger_entrada_salida, "Correcto en handshake con kernel\n");
    }
    else {
        log_info(logger_entrada_salida, "Error en handshake con kernel\n");
        return EXIT_FAILURE;
    }
   
    cerrar_programa();
    return 0;
}