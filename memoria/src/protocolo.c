#include <../include/protocolo.h>

//Funcion que atiende la peticion de Kernel segun el cod
void memoria_atender_cliente(void* socket){
     int socket_cliente = *(int*)socket;
    uint32_t cod_op;
	op_code response;
    t_list* valores =  malloc(sizeof(t_list));
    t_io_memo_escritura* input = malloc(sizeof(t_io_memo_escritura));
    //t_paquete* paquete;
	void* output;
     //t_proceso_memoria contexto_ejecucion;    
    //t_paquete* paquete;
    t_proceso_memoria* solicitud_instruccion = malloc(sizeof(t_proceso_memoria));
    t_busqueda_marco* solicitud_marco = malloc(sizeof(t_busqueda_marco));
    t_escribir_leer* peticion_leer = malloc(sizeof(t_io_direcciones_fisicas));
    //t_escribir_leer* peticion_escribir = malloc(sizeof(t_escribir_leer));
    //t_resize* solicitud_resize = malloc(sizeof(t_resize));
    //t_copy* copiar_valor = malloc(sizeof(t_copy));



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
            valores = recibir_paquete(socket_cliente);
            t_m_crear_proceso *iniciar_proceso = deserializar_crear_proceso(valores);
            leer_instrucciones(iniciar_proceso->archivo_pseudocodigo);                  
            crear_proceso(iniciar_proceso->pid);
            //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_respuesta_crear_proceso(iniciar_proceso, socket_cliente);
            break;

		// case FINALIZAR_PROCESO:
		// 	   valores = recibir_paquete(socket_cliente);
        //     t_pcb* finalizar_proceso = deserializar_finalizar_proceso(valores);
		// 	   finalizar_preceso(finalizar_proceso->pid);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
        //     enviar_respuesta_finalizar_proceso(finalizar_proceso, socket_cliente);
        //     break;

/*---------------------------- CPU-------------------------*/
        case PROXIMA_INSTRUCCION:
            valores = recibir_paquete(socket_cliente);
            solicitud_instruccion = deserializar_proxima_instruccion(valores);         
            char* instruccion = buscar_instruccion(solicitud_instruccion->pid, solicitud_instruccion->program_counter);
            log_trace(logger_memoria, "Se Encontro la Instruccion: %s", instruccion);
		//     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_respuesta_instruccion(instruccion, socket_cliente);     
            break;

        case PEDIDO_MARCO_A_MEMORIA:
            valores = recibir_paquete(socket_cliente);
            solicitud_marco = deserializar_solicitud_marco(valores);
            int marco = buscar_marco_pagina(solicitud_marco->pid, solicitud_marco->nro_pagina);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_solicitud_marco(marco, socket_cliente);
            break;


        case SOLICITUD_TAMANIO_PAGINA:
            printf("Envio tamaño de pagina\n");
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_solicitud_tamanio(cfg_memoria->TAM_PAGINA, socket_cliente);
            break;

        case PETICION_VALOR_MEMORIA:
            valores = recibir_paquete(socket_cliente);
            peticion_leer = deserializar_peticion_valor(valores);     
            void* valor = leer_memoria(peticion_leer->pid, peticion_leer->direccion_fisica, peticion_leer->tamanio);          
            log_info(logger_memoria, "PID: %d - Acción: LEER - Direccion fisica: %d - Tamaño: %d", peticion_leer->pid, peticion_leer->direccion_fisica, peticion_leer->tamanio);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);    
            enviar_peticion_valor(valor, socket_cliente);                            
            break;

        // case GUARDAR_EN_DIRECCION_FISICA:
        //     valores = recibir_paquete(socket_cliente);
        //     peticion_escribir = deserializar_peticion_guardar(valores);   
        //     void* vañor = escribir_memoria(peticion_escribir->pid, peticion_escribir->direccion_fisica, peticion_escribir->valor, peticion_escribir->tamanio);
        //     log_info(logger_memoria, "PID: %d - Acción: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", peticion_escribir->pid, peticion_escribir->direccion_fisica), peticion_escribir->tamanio;
        //     free(peticion_escribir);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
        //     enviar_resultado_guardar(valor, socket_cliente);
        //     break;

        // case SOLICITUD_RESIZE:
        //     valores = recibir_paquete(socket_cliente);
        //     solicitud_resize = deserializar_solicitud_resize(valores);
        //     administrar_resize(solicitud_resize->pid, solicitud_resize->tamanio);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
        //     enviar_respuesta_resize(solicitud_resize, socket_cliente);
        //     break;

        // case ENVIO_COPY_STRING_A_MEMORIA:
        //     valores = recibir_paquete(socket_cliente);
        //     copiar_valor = deserializar_solicitud_copy(valores);
        //     copiar_solicitud(copiar_valor->pid, capiar_valor->direccion_fisica, copiar_valor->valor);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
        //     break;
		
