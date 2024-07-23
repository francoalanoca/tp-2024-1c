#include"../include/servidorCpu.h"
char* puerto_dispatch;
char * puerto_interrupt;
int fd_mod2 = -1;
//pcb *pcb_actual;

void* crear_servidor_dispatch(char* ip_cpu){
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
    fd_mod2 = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", ip_cpu,  puerto_dispatch);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod2 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
    conexion_kernel = fd_mod2;
log_info(logger_cpu, "va a escuchar");
//log_info(logger_cpu, "POST SEMAFORO");
  //     sem_post(&sem_conexion_lista);
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

     ///
   
   // t_paquete* paquete = malloc(sizeof(t_paquete));
     recv(cliente_socket, &(cop), sizeof(op_code), 0);
    //paquete->buffer = malloc(sizeof(t_buffer));
    //recv(cliente_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
    //paquete->buffer->stream = malloc(paquete->buffer->size);
    //recv(cliente_socket, paquete->buffer->stream, paquete->buffer->size, 0); //agregar &?
     ///
    printf("COP:%d\n",cop);
    while (cliente_socket != -1) {


       // if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
        if (cop != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            //return;
        }

    //switch (cop){
        switch (cop){
            case NUEVO_PROCESO:
            {
                printf("llega nuevo_proceso\n");
                t_list* lista_paquete_nuevo_proceso = recibir_paquete(cliente_socket);
                t_pcb* proceso = malloc(sizeof(t_pcb));
                proceso = proceso_deserializar(lista_paquete_nuevo_proceso); 
                proceso_actual = proceso; //Agregar a lista de procesos?
                free(proceso);
                break;
            }
             case INTERRUPCION_CPU:
            {
                t_list* lista_paquete_proceso_interrumpido = recibir_paquete(cliente_socket);
                t_proceso_interrumpido* proceso_interrumpido = malloc(sizeof(t_proceso_interrumpido)); //REVISAR
                log_info(logger_cpu, "SE RECIBE INTERRUPCION DE KERNEL");
                proceso_interrumpido = proceso_interrumpido_deserializar(lista_paquete_proceso_interrumpido); //QUE ES LO QUE RECIBO DE KERNEL? UN PROCESO?
                if(proceso_interrumpido->pcb->pid == proceso_actual->pid){
                    proceso_interrumpido_actual = proceso_interrumpido;
                    interrupcion_kernel = true;
                   
                }
            
                free(proceso_interrumpido);
                break;
            }

             case INSTRUCCION_RECIBIDA:
            {
                log_info(logger_cpu, "SE RECIBE INSTRUCCION DE MEMORIA");
                t_list* lista_paquete_instruccion_rec = recibir_paquete(cliente_socket);
                instr_t* instruccion_recibida = malloc(sizeof(instr_t));
                instruccion_recibida = instruccion_deserializar(lista_paquete_instruccion_rec);
                
                if(instruccion_recibida != NULL){
                    prox_inst = instruccion_recibida;
                    //SEMAFORO QUE ACTIVA EL SEGUIMIENTO DEL FLUJO EN FETCH
                    free(instruccion_recibida);
                    //  log_info(logger_cpu, "POST SEMAFORO");
                    // sem_post(&sem_conexion_lista);
                }
                else{
                    log_info(logger_cpu, "ERROR AL  RECIBIR INSTRUCCION DE MEMORIA");
                    free(instruccion_recibida);
                }
                break;
            }
            case MARCO_RECIBIDO:
            {
                log_info(logger_cpu, "MARCO RECIBIDO");
                t_list* lista_paquete_marco_rec = recibir_paquete(cliente_socket);
                uint32_t marco_rec = deserealizar_marco(lista_paquete_marco_rec);
                marco_recibido = marco_rec; 
                sem_post(&sem_marco_recibido);
                break;
            }
            case PETICION_VALOR_MEMORIA_RTA:
            {
                log_info(logger_cpu, "PETICION_VALOR_MEMORIA_RTA");
                t_list* lista_paquete_valor_memoria_rec = recibir_paquete(cliente_socket);
                uint32_t valor_rec = deserealizar_valor_memoria(lista_paquete_valor_memoria_rec);
               
                valor_registro_obtenido = valor_rec; 
                sem_post(&sem_valor_registro_recibido);
                break;
            }
            case SOLICITUD_RESIZE_RTA:
            {
                log_info(logger_cpu, "SOLICITUD_RESIZE_RTA");
                t_list* lista_paquete_rta_resize = recibir_paquete(cliente_socket);
                t_rta_resize* valor_rta_resize = deserealizar_rta_resize(lista_paquete_rta_resize);
                strcpy(rta_resize, valor_rta_resize->rta); 
                sem_post(&sem_valor_resize_recibido);
                break;
            }
            case SOLICITUD_TAMANIO_PAGINA_RTA:
            {
                log_info(logger_cpu, "SOLICITUD_TAMANIO_PAGINA_RTA");
                t_list* lista_paquete_tamanio_pag = recibir_paquete(cliente_socket);
                uint32_t valor_tamanio_pag = deserealizar_tamanio_pag(lista_paquete_tamanio_pag); 
                tamanio_pagina = valor_tamanio_pag; 
                sem_post(&sem_valor_tamanio_pagina);
                break;
            }
            default:
            {
                printf("Codigo de operacion no identifcado\n");
                break;
            }
            
           
    }   
//free(paquete->buffer->stream);
//free(paquete->buffer);
//free(paquete);
}
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

t_pcb *proceso_deserializar(t_list*  lista_paquete_proceso ) {
    t_pcb *proceso_nuevo = malloc(sizeof(t_pcb));
    proceso_nuevo->pid = *(uint32_t*)list_get(lista_paquete_proceso, 0);
    proceso_nuevo->program_counter = *(uint32_t*)list_get(lista_paquete_proceso, 1);
    proceso_nuevo->path_length = *(uint32_t*)list_get(lista_paquete_proceso, 2);
    proceso_nuevo->path = list_get(lista_paquete_proceso, 3);
    proceso_nuevo->registros_cpu.PC = *(uint32_t*)list_get(lista_paquete_proceso, 4);
    proceso_nuevo->registros_cpu.AX = *(uint32_t*)list_get(lista_paquete_proceso, 5);
    proceso_nuevo->registros_cpu.BX = *(uint32_t*)list_get(lista_paquete_proceso, 6);
    proceso_nuevo->registros_cpu.CX = *(uint32_t*)list_get(lista_paquete_proceso, 7);
    proceso_nuevo->registros_cpu.DX = *(uint32_t*)list_get(lista_paquete_proceso, 8);
    proceso_nuevo->registros_cpu.EAX = *(uint32_t*)list_get(lista_paquete_proceso, 9);
    proceso_nuevo->registros_cpu.EBX = *(uint32_t*)list_get(lista_paquete_proceso, 10);
    proceso_nuevo->registros_cpu.ECX = *(uint32_t*)list_get(lista_paquete_proceso, 11);
    proceso_nuevo->registros_cpu.EDX = *(uint32_t*)list_get(lista_paquete_proceso, 12);
    proceso_nuevo->registros_cpu.SI = *(uint32_t*)list_get(lista_paquete_proceso, 13);
    proceso_nuevo->registros_cpu.DI = *(uint32_t*)list_get(lista_paquete_proceso, 14);
    proceso_nuevo->estado = *(uint32_t*)list_get(lista_paquete_proceso, 15);
    proceso_nuevo->tiempo_ejecucion = *(uint32_t*)list_get(lista_paquete_proceso, 16);
    proceso_nuevo->quantum = *(uint32_t*)list_get(lista_paquete_proceso, 17);
	return proceso_nuevo;
}



void* crear_servidor_interrupt(char* ip_cpu){
    log_info(logger_cpu, "empieza crear_servidor_interrupt");

    log_info(logger_cpu, "valor de PUERTO_ESCUCHA_INTERRUPT: %s", cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
    

    puerto_interrupt = malloc((strlen(cfg_cpu->PUERTO_ESCUCHA_INTERRUPT) + 1) * sizeof(char));
if (puerto_interrupt != NULL) {
    strcpy(puerto_interrupt, cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
}
else{
    log_info(logger_cpu,"error al asignar memoria a variable del puerto");
}
    //strcpy(puerto_interrupt, cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
    log_info(logger_cpu, "crea puerto_interrupt");
    printf("El puerto_interrupt es: %s", puerto_interrupt);
    fd_mod2 = iniciar_servidor(logger_cpu, "SERVER CPU INTERRUPT", ip_cpu,  puerto_interrupt);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod2 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
log_info(logger_cpu, "va a escuchar");
    while (server_escuchar(logger_cpu, "SERVER CPU INTERRUPT", (uint32_t)fd_mod2));
}

t_proceso_interrumpido *proceso_interrumpido_deserializar(t_list*  lista_paquete_proceso_interrumpido) {
    t_proceso_interrumpido *proceso_interrumpido_nuevo = malloc(sizeof(t_proceso_interrumpido));
	uint32_t tamanio_lista = malloc(sizeof(uint32_t));

    proceso_interrumpido_nuevo->pcb->pid = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 0);
    proceso_interrumpido_nuevo->pcb->program_counter = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 1);
    proceso_interrumpido_nuevo->pcb->path_length = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 2);
    proceso_interrumpido_nuevo->pcb->path = list_get(lista_paquete_proceso_interrumpido, 3);
    proceso_interrumpido_nuevo->pcb->registros_cpu.PC = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 4);
    proceso_interrumpido_nuevo->pcb->registros_cpu.AX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 5);
    proceso_interrumpido_nuevo->pcb->registros_cpu.BX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 6);
    proceso_interrumpido_nuevo->pcb->registros_cpu.CX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 7);
    proceso_interrumpido_nuevo->pcb->registros_cpu.DX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 8);
    proceso_interrumpido_nuevo->pcb->registros_cpu.EAX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 9);
    proceso_interrumpido_nuevo->pcb->registros_cpu.EBX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 10);
    proceso_interrumpido_nuevo->pcb->registros_cpu.ECX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 11);
    proceso_interrumpido_nuevo->pcb->registros_cpu.EDX = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 12);
    proceso_interrumpido_nuevo->pcb->registros_cpu.SI = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 13);
    proceso_interrumpido_nuevo->pcb->registros_cpu.DI = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 14);
    proceso_interrumpido_nuevo->pcb->estado = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 15);
    proceso_interrumpido_nuevo->pcb->tiempo_ejecucion = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 16);
    proceso_interrumpido_nuevo->pcb->quantum = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 17);
    proceso_interrumpido_nuevo->motivo_interrupcion = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 18);	
    return proceso_interrumpido_nuevo;
}

