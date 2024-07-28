#include <../include/protocolo.h>

//Funcion que atiende la peticion de Kernel segun el cod
void memoria_atender_cliente(void* socket){
     int socket_cliente = *(int*)socket;
    uint32_t cod_op;
	op_code response;
    t_list* valores =  malloc(sizeof(t_list));
    t_io_memo_escritura* input = malloc(sizeof(t_io_memo_escritura));
    
	void* output;  



    while (1) {
       
    //uint32_t cod_op = recibir_operacion(socket_cliente);
        if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
            log_info(logger_memoria, "DISCONNECT!");

            break;
        }
    switch (cod_op) {
		
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con cliente");
            response = HANDSHAKE_OK;
            if (send(socket_cliente, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a cliente");
                   
                break;
            }
            break;

/*---------------------------- KERNEL-------------------------*/
        case CREAR_PROCESO_KERNEL:
            log_info(logger_memoria, "Recibí CREAR_PROCESO_KERNEL \n");
            valores = recibir_paquete(socket_cliente);
            t_m_crear_proceso *iniciar_proceso = deserializar_crear_proceso(valores);
            leer_instrucciones(iniciar_proceso->archivo_pseudocodigo, iniciar_proceso->pid);                  
            crear_proceso(iniciar_proceso->pid);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_respuesta_crear_proceso(iniciar_proceso, socket_cliente);
            log_info(logger_memoria, "enviada respuesta de CREAR_PROCESO_KERNEL_FIN \n");
            break;

		case FINALIZAR_PROCESO:
            log_info(logger_memoria, "Recibí FINALIZAR_PROCESO \n");
            valores = recibir_paquete(socket_cliente);
            uint32_t pid_proceso_a_finalizar = deserializar_finalizar_proceso(valores);
            finalizar_proceso(pid_proceso_a_finalizar);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, socket_cliente);
            log_info(logger_memoria, "enviada respuesta de FINALIZAR_PROCESO_FIN \n");
            break;

/*---------------------------- CPU-------------------------*/
        case PROXIMA_INSTRUCCION:
            log_info(logger_memoria, "Recibí PROXIMA_INSTRUCCION \n");
            valores = recibir_paquete(socket_cliente);
            t_proceso_memoria* solicitud_instruccion = deserializar_proxima_instruccion(valores);         
            char* instruccion = buscar_instruccion(solicitud_instruccion->pid, solicitud_instruccion->program_counter);
            log_trace(logger_memoria, "Se Encontro la Instruccion: %s", instruccion);
		    usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_respuesta_instruccion(instruccion, socket_cliente);     
            log_info(logger_memoria, "enviada respuesta de INSTRUCCION_RECIBIDA \n");
            break;

        case PEDIDO_MARCO_A_MEMORIA:
            log_info(logger_memoria, "Recibida PEDIDO_MARCO_A_MEMORIA\n");
            valores = recibir_paquete(socket_cliente);
            t_busqueda_marco* solicitud_marco = deserializar_solicitud_marco(valores);
            int marco = buscar_marco_pagina(solicitud_marco->pid, solicitud_marco->nro_pagina);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_solicitud_marco(marco, socket_cliente);
            log_info(logger_memoria, "Marco enviado %d \n", marco);
            break;


        case SOLICITUD_TAMANIO_PAGINA:
            log_info(logger_memoria, "Recibí SOLICITUD_TAMANIO_PAGINA \n");
            printf("Envio tamaño de pagina\n");
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_solicitud_tamanio(cfg_memoria->TAM_PAGINA, socket_cliente);
            log_info(logger_memoria, "enviada respuesta de SOLICITUD_TAMANIO_PAGINA_RTA \n");
            break;

        case PETICION_VALOR_MEMORIA:
            log_info(logger_memoria, "Recibí PETICION_VALOR_MEMORIA \n");
            valores = recibir_paquete(socket_cliente);
            t_escribir_leer* peticion_leer = deserializar_peticion_valor(valores);     
            void* respuesta_leer = leer_memoria(peticion_leer->pid, peticion_leer->direccion_fisica, peticion_leer->tamanio);
            log_trace(logger_memoria, "Log Obligatorio: \n");
            log_info(logger_memoria, "Acceso a espacio de usuario: \n");          
            log_info(logger_memoria, "PID: %d - Acción: LEER - Direccion fisica: %d - Tamaño: %d", peticion_leer->pid, peticion_leer->direccion_fisica, peticion_leer->tamanio);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);    
            enviar_peticion_valor(respuesta_leer, socket_cliente);
            log_info(logger_memoria, "enviada respuesta de PETICION_VALOR_MEMORIA_RTA \n");
            free(respuesta_leer);                            
            break;

        case GUARDAR_EN_DIRECCION_FISICA:
            log_info(logger_memoria, "Recibí GUARDAR_EN_DIRECCION_FISICA \n");
            valores = recibir_paquete(socket_cliente);
            t_escribir_leer* peticion_escribir = deserializar_peticion_guardar(valores);   
            char* respuesta_escribir = escribir_memoria(peticion_escribir->pid, peticion_escribir->direccion_fisica, peticion_escribir->valor, peticion_escribir->tamanio);
            log_trace(logger_memoria, "Log Obligatorio: \n");
            log_info(logger_memoria, "Acceso a espacio de usuario: \n");
            log_info(logger_memoria, "PID: %d - Acción: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", peticion_escribir->pid, peticion_escribir->direccion_fisica, peticion_escribir->tamanio);
            free(peticion_escribir);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_resultado_guardar(respuesta_escribir, socket_cliente);
            log_info(logger_memoria, "enviada respuesta de GUARDAR_EN_DIRECCION_FISICA_RTA \n");
            free(respuesta_escribir);
            break;

        case SOLICITUD_RESIZE:
            log_info(logger_memoria, "SOLICITUD_RESIZE recibida \n");
            valores = recibir_paquete(socket_cliente);
            t_resize* solicitud_resize = deserializar_solicitud_resize(valores);
            op_code respuesta_resize = administrar_resize(solicitud_resize->pid, solicitud_resize->tamanio);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            if (send(socket_cliente, &respuesta_resize, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de resize a cliente");
                   
                break;
            }
            log_info(logger_memoria, "enviada respuesta de SOLICITUD_RESIZE \n");
            break;

        case ENVIO_COPY_STRING_A_MEMORIA:
            log_info(logger_memoria, "Recibí ENVIO_COPY_STRING_A_MEMORIA \n");
            valores = recibir_paquete(socket_cliente);
            t_copy* copiar_valor = deserializar_solicitud_copy(valores);
            void* respuesta_copy = copiar_solicitud(copiar_valor->pid, copiar_valor->direccion_fisica, copiar_valor->valor);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_resultado_copiar(respuesta_copy, socket_cliente);
            log_info(logger_memoria, "enviada respuesta de ENVIO_COPY_STRING_A_MEMORIA_RTA \n");
            free(respuesta_copy);
            break;
		
/*---------------------------- ENTRADASALIDA-------------------------*/  
        case IO_M_STDIN: // lee de teclado y escribe en memoria
            log_info(logger_memoria, "Recibida IO_M_STDIN \n");
            // Llenamos la lista con los datos recibidos de recibir_paquete 
            valores = recibir_paquete(socket_cliente);

            // Deserializamos los valores de la lista 
            input = deserializar_input(valores);

            if (valores == NULL || list_size(valores) == 0) {
                printf("El paquete vino vacío\n");
                break;
            }        
            
            int dir = (int) input->direcciones_fisicas;
            escribir_memoria(input->pid, dir , input->input, input->input_length);    //ver
            response = IO_M_STDIN_FIN;
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            if (send(socket_cliente, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send IO_M_STDIN response");
                break;
            }

            log_info(logger_memoria, "Memoria envio IO_M_STDIN_FIN a IO \n");
            list_clean(valores);
            break;           
            
		case IO_M_STDOUT: // Lee de memoria e imprime por pantalla
            log_info(logger_memoria, "Recibida IO_M_STDOUT \n");

            t_io_output* io_output = malloc(sizeof(t_io_output));
            t_io_direcciones_fisicas* io_stdout = malloc(sizeof(t_io_direcciones_fisicas));
                
            valores = recibir_paquete(socket_cliente);
            io_stdout = deserializar_io_df(valores);
        
            if (valores == NULL || list_size(valores) == 0) {
                printf("El paquete vino vacío\n");
                break;
            }

             
            output =  leer_memoria(io_stdout->pid, (int) input->direcciones_fisicas, io_stdout->tamanio_operacion);
            uint32_t tamanio_output = string_length(output)+1;
            io_output->pid = io_stdout->pid;
            io_output->output_length = tamanio_output;
            io_output->output = output;

            printf("Tamanio output %d\n",io_output->output_length);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_output(io_output ,socket_cliente, IO_M_STDOUT_FIN );
            log_info(logger_memoria, "Sevidor Memoria envía IO_M_STDOUT_FIN al cliente\n");
            list_clean(valores);    
            break;
            
        case IO_FS_WRITE://Lee de memoria y escribe en un archivo
            log_info(logger_memoria, "Recibida IO_FS_WRITE \n");

            t_io_output* io_escritura = malloc(sizeof(t_io_output));
            t_io_direcciones_fisicas* io_fs_write = malloc(sizeof(t_io_direcciones_fisicas));
                
                                
            valores = recibir_paquete(socket_cliente);
            io_fs_write = deserializar_io_df(valores);
        
            if (valores == NULL || list_size(valores) == 0) {
                printf("El paquete vino vacío\n");
                break;
            } 
            printf("direccionde posicion  %d\n", (int) input->direcciones_fisicas);
            char* escritura = leer_memoria(io_fs_write->pid, (int) input->direcciones_fisicas, io_fs_write->tamanio_operacion);     //ver
            
            printf("Lectura %s\n",escritura);
            uint32_t tamanio_escritura = string_length(escritura)+1;
            io_escritura->pid = io_fs_write->pid;
            io_escritura->output_length = tamanio_escritura;
            io_escritura->output = escritura;
            printf("Tamanio output %d\n",io_escritura->output_length);
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_output(io_escritura ,socket_cliente, IO_FS_WRITE_M);
            log_info(logger_memoria, "Sevidor Memoria envía IO_M_STDOUT_FIN al cliente\n");
            list_clean(valores);
            break;

        case IO_FS_READ: //Lee de un archivo y escribe en memoria
            log_info(logger_memoria, "Recibida IO_FS_READ \n");
            
            valores = recibir_paquete(socket_cliente);         
            input = deserializar_input(valores);        
             
            escribir_memoria(input->pid, (int) input->direcciones_fisicas, input->input, input->input_length);      
            response = IO_FS_READ_M; // termina de escribir
            usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            if (send(socket_cliente, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send IO_FS_READ_M response");
                break;
            }

            log_info(logger_memoria, "Sevidor Memoria envía IO_FS_READ_M al cliente\n");
            list_clean(valores);
            break;

        case -1:
			log_error(logger_memoria, " Cliente se desconectó.");
			//return EXIT_FAILURE;
			break;       
        
        
        default:
			log_warning(logger_memoria,"Operacion desconocida");
			break;  
        
        
        }
    }
}