/*---------------------------- ENTRADASALIDA-------------------------*/  
        case IO_M_STDIN: // lee de teclado y escribe en memoria
            printf("Recibida IO_M_STDIN \n");
            // Llenamos la lista con los datos recibidos de recibir_paquete 
            valores = recibir_paquete(socket_cliente);

            // Deserializamos los valores de la lista 
            input = deserializar_input(valores);

            if (valores == NULL || list_size(valores) == 0) {
                printf("El paquete vino vacío\n");
                break;
            }        

            escribir_memoria(input->pid, input->direcciones_fisicas, input->input, input->input_length);    //ver
            response = IO_M_STDIN_FIN;
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            if (send(socket_cliente, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send IO_M_STDIN response");
                break;
            }

            printf("Memoria envio IO_M_STDIN_FIN a IO \n");
            list_clean(valores);
            break;           
            
		case IO_M_STDOUT: // Lee de memoria e imprime por pantalla
             printf("Recibida IO_M_STDOUT \n");

            t_io_output* io_output = malloc(sizeof(t_io_output));
            t_io_direcciones_fisicas* io_stdout = malloc(sizeof(t_io_direcciones_fisicas));
                
            valores = recibir_paquete(socket_cliente);
            io_stdout = deserializar_io_df(valores);
        
            if (valores == NULL || list_size(valores) == 0) {
                printf("El paquete vino vacío\n");
                break;
            }

             
            output =  leer_memoria(io_stdout->pid, list_get(io_stdout->direcciones_fisicas,0), io_stdout->tamanio_operacion);
            uint32_t tamanio_output = string_length(output)+1;
            io_output->pid = io_stdout->pid;
            io_output->output_length = tamanio_output;
            io_output->output = output;

            printf("Tamanio output %d\n",io_output->output_length);
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            enviar_output(io_output ,socket_cliente, IO_M_STDOUT_FIN );
            list_clean(valores);    
            break;
        case IO_FS_WRITE://Lee de memoria y escribe en un archivo
            printf("Recibida IO_FS_WRITE \n");

            t_io_output* io_escritura = malloc(sizeof(t_io_output));
            t_io_direcciones_fisicas* io_fs_write = malloc(sizeof(t_io_direcciones_fisicas));
                
                                
            valores = recibir_paquete(socket_cliente);
            io_fs_write = deserializar_io_df(valores);
        
            if (valores == NULL || list_size(valores) == 0) {
                printf("El paquete vino vacío\n");
                break;
            }
            char* escritura = leer_memoria(io_fs_write->pid, list_get(io_fs_write->direcciones_fisicas,0), io_fs_write->tamanio_operacion);     //ver
            
            
            uint32_t tamanio_escritura = string_length(escritura)+1;
            io_escritura->pid = io_fs_write->pid;
            io_escritura->output_length = tamanio_escritura;
            io_escritura->output = escritura;
            printf("Tamanio output %d\n",io_escritura->output_length);
            enviar_output(io_escritura ,socket_cliente, IO_FS_WRITE_M);
            printf("Sevidor Memoria envía IO_M_STDOUT_FIN al cliente\n");
            list_clean(valores);
            break;

        case IO_FS_READ: //Lee de un archivo y escribe en memoria
            printf("Recibida IO_FS_READ \n");
            
            valores = recibir_paquete(socket_cliente);         
            input = deserializar_input(valores);        
             
            escribir_memoria(input->pid, input->direcciones_fisicas, input->input, input->input_length);      
            response = IO_FS_READ_M; // termina de escribir
        //     usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
            if (send(socket_cliente, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send IO_FS_READ_M response");
                break;
            }

            printf("Sevidor Memoria envía IO_FS_READ_M al cliente\n");
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


