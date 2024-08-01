#include "../include/servidor_kernel.h"


void* crearServidor(){
 
    socket_servidor = iniciar_servidor(logger_kernel, "SERVER KERNEL", "127.0.0.0",cfg_kernel->PUERTO_ESCUCHA );
    if (socket_servidor == 0) {
        log_error(logger_kernel, "Fallo al crear el servidor, cerrando KERNEL");
        return EXIT_FAILURE;
    }
    sem_post(&sem_crearServidor);
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

        if (recv(cliente_socket, &cop, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

        switch (cop) {
            
            case HANDSHAKE:
                log_info(logger, "Handshake recibido");
                response = HANDSHAKE_OK;
                if (send(cliente_socket, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                    log_error(logger, "Error al enviar respuesta de handshake a cliente");
                    break;
                }
            
            case HANDSHAKE_OK:

                log_info(logger_kernel, "handshake recibido exitosamente con I/O");
                

            break;
            case INTERFAZ_ENVIAR:
                printf("Received INTERFAZ_ENVIAR request se queda\n");

                t_list* lista_paquete_interfaz = list_create();
                
                lista_paquete_interfaz = recibir_paquete(cliente_socket);

                
                t_interfaz* interfaz_recibida = deserializar_interfaz(lista_paquete_interfaz);
                log_info(logger_kernel, "Deseralizo la interfaz recibida"); //despues borrar
                if (lista_paquete_interfaz == NULL || list_size(lista_paquete_interfaz) == 0) {
                    printf("Failed to receive data or empty list\n");
                    break;
                }
                // armar estructura con la interfaz recibida y cargar al diccionario
                t_interfaz_diccionario* interfaz_nueva = malloc(sizeof(t_interfaz_diccionario));
                interfaz_nueva->nombre = interfaz_recibida->nombre;
                interfaz_nueva->tipo = interfaz_recibida->tipo;
                log_info(logger_kernel, "Cliente socket entradasalida %d",cliente_socket); //despues borrar
                interfaz_nueva->conexion = cliente_socket;
			    
                dictionary_put(interfaces,interfaz_recibida->nombre,interfaz_nueva);
               
                //agrego la nueva cola blocked de la interfaz conectada
                  pthread_mutex_lock(&mutex_cola_blocked);
                dictionary_put(planificador->cola_blocked ,interfaz_recibida->nombre,list_create());
                  pthread_mutex_unlock(&mutex_cola_blocked);
                log_info(logger_kernel,"cola de bloqueados agregada a interfaz %s\n", interfaz_recibida->nombre);//despues borrar
                uint32_t response_interfaz = INTERFAZ_RECIBIDA;
                if (send(cliente_socket, &response_interfaz, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                perror("send INTERFAZ_RECIBIDA response");
                }
                //liberar_memoria_t_interfaz(interfaz_recibida);
                //liberar_memoria_t_interfaz_diccionario(interfaz_nueva);  
                //list_destroy_and_destroy_elements(lista_paquete_interfaz,free);
            break;
            case IO_K_GEN_SLEEP_FIN:
                
                t_list* lista_paquete_interfaz_pid_sleep = recibir_paquete(cliente_socket);
                t_interfaz_pid* interfaz_pid_sleep = deserializar_interfaz_pid(lista_paquete_interfaz_pid_sleep);
                // sem_wait(&sem_interrupcion_atendida);          
                
                 desbloquear_y_agregar_a_ready(planificador,interfaz_pid_sleep->pid, interfaz_pid_sleep->nombre_interfaz);
                 log_info(logger_kernel, "PID: %u - Estado Anterior: BLOQUEADO - Estado Actual: READY", interfaz_pid_sleep->pid); // REPETIR EN TODAS LAS REPSUESTAS DE IO	
 
                //list_destroy_and_destroy_elements(lista_paquete_interfaz_pid_sleep,free);
               // free(interfaz_pid_sleep);
               
            break;
            case IO_K_FS_CREATE_FIN:
                printf("Received IO_K_FS_CREATE_FIN request\n");
                //TODO: deserealizar paquete. nombre interfaz, pid.(LISTO)
                t_list* lista_paquete_interfaz_pid_create = recibir_paquete(cliente_socket);
                t_interfaz_pid* interfaz_pid_create = deserializar_interfaz_pid(lista_paquete_interfaz_pid_create);
                t_pcb* proceso_create = malloc(sizeof(t_pcb));
                proceso_create = buscar_pcb_en_lista_de_data(dictionary_get(planificador->cola_blocked, interfaz_pid_create->nombre_interfaz),interfaz_pid_create->pid); //TODO: crear funcion para encontrar pcb en una lista de t_data

                //desbloquear_y_agregar_a_ready(interfaz_pid_create,proceso_create);
               
                log_info(logger_kernel, "PID: %u - Estado Anterior: BLOQUEADO - Estado Actual: READY", proceso_create->pid); // REPETIR EN TODAS LAS REPSUESTAS DE IO	
                sem_post(&sem_io_fs_libre); 

                list_destroy_and_destroy_elements(lista_paquete_interfaz_pid_create,free);
                liberar_memoria_t_interfaz_pid(interfaz_pid_create);
                liberar_memoria_pcb(proceso_create);
            break;
            case IO_K_FS_DELETE_FIN:
                printf("Received IO_K_FS_DELETE_FIN request\n");
                
                t_list* lista_paquete_interfaz_pid_delete = recibir_paquete(cliente_socket);
                t_interfaz_pid* interfaz_pid_delete = deserializar_interfaz_pid(lista_paquete_interfaz_pid_delete);
                t_pcb* proceso_delete = malloc(sizeof(t_pcb));
                proceso_delete = buscar_pcb_en_lista_de_data(dictionary_get(planificador->cola_blocked, interfaz_pid_delete->nombre_interfaz),interfaz_pid_delete->pid); //TODO: crear funcion para encontrar pcb en una lista de t_data

                //desbloquear_y_agregar_a_ready(interfaz_pid_delete,proceso_delete);
               
                log_info(logger_kernel, "PID: %u - Estado Anterior: BLOQUEADO - Estado Actual: READY", proceso_delete->pid); // REPETIR EN TODAS LAS REPSUESTAS DE IO	
                sem_post(&sem_io_fs_libre);  

                list_destroy_and_destroy_elements(lista_paquete_interfaz_pid_delete,free);
                liberar_memoria_t_interfaz_pid(interfaz_pid_delete);
                liberar_memoria_pcb(proceso_delete);


            break;
            case IO_K_FS_TRUNCATE_FIN:
                printf("Received IO_K_FS_TRUNCATE_FIN request\n");
                
                t_list* lista_paquete_interfaz_pid_truncate = recibir_paquete(cliente_socket);
                t_interfaz_pid* interfaz_pid_truncate = deserializar_interfaz_pid(lista_paquete_interfaz_pid_truncate);
                t_pcb* proceso_truncate = malloc(sizeof(t_pcb));
               

                proceso_truncate = buscar_pcb_en_lista_de_data(dictionary_get(planificador->cola_blocked, interfaz_pid_truncate->nombre_interfaz),interfaz_pid_truncate->pid); //TODO: crear funcion para encontrar pcb en una lista de t_data

                //desbloquear_y_agregar_a_ready(interfaz_pid_truncate,proceso_truncate);
               
                log_info(logger_kernel, "PID: %u - Estado Anterior: BLOQUEADO - Estado Actual: READY", proceso_truncate->pid); // REPETIR EN TODAS LAS REPSUESTAS DE IO	
                sem_post(&sem_io_fs_libre);  

                list_destroy_and_destroy_elements(lista_paquete_interfaz_pid_truncate,free);
                liberar_memoria_t_interfaz_pid(interfaz_pid_truncate);
                liberar_memoria_pcb(proceso_truncate);

            break;
            case IO_K_FS_READ_FIN:
                printf("Received IO_K_FS_READ_FIN request\n");
                
                t_list* lista_paquete_interfaz_pid_read = recibir_paquete(cliente_socket);
                t_interfaz_pid* interfaz_pid_read = deserializar_interfaz_pid(lista_paquete_interfaz_pid_read);
                t_pcb* proceso_read = malloc(sizeof(t_pcb));
                proceso_read = buscar_pcb_en_lista_de_data(dictionary_get(planificador->cola_blocked, interfaz_pid_read->nombre_interfaz),interfaz_pid_read->pid); //TODO: crear funcion para encontrar pcb en una lista de t_data

                //desbloquear_y_agregar_a_ready(interfaz_pid_read,proceso_read);
               
                log_info(logger_kernel, "PID: %u - Estado Anterior: BLOQUEADO - Estado Actual: READY", proceso_read->pid); // REPETIR EN TODAS LAS REPSUESTAS DE IO	
                sem_post(&sem_io_fs_libre); 

                list_destroy_and_destroy_elements(lista_paquete_interfaz_pid_read,free);
                liberar_memoria_t_interfaz_pid(interfaz_pid_read);
                liberar_memoria_pcb(proceso_read); 

            break;
            case IO_K_FS_WRITE_FIN:
                printf("Received IO_K_FS_WRITE_FIN request\n");
                
                t_list* lista_paquete_interfaz_pid_write = recibir_paquete(cliente_socket);
                t_interfaz_pid* interfaz_pid_write = deserializar_interfaz_pid(lista_paquete_interfaz_pid_write);
                t_pcb* proceso_write = malloc(sizeof(t_pcb));
                proceso_write = buscar_pcb_en_lista_de_data(dictionary_get(planificador->cola_blocked, interfaz_pid_write->nombre_interfaz),interfaz_pid_write->pid); //TODO: crear funcion para encontrar pcb en una lista de t_data

                //desbloquear_y_agregar_a_ready(interfaz_pid_write,proceso_write);
               
                log_info(logger_kernel, "PID: %u - Estado Anterior: BLOQUEADO - Estado Actual: READY", proceso_write->pid); // REPETIR EN TODAS LAS REPSUESTAS DE IO	
                sem_post(&sem_io_fs_libre);  

                list_destroy_and_destroy_elements(lista_paquete_interfaz_pid_write,free);
                liberar_memoria_t_interfaz_pid(interfaz_pid_write);
                liberar_memoria_pcb(proceso_write);

            break;
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
            
            }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    log_destroy(logger);
    return;
}






t_pcb* buscar_pcb_en_lista_de_data(t_list* lista_de_data, uint32_t pid){
   t_proceso_data* pcb_data_de_lista = malloc(sizeof(t_proceso_data));
   for (uint32_t i = 0; i < lista_de_data->elements_count; i++)
   {
      pcb_data_de_lista = list_get(lista_de_data,i);
      if(pcb_data_de_lista->pcb->pid == pid){
         return pcb_data_de_lista->pcb;
      }
   }
   
   return NULL;
}

t_interfaz_pid* deserializar_interfaz_pid(t_list*  lista_paquete ){
    t_interfaz_pid* interfaz_pid = malloc(sizeof(t_interfaz_pid));
    interfaz_pid->pid= *(uint32_t*)list_get(lista_paquete, 0);
    interfaz_pid->nombre_length= *(uint32_t*)list_get(lista_paquete, 1);
    interfaz_pid->nombre_interfaz = list_get(lista_paquete, 2);
    
	return interfaz_pid;
}

void desbloquear_y_agregar_a_ready(t_planificador* planificador,int pid,char* nombre_interfaz){
            log_info(logger_kernel, "PLANIFICADOR ALGORITMO %d", planificador->algoritmo);
            pthread_mutex_lock(&mutex_cola_blocked);
             t_pcb* proceso =  desbloquear_proceso_io(planificador,pid,nombre_interfaz);
            pthread_mutex_unlock(&mutex_cola_blocked);    
               if  ( planificador->algoritmo == VIRTUAL_ROUND_ROBIN ) {
                    if  (proceso->tiempo_ejecucion != cfg_kernel->QUANTUM) {
                        pthread_mutex_lock(&mutex_cola_ready_prioridad);
                        log_info(logger_kernel, "me meti en  if vrr %d", planificador->algoritmo);
                        list_add(planificador->cola_ready_prioridad, proceso); // como todavia le queda por ejecutar se asigna a la cola de prioridad
                        pthread_mutex_unlock(&mutex_cola_ready_prioridad);
                        sem_post(&sem_prioridad_io);
                    }
                }else {
                        pthread_mutex_lock(&mutex_cola_ready);
                        list_add(planificador->cola_ready, proceso);                     
                        pthread_mutex_unlock(&mutex_cola_ready);
                        sem_post(&sem_prioridad_io);
                }	 
    
}

t_pcb * desbloquear_proceso_io(t_planificador* planificadorloco,int pid,char* nombre_interfaz){
    
    t_list*list_io = dictionary_get(planificadorloco->cola_blocked,nombre_interfaz);
    int indice = encontrar_indice_proceso_data_por_pid(list_io,pid);
    t_proceso_data*  proceso_desbloqueado =  list_remove(list_io,indice); // en teoria el header de la lista es el proceso bloqueado actual
    dictionary_put(planificadorloco->cola_blocked,nombre_interfaz,list_io); // actualizo la lista

    return proceso_desbloqueado->pcb;
    
}