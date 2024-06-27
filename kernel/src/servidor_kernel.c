#include "../include/servidor_kernel.h"



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
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
            case INTERFAZ_ENVIAR:
                printf("Received INTERFAZ_ENVIAR request\n");

                t_list* lista_paquete_interfaz = malloc(sizeof(t_list));
                lista_paquete_interfaz = recibir_paquete(conexion_cpu_dispatch);

         
                t_interfaz* interfaz_recibida = deserializar_interfaz(lista_paquete_interfaz);
        
                if (lista_paquete_interfaz == NULL || list_size(lista_paquete_interfaz) == 0) {
                    printf("Failed to receive data or empty list\n");
                    break;
                }
                // armar estructura con la interfaz recibida y cargar al diccionario
                t_interfaz_diccionario* interfaz_nueva = malloc(sizeof(t_interfaz_diccionario));
                interfaz_nueva->nombre = interfaz_recibida->nombre;
                interfaz_nueva->tipo = interfaz_recibida->tipo;
                interfaz_nueva->conexion = cliente_socket;
			    
                dictionary_put(interfaces,interfaz_recibida->nombre,interfaz_nueva);

                uint32_t response_interfaz = INTERFAZ_RECIBIDA;
                if (send(cliente_socket, &response_interfaz, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send INTERFAZ_RECIBIDA response");
                }

                free(interfaz_nueva);   
                free(lista_paquete_interfaz);
                break;
            }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}



void Empezar_conexiones(){

    //conexion con cpu-dispatch
    conexion_cpu_dispatch = crear_conexion(logger_kernel, "KERNEL", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_DISPATCH);
    
    log_info(logger_kernel, "Socket de KERNEL : %d\n",conexion_cpu_dispatch);  

    //conexion con cpu-interrupt
    conexion_cpu_interrupt = crear_conexion(logger_kernel, "KERNEL", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_INTERRUPT);
    
    log_info(logger_kernel, "Socket de KERNEL : %d\n",conexion_cpu_interrupt);

    //conexion con memoria
    conexion_memoria = crear_conexion(logger_kernel, "MEMORIA", cfg_kernel->IP_MEMORIA, cfg_kernel->PUERTO_MEMORIA);
    
    log_info(logger_kernel, "Socket de MEMORIA : %d\n",conexion_memoria); 

}