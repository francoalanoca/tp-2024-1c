#include"../include/servidorCpu.h"
char* puerto_dispatch;
char * puerto_interrupt;
int fd_mod2 = -1;
int fd_mod3 = -1;
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
log_info(logger_cpu, "va a escuchar a la conexion: %d",conexion_kernel);
//log_info(logger_cpu, "POST SEMAFORO");
  //     sem_post(&sem_valor_instruccion);
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
    t_log *logger = malloc(sizeof(t_log));
    logger = args->log;
    int cliente_socket = args->fd;
    //char *server_name = args->server_name;
    free(args);

     op_code cop;

     ///
   
   // t_paquete* paquete = malloc(sizeof(t_paquete));

 
    while (cliente_socket != -1) {
   
        if (recv(cliente_socket, &cop, sizeof(int32_t), MSG_WAITALL) != sizeof(int32_t)) {
            log_info(logger, "DISCONNECT! KERNEL");

            break;
        }
           printf("COP:%d\n",cop);

        switch (cop){
            case NUEVO_PROCESO:
            {
                printf("llega nuevo_proceso\n");
                t_list* lista_paquete_nuevo_proceso = recibir_paquete(cliente_socket);
                t_pcb* proceso = proceso_deserializar(lista_paquete_nuevo_proceso); 
                printf("llega PATH:%s\n",proceso->path);
                pthread_mutex_lock(&mutex_proceso_actual);
                proceso_actual = proceso; //Agregar a lista de procesos?
                strcpy(proceso_actual->path,proceso->path);
                pthread_mutex_unlock(&mutex_proceso_actual);
                list_destroy_and_destroy_elements(lista_paquete_nuevo_proceso,free);
                //free(proceso);
                printf("pase free proceso\n");
                break;
            }
             case INTERRUPCION_KERNEL:
            {
                t_list* lista_paquete_proceso_interrumpido = recibir_paquete(cliente_socket);
                t_proceso_interrumpido* proceso_interrumpido = malloc(sizeof(t_proceso_interrumpido)); //REVISAR
                log_info(logger_cpu, "SE RECIBE INTERRUPCION DE KERNEL");
                proceso_interrumpido = proceso_interrumpido_deserializar(lista_paquete_proceso_interrumpido); //QUE ES LO QUE RECIBO DE KERNEL? UN PROCESO?
                if(proceso_interrumpido->pcb->pid == proceso_actual->pid){
                    pthread_mutex_lock(&mutex_proceso_interrumpido_actual);
                    proceso_interrumpido_actual = proceso_interrumpido;
                    pthread_mutex_unlock(&mutex_proceso_interrumpido_actual);
                    pthread_mutex_lock(&mutex_interrupcion_kernel);
                    interrupcion_kernel = true;
                    pthread_mutex_unlock(&mutex_interrupcion_kernel);
                }
                list_destroy_and_destroy_elements(lista_paquete_proceso_interrumpido,free);
                free(proceso_interrumpido);
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

void atender_memoria (int socket_memoria) {

    op_code cop;
  
    while (socket_memoria != -1) {

        if (recv(socket_memoria, &cop, sizeof(int32_t), MSG_WAITALL) != sizeof(int32_t)) {
            log_info(logger_cpu, "DISCONNECT! Atender memoria");

            break;
        }
    switch (cop) {

        case INSTRUCCION_RECIBIDA:
                    {
                        log_info(logger_cpu, "SE RECIBE INSTRUCCION DE MEMORIA");
                         
                        t_list* lista_paquete_instruccion_rec = recibir_paquete(socket_memoria);
                         log_info(logger_cpu, "Paquete recibido");
                        instr_t* instruccion_recibida = instruccion_deserializar(lista_paquete_instruccion_rec);
                         log_info(logger_cpu, "EL codigo de instrucción es %d ",instruccion_recibida->id);
                        if(instruccion_recibida != NULL){
                            prox_inst = instruccion_recibida;
                            log_info(logger_cpu, "EL codigo de instrucción es %d ",prox_inst->id);
                            //SEMAFORO QUE ACTIVA EL SEGUIMIENTO DEL FLUJO EN FETCH
                            list_destroy_and_destroy_elements(lista_paquete_instruccion_rec,free);
                            /*free(instruccion_recibida->param1);
                            free(instruccion_recibida->param2);
                            free(instruccion_recibida->param3);
                            free(instruccion_recibida->param4);
                            free(instruccion_recibida->param5);
                            free(instruccion_recibida);*/
                            log_info(logger_cpu, "POST SEMAFORO");
                            sem_post(&sem_valor_instruccion);
                        }
                        else{
                            log_info(logger_cpu, "ERROR AL  RECIBIR INSTRUCCION DE MEMORIA");
                            list_destroy_and_destroy_elements(lista_paquete_instruccion_rec,free);
                            /*free(instruccion_recibida->param1);
                            free(instruccion_recibida->param2);
                            free(instruccion_recibida->param3);
                            free(instruccion_recibida->param4);
                            free(instruccion_recibida->param5);
                            free(instruccion_recibida);*/
                        }
                        break;
                    }
                    case MARCO_RECIBIDO:
                    {
                        log_info(logger_cpu, "MARCO RECIBIDO");
                        t_list* lista_paquete_marco_rec = recibir_paquete(socket_memoria);
                        uint32_t marco_rec = deserealizar_marco(lista_paquete_marco_rec);
                        marco_recibido = marco_rec;
                        list_destroy_and_destroy_elements(lista_paquete_marco_rec,free);
                        sem_post(&sem_marco_recibido);
                        break;
                    }
                    case PETICION_VALOR_MEMORIA_RTA:
                    {
                        log_info(logger_cpu, "PETICION_VALOR_MEMORIA_RTA");
                        t_list* lista_paquete_valor_memoria_rec = recibir_paquete(socket_memoria);
                        char* valor_rec = deserealizar_valor_memoria(lista_paquete_valor_memoria_rec);
                    
                        valor_registro_obtenido = valor_rec; 

                        list_destroy_and_destroy_elements(lista_paquete_valor_memoria_rec,free);
                        sem_post(&sem_valor_registro_recibido);
                        break;
                    }
                    case SOLICITUD_RESIZE_RTA:
                    {
                        log_info(logger_cpu, "SOLICITUD_RESIZE_RTA");
                        t_list* lista_paquete_rta_resize = recibir_paquete(socket_memoria);
                        t_rta_resize* valor_rta_resize = deserealizar_rta_resize(lista_paquete_rta_resize);
                        strcpy(rta_resize, valor_rta_resize->rta); 

                        list_destroy_and_destroy_elements(lista_paquete_rta_resize,free);
                        free(valor_rta_resize->rta);
                        free(valor_rta_resize);
                        sem_post(&sem_valor_resize_recibido);
                        break;
                    }
                    case SOLICITUD_TAMANIO_PAGINA_RTA:
                    {
                        log_info(logger_cpu, "SOLICITUD_TAMANIO_PAGINA_RTA");
                        t_list* lista_paquete_tamanio_pag = recibir_paquete(socket_memoria);
                        uint32_t valor_tamanio_pag = deserealizar_tamanio_pag(lista_paquete_tamanio_pag); 
                        tamanio_pagina = valor_tamanio_pag; 

                        list_destroy_and_destroy_elements(lista_paquete_tamanio_pag,free);
                        sem_post(&sem_valor_tamanio_pagina);
                        break;
                    }
                    default:
                    {
                        log_error(logger_cpu, "Operacion invalida enviada desde kernel");
                        break;
                    }
                    break;
            }
        }
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;
    printf("El handshake a enviar es: %d", handshake);
    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

t_pcb *proceso_deserializar(t_list*  lista_paquete_proceso ) {
    t_pcb *proceso_nuevo = malloc(sizeof(t_pcb));
   
    proceso_nuevo->pid = *(uint32_t*)list_get(lista_paquete_proceso, 0);
    proceso_nuevo->program_counter = *(uint32_t*)list_get(lista_paquete_proceso, 1);
    proceso_nuevo->path_length = *(uint32_t*)list_get(lista_paquete_proceso, 2);
    proceso_nuevo->path = malloc(proceso_nuevo->path_length);
    proceso_nuevo->path = strdup(list_get(lista_paquete_proceso, 3));
    proceso_nuevo->registros_cpu.PC = *(uint32_t*)list_get(lista_paquete_proceso, 4);
    proceso_nuevo->registros_cpu.AX = *(uint8_t*)list_get(lista_paquete_proceso, 5);
    proceso_nuevo->registros_cpu.BX = *(uint8_t*)list_get(lista_paquete_proceso, 6);
    proceso_nuevo->registros_cpu.CX = *(uint8_t*)list_get(lista_paquete_proceso, 7);
    proceso_nuevo->registros_cpu.DX = *(uint8_t*)list_get(lista_paquete_proceso, 8);
    proceso_nuevo->registros_cpu.EAX = *(uint32_t*)list_get(lista_paquete_proceso, 9);
    proceso_nuevo->registros_cpu.EBX = *(uint32_t*)list_get(lista_paquete_proceso, 10);
    proceso_nuevo->registros_cpu.ECX = *(uint32_t*)list_get(lista_paquete_proceso, 11);
    proceso_nuevo->registros_cpu.EDX = *(uint32_t*)list_get(lista_paquete_proceso, 12);
    proceso_nuevo->registros_cpu.SI = *(uint32_t*)list_get(lista_paquete_proceso, 13);
    proceso_nuevo->registros_cpu.DI = *(uint32_t*)list_get(lista_paquete_proceso, 14);
    proceso_nuevo->estado = *(uint32_t*)list_get(lista_paquete_proceso, 15);
    proceso_nuevo->tiempo_ejecucion = *(uint32_t*)list_get(lista_paquete_proceso, 16);
    proceso_nuevo->quantum = *(uint32_t*)list_get(lista_paquete_proceso, 17);
    printf("PID DESERIALIZADO:%u\n",proceso_nuevo->pid);
    printf("PATH PROC LENGTH:%u\n",*(uint32_t*)list_get(lista_paquete_proceso, 2));
    printf("PATH PROC DESERIALIZADO:%s\n",list_get(lista_paquete_proceso, 3));
    printf("ESTADO DESERIALIZADO:%u\n",proceso_nuevo->estado);
    printf("TIEMPO EJ DESERIALIZADO:%u\n",proceso_nuevo->tiempo_ejecucion);
    printf("QUANTUM DESERIALIZADO:%u\n",proceso_nuevo->quantum);
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
    fd_mod3 = iniciar_servidor(logger_cpu, "SERVER CPU INTERRUPT", ip_cpu,  puerto_interrupt);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod3 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
log_info(logger_cpu, "va a escuchar");
    while (server_escuchar(logger_cpu, "SERVER CPU INTERRUPT", (uint32_t)fd_mod3));
}

t_proceso_interrumpido *proceso_interrumpido_deserializar(t_list*  lista_paquete_proceso_interrumpido) {
    t_proceso_interrumpido *proceso_interrumpido_nuevo = malloc(sizeof(t_proceso_interrumpido));

    proceso_interrumpido_nuevo->pcb->pid = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 0);
    proceso_interrumpido_nuevo->motivo_interrupcion = *(uint32_t*)list_get(lista_paquete_proceso_interrumpido, 1);	
    return proceso_interrumpido_nuevo;
}

instr_t* instruccion_deserializar(t_list* lista_paquete_inst){
    instr_t *instruccion_nueva = malloc(sizeof(instr_t));
    log_info(logger_cpu, "Paquete recibido Entro en instruccion deserealizar");
    char* instruccion = list_get(lista_paquete_inst, 0);
    log_info(logger_cpu,"instruccioiin recibida: %s", instruccion);
    armar_instr(instruccion_nueva,instruccion);
  
	return instruccion_nueva;
}

 uint32_t deserealizar_marco(t_list*  lista_paquete ){
    uint32_t marco_rec = malloc(sizeof(uint32_t));
    marco_rec = *(uint32_t*)list_get(lista_paquete, 0);

	return marco_rec;
}

char* deserealizar_valor_memoria(t_list*  lista_paquete ){
    uint32_t tamanio_valor_recibido = *(uint32_t*)list_get(lista_paquete, 0);
    char* valor_recibido = malloc(tamanio_valor_recibido);
    valor_recibido = list_get(lista_paquete, 1);

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

void armar_instr(instr_t *instr, const char *input) {
    // Copia la cadena de entrada para no modificar el original
    char *input_copy = strdup(input);
    char *token = strtok(input_copy, " ");
    if (token == NULL) {
        free(input_copy);
        return;
    }
      log_info(logger_cpu, "el token es %s", token );
    // Primer token es el id
    instr->id = str_to_tipo_instruccion(token);
    instr->idLength = strlen(token);

    // Inicializo estructura
    instr->param1Length = 0;
    instr->param1 = NULL;
    instr->param2Length = 0;
    instr->param2 = NULL;
    instr->param3Length = 0;
    instr->param3 = NULL;
    instr->param4Length = 0;
    instr->param4 = NULL;
    instr->param5Length = 0;
    instr->param5 = NULL;

    int param_count = 0;
    while ((token = strtok(NULL, " ")) != NULL) {
        switch (param_count) {
            case 0:
                instr->param1 = strdup(token);
                instr->param1Length = strlen(token);
                break;
            case 1:
                instr->param2 = strdup(token);
                instr->param2Length = strlen(token);
                break;
            case 2:
                instr->param3 = strdup(token);
                instr->param3Length = strlen(token);
                break;
            case 3:
                instr->param4 = strdup(token);
                instr->param4Length = strlen(token);
                break;
            case 4:
                instr->param5 = strdup(token);
                instr->param5Length = strlen(token);
                break;
            default:
                break;
        }
        param_count++;
    }

    free(input_copy);
}

void free_instr(instr_t *instr) {
    
    if (instr->param1 != NULL) free(instr->param1);
    if (instr->param2 != NULL) free(instr->param2);
    if (instr->param3 != NULL) free(instr->param3);
    if (instr->param4 != NULL) free(instr->param4);
    if (instr->param5 != NULL) free(instr->param5);
}