instr_t* instruccion_deserializar(t_list* lista_paquete_inst){
        instr_t *instruccion_nueva = malloc(sizeof(instr_t));

    instruccion_nueva->idLength = *(uint32_t*)list_get(lista_paquete_inst, 0);
    instruccion_nueva->id = *(uint32_t*)list_get(lista_paquete_inst, 1);
    instruccion_nueva->param1Length = *(uint32_t*)list_get(lista_paquete_inst, 2); 
    instruccion_nueva->param1 = list_get(lista_paquete_inst, 3);
    instruccion_nueva->param2Length = *(uint32_t*)list_get(lista_paquete_inst, 4); 
    instruccion_nueva->param2 = list_get(lista_paquete_inst, 5);
    instruccion_nueva->param3Length = *(uint32_t*)list_get(lista_paquete_inst, 6); 
    instruccion_nueva->param3 = list_get(lista_paquete_inst, 7);
    instruccion_nueva->param4Length = *(uint32_t*)list_get(lista_paquete_inst, 8); 
    instruccion_nueva->param4 = list_get(lista_paquete_inst, 9);
    instruccion_nueva->param5Length = *(uint32_t*)list_get(lista_paquete_inst, 10); 
    instruccion_nueva->param5 = list_get(lista_paquete_inst, 11);

	return instruccion_nueva;
}

 uint32_t deserealizar_marco(t_list*  lista_paquete ){
    uint32_t marco_recibido = malloc(sizeof(uint32_t));
    marco_recibido = *(uint32_t*)list_get(lista_paquete, 0);

	return marco_recibido;
}

uint32_t deserealizar_valor_memoria(t_list*  lista_paquete ){
    uint32_t valor_recibido = malloc(sizeof(uint32_t));
    valor_recibido = *(uint32_t*)list_get(lista_paquete, 0);

	return valor_recibido;
}

t_rta_resize* deserealizar_rta_resize(t_list*  lista_paquete ){
    t_rta_resize* valor_rta_resize = malloc(sizeof(t_rta_resize));
    valor_rta_resize->tamanio_rta= *(uint32_t*)list_get(lista_paquete, 0);
    valor_rta_resize->rta= list_get(lista_paquete, 1);
	return valor_rta_resize;
}

uint32_t deserealizar_tamanio_pag(t_list*  lista_paquete ){
    uint32_t valor_tam_pag = malloc(sizeof(uint32_t));
    valor_tam_pag = *(uint32_t*)list_get(lista_paquete, 0);

	return valor_tam_pag;
}