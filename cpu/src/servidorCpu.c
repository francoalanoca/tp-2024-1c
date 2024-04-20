#include"../include/servidorCpu.h"
int puerto_dispatch;
int fd_mod2 = -1;
void* crear_servidor_dispatch(){
    log_info(logger_cpu, "empieza crear_servidor_dispatch");
    puerto_dispatch = cfg_cpu->PUERTO_ESCUCHA_DISPATCH;
    log_info(logger_cpu, "crea puerto_dispatch");
    printf("El puerto_dispatch es: %d", puerto_dispatch);
    fd_mod2 = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", "127.0.0.1", puerto_dispatch);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod2 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }

    while (server_escuchar(logger_cpu, "SERVER cpu", (uint32_t)fd_mod2));
}

int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
        pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,cliente_socket);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

void procesar_conexion(int cliente_socket){
    printf("El socket del cliente es: %d", cliente_socket);
}

