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
    t_list* lista;
    //t_buffer* un_buffer;
	while (1) {
        //Se queda esperando a que KErnel le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel); 
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_kernel);
			log_info(logger_memoria, "Me llegaron los siguientes valores:\n");
			//list_iterate(lista, (void*) iterator);
			break;
		// case HANDSHAKE:
		// 	enviar_paquete(paquete, fd_kernel);
		// 	break;
        // case CREAR_PROCESO_KERNEL:
        //     paquete = recibir_paquete(fd_kernel);
        //     atender_crear_proceso(un_buffer);
        //     break;
		// case FINALIZAR_PROCESO:
		// 	paquete = recibir_paquete(fd_kernel);
		// 	atender_finalizar_preceso(paquete);
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
    t_list* lista;
    //t_paquete* paquete;


	while (1) {

        //contexto_ejecucion = malloc(sizeof(t_proceso_memoria));
        //Se queda esperando a que Cpu le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_cpu);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_cpu);
			log_info(logger_memoria, "Me llegaron los siguientes valores:\n");
			//list_iterate(lista, (void*) iterator);
			break;
		// case HANDSHAKE:
		// 	enviar_paquete(paquete, fd_cpu);
		// 	break;
        // case PROXIMA_INSTRUCCION:
        //     paquete = recibir_paquete(fd_cpu);
        //     eliminar_paquete(paquete);       //libero el paquete para luego enviar la instruccion en su contenido
		//	   unsleep(cfg_memoria->RETARDO_RESPUESTA);
        //     enviar_instruccion(contexto_ejecucion, paquete);     //serializo la insctruccion y la envio
        //     break;


		//case PROXIMA_INSTRUCCION://CPU me pide su sig instruccion
				//abrir archivo de instrucciones (buscar como leer un archivo linea por linea)
				//ubicar instruccion a partir PC que me envia cpu
				//enviar codigo de operacion (INSTRUCCION_RECIBIDA) a cpu para que espere la instruccion(armar paquete con instruccion y codigo de opercion nuevo)


		// case AMPLIAR_PROCESO:
		// 	paquete = recibir_paquete(fd_cpu);
		// 	ampliar_proceso();
		// 	break;
		// case REDUCIR_PROCESO:
		// 	paquete = recibir_paquete(fd_cpu);
		// 	reducir_proceso();
		// 	break;
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
    t_list* lista;
    //t_paquete* paquete;
	while (1) {
        //Se queda esperando a que IO le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_entradasalida);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_entradasalida);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_entradasalida);
			log_info(logger_memoria, "Me llegaron los siguientes valores:\n");
			//list_iterate(lista, (void*) iterator);
			break;
		// case HANDSHAKE:
		// 	enviar_paquete(paquete, fd_entradasalida);
		// 	break;
        // case IO_M_STDIN_READ:
        //     paquete = recibir_paquete(fd_entradasalida);
        //     guardar_datos(paquete);
		//	   enviar_paquete(paquete, fd_entradasalida);
        //     break;
		// case IO_M_STDOUT_WRITE:
		// 	paquete = recibir_paquete(fd_entradasalida);
		// 	buscar_datos(paquete);
		// 	enviar_paquete(paquete, fd_entradasalida);
		// 	break;
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