#include <servidor_kernel.h>



void* crearServidor(){
 
    socket_servidor = iniciar_servidor(logger_kernel, "SERVER KERNEL", "127.0.0.0", cfg_kernel->PUERTO_ESCUCHA);
    if (socket_servidor == 0) {
        log_error(logger_kernel, "Fallo al crear el servidor, cerrando KERNEL");
        return EXIT_FAILURE;
    }

    while (server_escuchar(logger_kernel, "SERVER KERNEL", (uint32_t)socket_servidor));
}

int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t atenderNuevaConexion;
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&atenderNuevaConexion, NULL,procesar_conexion,args);
        pthread_detach(atenderNuevaConexion);
        return 1;
    }
    return 0;
}


void procesar_conexion(void *void_args) {
    t_procesar_conexion_args *args = (t_procesar_conexion_args *) void_args;
    t_log *logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);
    uint32_t response;
    op_code cop;
    while (cliente_socket != -1) {

        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

        switch (cop) {
            
            case HANDSHAKE:
                log_info(logger, "Handshake recibido");
                response = HANDSHAKE;
                if (send(cliente_socket, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger, "Error al enviar respuesta de handshake a cliente");
                    break;
                }
            
            case HANDSHAKE_OK:

                log_info(logger_kernel, "handshake recibido exitosamente con I/O");
                break;

            break;
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
        }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}