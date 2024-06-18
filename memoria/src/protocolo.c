#include <../include/protocolo.h>




//Funcion que crea hilos para cada modulo y los va atendiendo
void escuchar_modulos(){
    //Atender a Kernel
    pthread_t hilo_kernel;
    //se crea un nuevo hilo que atiende al cliente
    pthread_create(&hilo_kernel, NULL, (void*) memoria_atender_kernel, NULL);
    //se desacopla del hilo principal para no interferir
    pthread_detach(hilo_kernel);

    //Atender a Cpu
    pthread_t hilo_cpu;
    //se crea un nuevo hilo que atiende al cliente
    pthread_create(&hilo_cpu, NULL, (void*) memoria_atender_cpu, NULL);
    //se desacopla del hilo principal para no interferir
    pthread_detach(hilo_cpu);

    //Atender a EntradaSalida
    pthread_t hilo_entradasalida;
    //se crea un nuevo hilo que atiende al cliente
    pthread_create(&hilo_entradasalida, NULL, (void*)memoria_atender_io, NULL);
    //caundo el hilo principal llega a join este se detiene hasta que se termine el hilo actual en lugar de deacoplarse
    pthread_join(hilo_entradasalida, NULL);
}






//Funcion que atiende la peticion de Kernel segun el cod
void memoria_atender_kernel(){
    
    
	op_code response;
    t_list* valores =  malloc(sizeof(t_list));


	while (1) {
        //Se queda esperando a que KErnel le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
		
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con Kernel");
            response = HANDSHAKE_OK;
            if (send(fd_entradasalida, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a kernel");
                   
                break;
            }
            break;

        // case CREAR_PROCESO_KERNEL:
        //     valores = recibir_paquete(fd_kernel);
        //     t_m_crear_proceso *iniciar_proceso = deserializar_crear_proceso(valores);
        //     leer_instrucciones(iniciar_proceso->archivo_pseudocodigo);                  //No corre ver
        //     crear_proceso(iniciar_proceso->pcb->pid, iniciar_proceso->tamanio);

        //     enviar_respuesta_crear_proceso(iniciar_proceso, fd_kernel);
        //     break;

		// case FINALIZAR_PROCESO:
		// 	valores = recibir_paquete(fd_kernel);
        //     t_pcb* finalizar_proceso = deserializar_finalizar_proceso(valores);

		// 	finalizar_preceso(finalizar_proceso->pid);
        //     enviar_respuesta_finalizar_proceso(finalizar_proceso, fd_kernel);
        //     break;

		case -1:
			log_error(logger_memoria, "Kernel se desconecto. Terminando servidor.");
			//return EXIT_FAILURE;
			break;
		default:
			log_warning(logger_memoria,"Operacion desconocida de Kernel.");
			break;
		}
    }
}


