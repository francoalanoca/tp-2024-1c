#include "../include/main.h"

char *path_config;
char *ip_es;

int main(int argc, char *argv[]) {
    path_config = argv[1];
    ip_es = argv[2];
    int socket_memoria;
    int socket_kernel;

    printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;
    }


    //CONEXION//
    socket_kernel = generar_conexion(logger_entrada_salida, "KERNEL", cfg_entrada_salida->IP_KERNEL, cfg_entrada_salida->PUERTO_KERNEL);

       printf("socket de la kernel : %d",socket_kernel);

    socket_memoria = generar_conexion(logger_entrada_salida, "MEMORIA", cfg_entrada_salida->IP_MEMORIA, cfg_entrada_salida->PUERTO_MEMORIA);

    printf("socket de la memoria : %d",socket_memoria);   

    //HANDSHAKE//

    if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_entrada_salida, "Correcto en handshake con memoria");
    }
    else {
        log_info(logger_entrada_salida, "Error en handshake con memoria");
        return EXIT_FAILURE;
    }


    if ( (hacer_handshake (socket_kernel) == HANDSHAKE)){
        log_info(logger_entrada_salida, "Correcto en handshake con kernel");
    }
    else {
        log_info(logger_entrada_salida, "Error en handshake con kernel");
        return EXIT_FAILURE;
    }
   
    cerrar_programa();
    return 0;
}