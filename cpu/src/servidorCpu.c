#include"../include/servidorCpu.h"
char* puerto_dispatch;
int fd_mod2 = -1;
//pcb *pcb_actual;

void* crear_servidor_dispatch(){
    log_info(logger_cpu, "empieza crear_servidor_dispatch");

    log_info(logger_cpu, "valor de PUERTO_ESCUCHA_DISPATCH: %s", cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
    

    puerto_dispatch = malloc((strlen(cfg_cpu->PUERTO_ESCUCHA_DISPATCH) + 1) * sizeof(char));
if (puerto_dispatch != NULL) {
    strcpy(puerto_dispatch, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
}
else{
    log_info(logger_cpu,"error al asignar memoria a variable del puerto");
}
    //strcpy(puerto_dispatch, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
    log_info(logger_cpu, "crea puerto_dispatch");
    printf("El puerto_dispatch es: %s", puerto_dispatch);
    fd_mod2 = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", "127.0.0.1",  puerto_dispatch);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod2 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
log_info(logger_cpu, "va a escuchar");
    while (server_escuchar(logger_cpu, "SERVER CPU", (uint32_t)fd_mod2));
}

int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    log_info(logger_cpu, "entra a server escuchar");
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    log_info(logger_cpu, "sale de esperar_cliente");

    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
         t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        //pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,cliente_socket);//TODO:Redefinir procesar_conexion para que reciba un PCB
        pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

/*
void procesar_conexion(int cliente_socket){
    printf("El socket del cliente es: %d", cliente_socket);
}*/

void procesar_conexion(void *v_args){
     t_procesar_conexion_args *args = (t_procesar_conexion_args *) v_args;
    t_log *logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

     op_code cop;

    while (cliente_socket != -1) {

        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

    switch (cop){
            case PCB:
            {
                log_info(logger_cpu, "PCB RECIBIDO");
               // pcb_actual = deserializar_pcb(cliente_socket);
                break;
            }
    }    
}
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