//Funcion que atiende la peticion de Cpu segun el cod
void memoria_atender_cpu(){

    //t_proceso_memoria contexto_ejecucion;
    
    //t_paquete* paquete;
	op_code response;
    t_list* valores =  malloc(sizeof(t_list));


	while (1) {

        //contexto_ejecucion = malloc(sizeof(t_proceso_memoria));
        //Se queda esperando a que Cpu le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
		
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con Kernel");
            response = HANDSHAKE_OK;
            if (send(fd_entradasalida, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a kernel");
                   
                break;
            }
            break;

        case PROXIMA_INSTRUCCION:
            valores = recibir_paquete(fd_cpu);
            t_proceso_memoria* proxima_instruccion = deserializar_proxima_instruccion(valores);         
            char* instruccion = buscar_instruccion(proxima_instruccion->pid, proxima_instruccion->program_counter);
            log_trace(logger_memoria, "Se Encontro la Instruccion: %s", instruccion);
        
		//     usleep(cfg_memoria->RETARDO_RESPUESTA);
            enviar_respuesta_instruccion(proxima_instruccion, fd_cpu);     
            break;


        // case PETICION_VALOR_MEMORIA:
        //     valores = recibir_paquete(fd_cpu);
        //     int direccion_fisica = deserializar_peticion_valor(valores);     //ver
        //     void* valor = leer_memoria(direccion_fisica);
        //     log_info(logger_memoria, "PID: %d - Acción: LEER - Direccion fisica: %d", leer_de_memoria->pcb->pid, leer_de_memoria->direccion_fisica);
        //     enviar_peticion_valor(valor, fd_cpu);                            //ver
        //     break;

        // case GUARDAR_EN_DIRECCION_FISICA:
        //     valores = recibir_paquete(fd_cpu);
        //     t_list* peticion_guardar = deserializar_peticion_guardar(valores);   //ver
        //     escribir_memoria(peticion_guardar->pid, peticion_guardar->direccion_fisica, peticion_guardar->valor, peticion_guardar->tamanio);
        //     log_info(logger_memoria, "PID: %d - Acción: ESCRIBIR - Direccion fisica: %d", peticion_guardar->pcb->pid, peticion_guardar->direccion_fisica);
        //     free(peticion_guardar);
        //     break;

        // case SOLICITUD_RESIZE:
        //     valores = recibir_paquete(fd_cpu);
        //     t_list* solicitud_resize = deserializar_solicitud_resize(valores);
        //     administrar_resize(solicitud_resize);
        //     enviar_respuesta_resize(solicitud_resize, fd_cpu);
        //     break;

        // case ENVIO_COPY_STRING_A_MEMORIA:
        //     valores = recibir_paquete(fd_cpu);
        //     t_list* copiar_valor = deserializar_copiar_valor(valores);
        //     copiar_solicitud(copiar_valor);
        //     break;

		case -1:
			log_error(logger_memoria, "CPU se desconecto. Terminando servidor.");
			//return EXIT_FAILURE;
			break;
		default:
			log_warning(logger_memoria,"Operacion desconocida de CPU.");
			break;
		}
    }
}



//Funcion que atiende la peticion de i/O segun el cod
void memoria_atender_io(){
    
	t_list* valores =  malloc(sizeof(t_list));
    //t_paquete* paquete;
	op_code response;
	uint32_t request_type;


	while (1) {
        //Se queda esperando a que IO le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_entradasalida);
		switch (cod_op) {
		
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con Kernel");
            response = HANDSHAKE_OK;
            if (send(fd_entradasalida, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a kernel");
                   
                break;
            }
            break;

        case IO_M_STDIN:

            printf("Solicitud IO_M_STDIN recibida\n");

            // Llenamos la lista con los datos recibidos de recibir_paquete 
            valores = recibir_paquete(fd_entradasalida);

            // Deserializamos los valores de la lista 
            deserializar_input(valores);
        
            if (valores == NULL || list_size(valores) == 0) {
                printf("Failed to receive data or empty list\n");
                break;
            }
            printf("despues de recbir paquete\n");
            request_type = (uint32_t)(uintptr_t)list_get(valores, 1);

            uint32_t response_interfaz = IO_M_STDIN_FIN;
            if (send(fd_entradasalida, &response_interfaz, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send INTERFAZ_RECIBIDA response");
                break;
            }

            printf("Memoria envio IO_M_STDIN_FIN a IO \n");
            break;


		case IO_M_STDOUT:

            printf("Solicitud IO_M_STDOUT recibida\n");

            t_io_output* io_output = malloc(sizeof(t_io_input));
            t_io_direcciones_fisicas* io_stdout = malloc(sizeof(t_io_direcciones_fisicas));
                
                                
            valores = recibir_paquete(fd_entradasalida);
            io_stdout = deserializar_io_df(valores);
        
            if (valores == NULL || list_size(valores) == 0) {
                printf("Failed to receive data or empty list\n");
                break;
            }

            printf("despues de recbir paquete\n");
            char* output = "LA PUERCA ESTA EN LA POCILGA";
            uint32_t tamanio_output = string_length(output)+1;
            io_output->pid = io_stdout->pid;
            io_output->output_length = tamanio_output;
            io_output->output = output;

            printf("Tamanio output %d\n",io_output->output_length);
            enviar_output(io_output ,fd_entradasalida);

            printf("Memoria envio IO_M_STDOUT_FIN a IO\n");
            break;

		case -1:
			log_error(logger_memoria, "Entrada/salida se desconecto. Terminando servidor.");
			//return EXIT_FAILURE;
			break;
		default:
			log_warning(logger_memoria,"Operacion desconocida de Entrada/salida.");
			break;
		}
    }
}