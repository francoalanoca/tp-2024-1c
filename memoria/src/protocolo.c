#include <../include/protocolo.h>

/*
	while (1) {
		int cliente_fd = esperar_cliente(logger_memoria, nombre, fd_memoria);
        //inicio handshake
        size_t bytes;

        int32_t handshake;
        int32_t resultOk = 0;
        int32_t resultError = -1;

        bytes = recv(cliente_fd, &handshake, sizeof(int32_t), MSG_WAITALL);
        if (handshake == HANDSHAKE) {
            bytes = send(cliente_fd, &resultOk, sizeof(int32_t), 0);
			flag_conexion_activa = true;
			log_info(logger_memoria, "Handshake salió bien, un gusto");
        } else {
            bytes = send(cliente_fd, &resultError, sizeof(int32_t), 0);
			flag_conexion_activa = false;
			log_warning(logger_memoria, "Handshake salió mal, no te conozco");
        }
        //fin handshake

        t_list* lista;

		while (flag_conexion_activa) {
			int cod_op = recibir_operacion(cliente_fd);
			switch (cod_op) {
			//case PROXIMA_INSTRUCCION://CPU me pide su sig instruccion
				//abrir archivo de instrucciones (buscar como leer un archivo linea por linea)
				//ubicar instruccion a partir PC que me envia cpu
				//enviar codigo de operacion (INSTRUCCION_RECIBIDA) a cpu para que espere la instruccion(armar paquete con instruccion y codigo de opercion nuevo)
				break;
			// case INTERFAZ_IO://interfaz de io que me tiene que pasar
			// 	lista = recibir_paquete(cliente_fd);
			// 	log_info(logger, "Me llegaron los siguientes valores:\n");
			// 	list_iterate(lista, (void*) iterator);
			// 	break;
			case -1:
				log_error(logger_memoria, "Fallo en el envío de paquete");
				close(cliente_fd); // Cerrar el descriptor del cliente desconectado
				flag_conexion_activa = false;
			default:
				log_warning(logger_memoria,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}

	}
	*/



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
        // case CREAR_PROCESO_KERNEL:
        //     un_buffer = recibir_paquete(fd_kernel);
        //     atender_crear_proceso(un_buffer);
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
        // case PROXIMA_INSTRUCCION:
        //     paquete = recibir_paquete(fd_cpu);
        //     eliminar_paquete(paquete);       //libero el paquete para luego enviar la instruccion en su contenido
        //     enviar_instruccion(contexto_ejecucion, paquete);     //serializo la insctruccion y la envio
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



void memoria_atender_io(){
    t_list* lista;
    //t_buffer* un_buffer;
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
        // case CREAR_PROCESO_KERNEL:
        //     un_buffer = recibir_paquete(fd_entradasalida);
        //     atender_crear_proceso(un_buffer);
        //     break;
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