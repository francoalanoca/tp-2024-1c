
#include "../include/protocolo_kernel.h"

//sem_t *sem_planificar;
t_pcb* pcb_actualizado_interrupcion;


void Escuchar_Msj_De_Conexiones(){
   sem_wait(&sem_crearServidor);
   log_info(logger_kernel, "ENTRO EN INICIAR CONEXIONES : %d\n",conexion_memoria); 
   conexion_memoria = crear_conexion(logger_kernel, "MEMORIA", cfg_kernel->IP_MEMORIA, cfg_kernel->PUERTO_MEMORIA);    
   log_info(logger_kernel, "Socket de MEMORIA : %d\n",conexion_memoria); 
   pthread_t hilo_kernel_memoria;
   pthread_create(&hilo_kernel_memoria, NULL, (void*)Kernel_escuchar_memoria, &conexion_memoria);
  

//Escuchar los msj de cpu - dispatch
   conexion_cpu_dispatch = crear_conexion(logger_kernel, "CPU", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_DISPATCH);    
   log_info(logger_kernel, "Socket de CP DISPATCH : %d\n",conexion_cpu_dispatch);  
   pthread_t hilo_cpu_dispatch;
   t_kernel_escuchar_cpu* params = malloc(sizeof(t_kernel_escuchar_cpu));
   params->conexion_cpu_dispatch = &conexion_cpu_dispatch;
   params->conexion_cpu_interrupt = &conexion_cpu_interrupt;
   pthread_create(&hilo_cpu_dispatch, NULL, (void*)Kernel_escuchar_cpu_dispatch,  (void*) params);
  

//Escuchar los msj de cpu - interrupt
   conexion_cpu_interrupt = crear_conexion(logger_kernel, "CPU", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_INTERRUPT);    
   log_info(logger_kernel, "Socket de CPU INTERRUP : %d\n",conexion_cpu_interrupt);
 pthread_t hilo_cpu_interrupt;
   pthread_create(&hilo_cpu_interrupt, NULL, (void*)Kernel_escuchar_cpu_interrupt,&conexion_cpu_interrupt);
   sem_post(&sem_EscucharMsj);
   pthread_detach(hilo_cpu_dispatch);
   pthread_detach(hilo_kernel_memoria);
   pthread_detach(hilo_cpu_interrupt); //, NULL);

}


void Kernel_escuchar_cpu_dispatch(void* args){
    t_kernel_escuchar_cpu* params = (t_kernel_escuchar_cpu*) args;
    int socket_dispatch = *(params->conexion_cpu_dispatch);
    int socket_interrupt = *(params->conexion_cpu_interrupt);

bool control_key = 1;
t_list* lista_paquete;
/*while (control_key)
{
   op_code cod_op = recibir_operacion(conexion_cpu_dispatch);*/
   uint32_t cod_op;
    while (socket_dispatch != -1) {

        if (recv(socket_dispatch, &cod_op, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
            log_info(logger_kernel, "DISCONNECT! DISPATCH");
            return;
        }
  
   switch (cod_op)
   {
   case INTERRUPCION_CPU:
      //Recibo t_proceso_interrumpido(pcb y motivo) desde cpu(funcion check_interrupt)
      log_info(logger_kernel,"Recibo INTERRUPCION_CPU desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_proceso_interrumpido* proceso_interrumpido = deserializar_proceso_interrumpido(lista_paquete);
       log_info(logger_kernel, "PROCESO INTERRUMPIDO PID %u ", proceso_interrumpido->pcb->pid);
      //Detecto motivo de interrupcion y dependiendo de este se decide que es lo que se hace 
       if  ( planificador->planificacion_detenida == false ) {  
               switch (proceso_interrumpido->motivo_interrupcion)
                  {
                  case INTERRUPCION_OUT_OF_MEMORY:
                     if(proceso_interrumpido->pcb != NULL){
                        poner_en_cola_exit(proceso_interrumpido->pcb);                   
                     
                        log_info(logger_kernel, "Finaliza el proceso %u - Motivo: OUT_OF_MEMORY ", proceso_interrumpido->pcb->pid);
                     }
                     else{
                        log_info(logger_kernel,"El proceso recibido es nulo");
                     }
                  case INSTRUCCION_EXIT:
                     log_info(logger_kernel,"EXIT RECIBIDO");
                     if(proceso_interrumpido->pcb != NULL){
                        poner_en_cola_exit(proceso_interrumpido->pcb); 
                        sem_post(&sem_contexto_ejecucion_recibido);                    
                        log_info(logger_kernel, "Finaliza el proceso %u - Motivo: SUCCESS", proceso_interrumpido->pcb->pid);
                     }
                     else{
                        log_info(logger_kernel,"El proceso recibido es nulo");
                     }
                     break;
                  
                  case FIN_QUANTUM:
                     printf("Se recibió una interrupción de FIN DE QUANTUM");  
                     if(proceso_interrumpido->pcb != NULL){
                        log_info(logger_kernel, "PID: %U - Desalojado por fin de Quantum", proceso_interrumpido->pcb->pid); 
                        log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: READY", proceso_interrumpido->pcb->pid);
                        pthread_mutex_lock(&mutex_cola_ready);
                        list_add(planificador->cola_ready,  proceso_interrumpido->pcb); // envolver con mutex 
                        pthread_mutex_unlock(&mutex_cola_ready);
                        free(proceso_interrumpido);// crear funcion para liberar pcb
                     }         
                     break;

                  
                  case ELIMINAR_PROCESO:
                     pcb_actualizado_interrupcion = proceso_interrumpido->pcb;
                     sem_post(&sem_contexto_ejecucion_recibido);
                     break;
                  case INTERRUPCION_IO:
                     log_info(logger_kernel, "Se recibió una interrupción de IO");  
                        if(proceso_interrumpido->pcb != NULL){
                           pthread_mutex_lock(&mutex_cola_blocked);
                           actualizar_pcb_proceso_bloqueado(planificador,proceso_interrumpido->interfaz,proceso_interrumpido->pcb) ;
                           pthread_mutex_unlock(&mutex_cola_blocked);
                           sem_post(&sem_interrupcion_atendida); // solo para actualizaar el contexto
                           log_info(logger_kernel, "Contexto actualizado pid %d;",proceso_interrumpido->pcb->pid);  
                           }
                        else{
                           log_info(logger_kernel,"El proceso recibido es nulo");
                        }      



                  
                  default:
                     //printf("Motivo de interrupcion desconocido. Se finaliza el proceso");
                     //mandar_proceso_a_finalizar(proceso_interrumpido->pcb->pid);
                     
                     break;
                  }
                  //liberar_proceso_interrumpido(proceso_interrumpido); hay que liberarlo?

                  break;

      } // fin de if de detener planificacion
   case SOLICITUD_IO_GEN_SLEEP:
      //Recibo PID, interfaz y unidades de trabajo de cpu, debo pedir a kernel que realice la instruccion IO_GEN_SLEEP (comprobar interfaz en diccionaro de interfaces antes)         
      
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_gen_sleep* io_gen_sleep = deserializar_io_gen_sleep(lista_paquete);
      log_info(logger_kernel,"OPERAR INTERFAZ %s",io_gen_sleep->nombre_interfaz);
      enviar_interrupcion_a_cpu(io_gen_sleep->pid,INTERRUPCION_IO,io_gen_sleep->nombre_interfaz,conexion_cpu_interrupt);
      //Verifico que la interfaz exista y este conectada
      if(dictionary_has_key(interfaces,io_gen_sleep->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = dictionary_get(interfaces,io_gen_sleep->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_GEN_SLEEP)){
            
          
            int valor_sem;
            sem_getvalue(&sem_io_fs_libre, &valor_sem);
             log_info(logger_kernel,"VALOR SEMAFORO sem_io_fs_libre %d",valor_sem);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_io_gen_sleep = malloc(sizeof(t_proceso_data));
            proceso_data_io_gen_sleep->op = IO_K_GEN_SLEEP;
            pthread_mutex_lock(&mutex_cola_exec);
             log_info(logger_kernel, "me meto en mutex: "); 
            proceso_data_io_gen_sleep->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_gen_sleep->pid);
            pthread_mutex_unlock(&mutex_cola_exec);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_espera* io_espera_a_bloquear = malloc(sizeof(t_io_espera));
            io_espera_a_bloquear->pid = io_gen_sleep->pid;
            io_espera_a_bloquear->tiempo_espera = io_gen_sleep->unidades_de_trabajo;
            proceso_data_io_gen_sleep->data = io_espera_a_bloquear;
            
            
            //sem_wait(&sem_interrupcion_atendida);// se traba el hilo a si mismo
            if(cronometro != NULL){
               if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
               
                  actualizar_quantum(proceso_data_io_gen_sleep->pcb);
               } 
            }
               log_info(logger_kernel, "INTERFAZ: %s",interfaz_encontrada->nombre); 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_io_gen_sleep->pcb->pid); // LOG OBLIGATORIO
            pthread_mutex_lock(&mutex_cola_blocked);          
            bloquear_proceso(planificador,proceso_data_io_gen_sleep,interfaz_encontrada->nombre);
          
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
             sem_wait(&sem_cpu_libre);
            t_list* lista_bloqueo = dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre);
            t_proceso_data* a_enviar_a_io_gen_sleep = list_get(lista_bloqueo,0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
              pthread_mutex_unlock(&mutex_cola_blocked);
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            log_info(logger_kernel,"Fd entradasalida %d",interfaz_encontrada->conexion);
            enviar_espera(io_espera_a_bloquear,interfaz_encontrada->conexion);
            pthread_mutex_unlock(&mutex_envio_io);

            //liberar_memoria_t_proceso_data(proceso_data_io_gen_sleep);
            //liberar_memoria_t_io_espera(io_espera_a_bloquear);
            //liberar_memoria_t_proceso_data(a_enviar_a_io_gen_sleep);
            
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_gen_sleep->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_gen_sleep->pid);
         }
         //liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_gen_sleep->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_gen_sleep->pid);
      }

     // replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_gen_sleep->pid));
      
      //liberar_memoria_t_io_gen_sleep(io_gen_sleep);

       log_info(logger_kernel, "FIN DE ENVIAR SLEEP GEN");
      break;

   case ENVIO_WAIT_A_KERNEL:
      //Recibo t_recurso(pcb y nombre recurso) desde cpu, debo asignar una instancia del recurso al proceso(verificar recursos disponibles)
      log_info(logger_kernel,"Recibo ENVIO_WAIT_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_recurso* recurso_recibido_wait = malloc(sizeof(t_recurso));
      recurso_recibido_wait = deserializar_recurso(lista_paquete);
      uint32_t indice_recurso_wait = buscar_indice_recurso(cfg_kernel->RECURSOS,recurso_recibido_wait->nombre_recurso);
      if(indice_recurso_wait != NULL){
         uint32_t valor_indice_recurso = list_get(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_wait);
         if(valor_indice_recurso != 0){
            list_replace(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_wait,valor_indice_recurso - 1);
            if(dictionary_has_key(procesos_recursos,recurso_recibido_wait->pcb->pid)){//El proceso ya tenia instancias de ese recurso
               //en el id correspondiente sumar uno a la instancia del recurso correspondiente
               char* pid_string = malloc(sizeof(recurso_recibido_wait->pcb->pid));
               pid_string = sprintf(pid_string, "%u", recurso_recibido_wait->pcb->pid);
               t_proceso_recurso_diccionario* registro_actual_diccionario = malloc(sizeof(t_proceso_recurso_diccionario));
               registro_actual_diccionario = dictionary_get(procesos_recursos,pid_string);
               uint32_t indice_recurso_buscado = buscar_indice_recurso(registro_actual_diccionario->nombres_recursos,recurso_recibido_wait->nombre_recurso);
               uint32_t valor_instancias_actual = list_get(registro_actual_diccionario->instancias_recursos,indice_recurso_buscado);
               list_replace(registro_actual_diccionario->instancias_recursos,indice_recurso_buscado,valor_instancias_actual + 1);
               dictionary_remove_and_destroy(procesos_recursos,pid_string,free);//TODO: reemplazar free por funcion que borre la esttructura y las listas que lo componen
               dictionary_put(procesos_recursos,pid_string,registro_actual_diccionario);
               free(pid_string);
            }
            else{//El proceso no tenia instancias de ese recurso
               //crear id nuevo y agregar a lista el recurso con instancia en 1
               char* pid_string = malloc(sizeof(recurso_recibido_wait->pcb->pid));
               pid_string = sprintf(pid_string, "%u", recurso_recibido_wait->pcb->pid);
               t_proceso_recurso_diccionario* registro_actual_diccionario = malloc(sizeof(t_proceso_recurso_diccionario));
               registro_actual_diccionario = dictionary_get(procesos_recursos,pid_string);
               list_add(registro_actual_diccionario->nombres_recursos,pid_string);
               list_add(registro_actual_diccionario->instancias_recursos,1);
               dictionary_remove_and_destroy(procesos_recursos,pid_string,free);//TODO: reemplazar free por funcion que borre la esttructura y las listas que lo componen
               dictionary_put(procesos_recursos,pid_string,registro_actual_diccionario);
               free(pid_string);
               liberar_memoria_t_proceso_recurso_diccionario(registro_actual_diccionario);
            }
         }
         else{
            //NO HAY INSTANCIAS DISPONIBLES, AGREAGAR A COLA DE BLOQUEADOS DEL RECURSO
            bloquear_proceso(planificador,recurso_recibido_wait->pcb,recurso_recibido_wait->nombre_recurso); 
            //replanificar_y_ejecutar(recurso_recibido_wait->pcb);
         }
         
      }
      else{
         //NO EXISTE RECURSO, MANDAR A EXIT
         mandar_proceso_a_finalizar(recurso_recibido_wait->pcb->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_RESOURCE  ", recurso_recibido_wait->pcb->pid);
        // replanificar_y_ejecutar(recurso_recibido_wait->pcb);
      }
      liberar_memoria_t_recurso(recurso_recibido_wait);
      break;
   case ENVIO_SIGNAL_A_KERNEL:
      //Recibo t_recurso(pcb y nombre recurso) desde cpu, debo liberar una instancia del recurso al proceso(verificar recursos disponibles)
      log_info(logger_kernel,"Recibo ENVIO_SIGNAL_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_recurso* recurso_recibido_signal = malloc(sizeof(t_recurso));
      recurso_recibido_signal = deserializar_recurso(lista_paquete);
      uint32_t indice_recurso_signal = buscar_indice_recurso(cfg_kernel->RECURSOS,recurso_recibido_signal->nombre_recurso);
      if(indice_recurso_signal != NULL){
         uint32_t valor_indice_recurso = list_get(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_signal);
         list_replace(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_signal,valor_indice_recurso + 1);

            if(dictionary_has_key(procesos_recursos,recurso_recibido_signal->pcb->pid)){//El proceso ya tenia instancias de ese recurso
               //en el id correspondiente sumar uno a la instancia del recurso correspondiente
               t_proceso_recurso_diccionario* registro_actual_diccionario = malloc(sizeof(t_proceso_recurso_diccionario));
               registro_actual_diccionario = dictionary_get(procesos_recursos,recurso_recibido_signal->pcb->pid);
               uint32_t indice_recurso_buscado = buscar_indice_recurso(registro_actual_diccionario->nombres_recursos,recurso_recibido_signal->nombre_recurso);
               uint32_t valor_instancias_actual = list_get(registro_actual_diccionario->instancias_recursos,indice_recurso_buscado);
               list_replace(registro_actual_diccionario->instancias_recursos,indice_recurso_buscado,valor_instancias_actual - 1);
               char* pid_string = malloc(sizeof(recurso_recibido_signal->pcb->pid));
               pid_string = sprintf(pid_string, "%u", recurso_recibido_signal->pcb->pid);
               dictionary_remove_and_destroy(procesos_recursos,pid_string,free);//TODO: reemplazar free por funcion que borre la esttructura y las listas que lo componen
               dictionary_put(procesos_recursos,pid_string,registro_actual_diccionario);
               free(pid_string);
               liberar_memoria_t_proceso_recurso_diccionario(registro_actual_diccionario);
            }
            else{//El proceso no tenia instancias de ese recurso
               //crear id nuevo y agregar a lista el recurso con instancia en 1
               t_proceso_recurso_diccionario* registro_actual_diccionario = malloc(sizeof(t_proceso_recurso_diccionario));
               registro_actual_diccionario = dictionary_get(procesos_recursos,recurso_recibido_signal->pcb->pid);
               char* pid_string = malloc(sizeof(recurso_recibido_signal->pcb->pid));
               pid_string = sprintf(pid_string, "%u", recurso_recibido_signal->pcb->pid);
               list_add(registro_actual_diccionario->nombres_recursos,pid_string);
               list_add(registro_actual_diccionario->instancias_recursos,0);
               dictionary_remove_and_destroy(procesos_recursos,pid_string,free);//TODO: reemplazar free por funcion que borre la esttructura y las listas que lo componen
               dictionary_put(procesos_recursos,pid_string,registro_actual_diccionario);
               free(pid_string);
               liberar_memoria_t_proceso_recurso_diccionario(registro_actual_diccionario);
            }
         

         t_list* lista_bloqueados_correspondiente = dictionary_get(planificador->cola_blocked,recurso_recibido_signal->nombre_recurso);
         if(lista_bloqueados_correspondiente->elements_count > 0){
            uint32_t indice_primer_valor = buscar_indice_primer_valor_no_nulo(lista_bloqueados_correspondiente);
               t_pcb* pcb_desbloqueado = malloc(sizeof(t_pcb));
               pcb_desbloqueado = list_get(lista_bloqueados_correspondiente,indice_primer_valor);
               desbloquear_proceso(planificador,pcb_desbloqueado,recurso_recibido_signal->nombre_recurso);
               liberar_memoria_pcb(pcb_desbloqueado);
         }
          enviar_proceso_a_cpu(recurso_recibido_signal->pcb,socket_dispatch);
          list_destroy_and_destroy_elements(lista_bloqueados_correspondiente,liberar_memoria_pcb);
      }
      else{
         //NO EXISTE RECURSO, MANDAR A EXIT
         mandar_proceso_a_finalizar(recurso_recibido_signal->pcb->pid);
      }
      liberar_memoria_t_recurso(recurso_recibido_signal);
      break;
   case SOLICITUD_IO_STDIN_READ:
      //Recibo pid, interfaz, direccion y tamanio desde cpu, se solicita a IO que haga la operacion IO_STDIN_READ(hay que bloquear el porceso)
      log_info(logger_kernel,"Recibo SOLICITUD_IO_STDIN_READ desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_stdin_stdout* io_stdin_read = malloc(sizeof(t_io_stdin_stdout));
      io_stdin_read = deserializar_io_stdin_stdout(lista_paquete);
      enviar_interrupcion_a_cpu(io_stdin_read->pid,INTERRUPCION_IO,io_stdin_read->nombre_interfaz,conexion_cpu_interrupt);
      if(dictionary_has_key(interfaces,io_stdin_read->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
         interfaz_encontrada = dictionary_get(interfaces,io_stdin_read->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_STDIN_READ)){
            
            
            sem_wait(&sem_io_fs_libre);// USAR SEMAFOROS para ordenar el envio a la interfaz, ya que solo atiende de a 1 pedido.
            
            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_stdin_read = malloc(sizeof(t_proceso_data));
            proceso_data_stdin_read->op = IO_K_STDIN;
            pthread_mutex_lock(&mutex_cola_exec);
            proceso_data_stdin_read->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid);
            pthread_mutex_unlock(&mutex_cola_exec);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_direcciones_fisicas* io_direcciones_fisicas_a_bloquear = malloc(sizeof(t_io_direcciones_fisicas));
            io_direcciones_fisicas_a_bloquear->pid = io_stdin_read->pid;
            io_direcciones_fisicas_a_bloquear->tamanio_operacion = io_stdin_read->tamanio;    
            io_direcciones_fisicas_a_bloquear->direcciones_fisicas = io_stdin_read->direccion; 
            proceso_data_stdin_read->data = io_direcciones_fisicas_a_bloquear;
            

            //sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS
               
            if(cronometro != NULL){
               if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
               
                  actualizar_quantum(proceso_data_stdin_read->pcb);
               } 
            }
                     
            bloquear_proceso(planificador,proceso_data_stdin_read,interfaz_encontrada->nombre);
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_stdin_read->pcb->pid);
            pthread_mutex_lock(&mutex_cola_blocked);  
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            pthread_mutex_unlock(&mutex_cola_blocked);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_io_df(io_direcciones_fisicas_a_bloquear,interfaz_encontrada->conexion,a_enviar_a_io->op);
            pthread_mutex_unlock(&mutex_envio_io);
            //liberar_memoria_t_proceso_data(proceso_data_stdin_read);
            //liberar_memoria_t_io_direcciones_fisicas(io_direcciones_fisicas_a_bloquear);
            //list_destroy_and_destroy_elements(lista_nueva_read,free);
            //liberar_memoria_t_proceso_data(a_enviar_a_io);



         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            //ENVIAR PROCESO A EXIT
            mandar_proceso_a_finalizar(io_stdin_read->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdin_read->pid);
         }

         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);

         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         //ENVIAR PROCESO A EXIT?
         mandar_proceso_a_finalizar(io_stdin_read->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdin_read->pid);
      }
     // replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid));
      liberar_memoria_t_io_stdin_stdout(io_stdin_read);
      break;
   case SOLICITUD_IO_STDOUT_WRITE:

      //Recibo pid, interfaz, direccion y tamanio desde cpu, se solicita a IO que haga la operacion IO_STDOUT_WRITE(hay que bloquear el porceso)
      log_info(logger_kernel,"Recibo SOLICITUD_IO_STDOUT_WRITE desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_stdin_stdout* io_stdout_write = malloc(sizeof(t_io_stdin_stdout));
      io_stdout_write = deserializar_io_stdin_stdout(lista_paquete);
      if(dictionary_has_key(interfaces,io_stdout_write->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_stdout_write->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_STDOUT_WRITE)){

            enviar_interrupcion_a_cpu(io_stdout_write->pid,INTERRUPCION_IO,io_stdout_write->nombre_interfaz,conexion_cpu_interrupt);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_stdout_write = malloc(sizeof(t_proceso_data));
            proceso_data_stdout_write->op = IO_K_STDOUT;
            proceso_data_stdout_write->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_stdout_write->pid);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_direcciones_fisicas* io_direcciones_fisicas_a_bloquear_write = malloc(sizeof(t_io_direcciones_fisicas));
            io_direcciones_fisicas_a_bloquear_write->pid = io_stdout_write->pid;
            io_direcciones_fisicas_a_bloquear_write->tamanio_operacion = io_stdout_write->tamanio;
            t_list* lista_nueva_write = list_create();
            list_add(lista_nueva_write,io_stdout_write->direccion);
            io_direcciones_fisicas_a_bloquear_write->direcciones_fisicas = lista_nueva_write; 
            proceso_data_stdout_write->data = io_direcciones_fisicas_a_bloquear_write;
            
            
            sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS

            if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
               actualizar_quantum(proceso_data_stdout_write->pcb);
            } 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_stdout_write->pcb->pid);
                     
            bloquear_proceso(planificador,proceso_data_stdout_write,interfaz_encontrada->nombre);
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io_write = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_io_df(a_enviar_a_io_write->data,socket_servidor,a_enviar_a_io_write->op);
            pthread_mutex_unlock(&mutex_envio_io);

            liberar_memoria_t_proceso_data(proceso_data_stdout_write);
            liberar_memoria_t_io_direcciones_fisicas(io_direcciones_fisicas_a_bloquear_write);
            list_destroy_and_destroy_elements(lista_nueva_write,free);
            liberar_memoria_t_proceso_data(a_enviar_a_io_write);
            
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            //ENVIAR PROCESO A EXIT
            mandar_proceso_a_finalizar(io_stdout_write->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdout_write->pid);
         }
         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         //ENVIAR PROCESO A EXIT?
         mandar_proceso_a_finalizar(io_stdout_write->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdout_write->pid);
      }
      //mandar_interrupcion_a_cpu();
      liberar_memoria_t_io_stdin_stdout(io_stdout_write);
      break;
   
   case SOLICITUD_IO_FS_CREATE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_CREATE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_crear_archivo* io_crear_archivo = malloc(sizeof(t_io_crear_archivo));
      io_crear_archivo = deserializar_io_crear_archivo(lista_paquete);
      if(dictionary_has_key(interfaces,io_crear_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_crear_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_CREATE)){
            enviar_interrupcion_a_cpu(io_crear_archivo->pid,INTERRUPCION_IO,io_crear_archivo->nombre_interfaz,conexion_cpu_interrupt);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_io_crear_archivo = malloc(sizeof(t_proceso_data));
            proceso_data_io_crear_archivo->op = IO_FS_CREATE;
            proceso_data_io_crear_archivo->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_crear_archivo->pid);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_gestion_archivo* io_create_archivo_a_bloquear = malloc(sizeof(t_io_gestion_archivo));
            io_create_archivo_a_bloquear->pid = io_crear_archivo->pid;
            io_create_archivo_a_bloquear->nombre_archivo_length = io_crear_archivo->nombre_archivo_length;
            strcpy(io_create_archivo_a_bloquear->nombre_archivo, io_crear_archivo->nombre_archivo);
            proceso_data_io_crear_archivo->data = io_create_archivo_a_bloquear;
            
            
            sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS

            if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
               actualizar_quantum(proceso_data_io_crear_archivo->pcb);
            } 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_io_crear_archivo->pcb->pid);
                     
            bloquear_proceso(planificador,proceso_data_io_crear_archivo,interfaz_encontrada->nombre);
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io_fs_create = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_gestionar_archivo(a_enviar_a_io_fs_create->data,socket_servidor,a_enviar_a_io_fs_create->op);
            pthread_mutex_unlock(&mutex_envio_io);
            liberar_memoria_t_proceso_data(proceso_data_io_crear_archivo);
            liberar_memoria_t_io_gestion_archivo(io_create_archivo_a_bloquear);
            liberar_memoria_t_proceso_data(a_enviar_a_io_fs_create);
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_crear_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_crear_archivo->pid);
         }
         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_crear_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_crear_archivo->pid);
      }

      //replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_crear_archivo->pid));
      liberar_memoria_t_io_crear_archivo(io_crear_archivo);
      break;
   case SOLICITUD_IO_FS_DELETE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_DELETE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_crear_archivo* io_delete_archivo = malloc(sizeof(t_io_crear_archivo));
      io_delete_archivo = deserializar_io_crear_archivo(lista_paquete);
      if(dictionary_has_key(interfaces,io_delete_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_delete_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_DELETE)){
            enviar_interrupcion_a_cpu(io_delete_archivo->pid,INTERRUPCION_IO,io_delete_archivo->nombre_interfaz,conexion_cpu_interrupt);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_io_delete_archivo = malloc(sizeof(t_proceso_data));
            proceso_data_io_delete_archivo->op = IO_FS_DELETE;
            proceso_data_io_delete_archivo->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_delete_archivo->pid);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_gestion_archivo* io_delete_archivo_a_bloquear = malloc(sizeof(t_io_gestion_archivo));
            io_delete_archivo_a_bloquear->pid = io_delete_archivo->pid;
            io_delete_archivo_a_bloquear->nombre_archivo_length = io_delete_archivo->nombre_archivo_length;
            strcpy(io_delete_archivo_a_bloquear->nombre_archivo, io_delete_archivo->nombre_archivo);
            proceso_data_io_delete_archivo->data = io_delete_archivo_a_bloquear;
            
            
            sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS

            if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
               actualizar_quantum(proceso_data_io_delete_archivo->pcb);
            } 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_io_delete_archivo->pcb->pid);
                     
            bloquear_proceso(planificador,proceso_data_io_delete_archivo,interfaz_encontrada->nombre);
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io_fs_delete = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_gestionar_archivo(a_enviar_a_io_fs_delete->data,socket_servidor,a_enviar_a_io_fs_delete->op);
            pthread_mutex_unlock(&mutex_envio_io);
            liberar_memoria_t_proceso_data(proceso_data_io_delete_archivo);
            liberar_memoria_t_io_gestion_archivo(io_delete_archivo_a_bloquear);
            liberar_memoria_t_proceso_data(proceso_data_io_delete_archivo);
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_delete_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_delete_archivo->pid);
         }
         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_delete_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_delete_archivo->pid);
      }
      //replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_delete_archivo->pid));
      liberar_memoria_t_io_crear_archivo(io_crear_archivo);
      break;
   case SOLICITUD_IO_FS_TRUNCATE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_TRUNCATE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_fs_truncate* io_truncate_archivo = malloc(sizeof(t_io_fs_truncate));
      io_truncate_archivo = deserializar_io_truncate_archivo(lista_paquete);

      if(dictionary_has_key(interfaces,io_truncate_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_truncate_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_TRUNCATE)){
            enviar_interrupcion_a_cpu(io_truncate_archivo->pid,INTERRUPCION_IO,io_truncate_archivo->nombre_interfaz,conexion_cpu_interrupt);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_io_truncate_archivo = malloc(sizeof(t_proceso_data));
            proceso_data_io_truncate_archivo->op = IO_FS_TRUNCATE;
            proceso_data_io_truncate_archivo->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_truncate_archivo->pid);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_gestion_archivo* io_truncate_archivo_a_bloquear = malloc(sizeof(t_io_gestion_archivo));
            io_truncate_archivo_a_bloquear->pid = io_truncate_archivo->pid;
            io_truncate_archivo_a_bloquear->nombre_archivo_length = io_truncate_archivo->nombre_archivo_length;
            strcpy(io_truncate_archivo_a_bloquear->nombre_archivo, io_truncate_archivo->nombre_archivo);
            io_truncate_archivo_a_bloquear->tamanio_archivo = io_truncate_archivo->tamanio;
            proceso_data_io_truncate_archivo->data = io_truncate_archivo_a_bloquear;
            
            
            sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS

            if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
               actualizar_quantum(proceso_data_io_truncate_archivo->pcb);
            } 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_io_truncate_archivo->pcb->pid);
                     
            bloquear_proceso(planificador,proceso_data_io_truncate_archivo,interfaz_encontrada->nombre);
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io_fs_truncate = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_gestionar_archivo(a_enviar_a_io_fs_truncate->data,socket_servidor,a_enviar_a_io_fs_truncate->op);
            pthread_mutex_unlock(&mutex_envio_io);
            liberar_memoria_t_proceso_data(proceso_data_io_truncate_archivo);
            liberar_memoria_t_io_gestion_archivo(io_truncate_archivo_a_bloquear);
            liberar_memoria_t_proceso_data(proceso_data_io_truncate_archivo);
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_truncate_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_truncate_archivo->pid);
         }
         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_truncate_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_truncate_archivo->pid);
      }

     // replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_truncate_archivo->pid));
      liberar_memoria_t_io_fs_truncate(io_truncate_archivo);
      break;
   case SOLICITUD_IO_FS_WRITE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_WRITE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_fs_write* io_write_archivo = malloc(sizeof(t_io_fs_write));
      io_write_archivo = deserializar_io_write_archivo(lista_paquete);
      if(dictionary_has_key(interfaces,io_write_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_write_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_WRITE)){
            enviar_interrupcion_a_cpu(io_write_archivo->pid,INTERRUPCION_IO,io_write_archivo->nombre_interfaz,conexion_cpu_interrupt);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_io_write_archivo = malloc(sizeof(t_proceso_data));
            proceso_data_io_write_archivo->op = IO_FS_WRITE;
            proceso_data_io_write_archivo->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_write_archivo->pid);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_readwrite_archivo* io_write_archivo_a_bloquear = malloc(sizeof(t_io_readwrite_archivo));
            io_write_archivo_a_bloquear->pid = io_write_archivo->pid;
            io_write_archivo_a_bloquear->nombre_archivo_length = io_write_archivo->nombre_archivo_length;
            strcpy(io_write_archivo_a_bloquear->nombre_archivo, io_write_archivo->nombre_archivo);
            io_write_archivo_a_bloquear->direcciones_fisicas = io_write_archivo->direccion;
            io_write_archivo_a_bloquear->puntero_archivo = io_write_archivo->puntero_archivo;
            io_write_archivo_a_bloquear->tamanio_operacion = io_write_archivo->tamanio;
            proceso_data_io_write_archivo->data = io_write_archivo_a_bloquear;
            
            
            sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS

            if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
               actualizar_quantum(proceso_data_io_write_archivo->pcb);
            } 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_io_write_archivo->pcb->pid);
                     
            bloquear_proceso(planificador,proceso_data_io_write_archivo,interfaz_encontrada->nombre);
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io_fs_write = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_io_readwrite(a_enviar_a_io_fs_write->data,socket_servidor,a_enviar_a_io_fs_write->op);
            pthread_mutex_unlock(&mutex_envio_io);
            liberar_memoria_t_proceso_data(proceso_data_io_write_archivo);
            liberar_memoria_t_io_readwrite_archivo(io_write_archivo_a_bloquear);
            liberar_memoria_t_proceso_data(a_enviar_a_io_fs_write);
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_write_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_write_archivo->pid);
         }
         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_write_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_write_archivo->pid);
      }

      //replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_write_archivo->pid));
      liberar_memoria_t_io_fs_write(io_write_archivo);
      break;
   case SOLICITUD_IO_FS_READ_A_KERNEL:

      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_READ_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(socket_dispatch);
      
      t_io_fs_write* io_read_archivo = malloc(sizeof(t_io_fs_write));
      io_read_archivo = deserializar_io_write_archivo(lista_paquete);

      if(dictionary_has_key(interfaces,io_read_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_read_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_READ)){
            enviar_interrupcion_a_cpu(io_read_archivo->pid,INTERRUPCION_IO,io_read_archivo->nombre_interfaz,conexion_cpu_interrupt);
            sem_wait(&sem_io_fs_libre);

            // preparo la estructura para mandar a  cola de bloqueados correspondiente
            t_proceso_data* proceso_data_io_read_archivo = malloc(sizeof(t_proceso_data));
            proceso_data_io_read_archivo->op = IO_FS_READ;
            proceso_data_io_read_archivo->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_read_archivo->pid);
            //transformo t_io_stdin_stdout en t_io_direcciones_fisicas y lo paso como data del t_proceso_data
            t_io_readwrite_archivo* io_read_archivo_a_bloquear = malloc(sizeof(t_io_readwrite_archivo));
            io_read_archivo_a_bloquear->pid = io_read_archivo->pid;
            io_read_archivo_a_bloquear->nombre_archivo_length = io_read_archivo->nombre_archivo_length;
            strcpy(io_read_archivo_a_bloquear->nombre_archivo, io_read_archivo->nombre_archivo);
            io_read_archivo_a_bloquear->direcciones_fisicas = io_read_archivo->direccion;
            io_read_archivo_a_bloquear->puntero_archivo = io_read_archivo->puntero_archivo;
            io_read_archivo_a_bloquear->tamanio_operacion = io_read_archivo->tamanio;
            proceso_data_io_read_archivo->data = io_read_archivo_a_bloquear;
            
            
            sem_wait(&sem_interrupcion_atendida);// agregar antes de los bloques de io en TODOS

            if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
               actualizar_quantum(proceso_data_io_read_archivo->pcb);
            } 
            log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_data_io_read_archivo->pcb->pid);
                     
            bloquear_proceso(planificador,proceso_data_io_read_archivo,interfaz_encontrada->nombre);
            //obtener proximo proceso en la lista de bloqueados de esa interfaz y enviar ese a IO
            t_proceso_data* a_enviar_a_io_fs_read = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //enviar_io_stdin_read(io_stdin_read,socket_servidor);
            pthread_mutex_lock(&mutex_envio_io);
            enviar_io_readwrite(a_enviar_a_io_fs_read->data,socket_servidor,a_enviar_a_io_fs_read->op);
            pthread_mutex_unlock(&mutex_envio_io);
            liberar_memoria_t_proceso_data(proceso_data_io_read_archivo);
            liberar_memoria_t_io_readwrite_archivo(io_read_archivo_a_bloquear);
            liberar_memoria_t_proceso_data(a_enviar_a_io_fs_read);
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            sem_wait(&sem_planificar);
            mandar_proceso_a_finalizar(io_read_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_read_archivo->pid);
            sem_post(&sem_planificar);
         }
         liberar_memoria_t_interfaz_diccionario(interfaz_encontrada);
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         sem_wait(&sem_planificar);
         mandar_proceso_a_finalizar(io_read_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_read_archivo->pid);
         sem_post(&sem_planificar);
      }
      
      //replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_read_archivo->pid));
      liberar_memoria_t_io_fs_write(io_read_archivo);
      break;
 /*  case -1:
      log_error(logger_kernel, "Desconexion de cpu - Dispatch");
      control_key = 0;
      break;*/
   default:
      log_warning(logger_kernel, "Operacion desconocida de cpu - Dispatch");
      break;
   }

   list_destroy(lista_paquete); // 0j0 CON EESTA LINEA
}

}

void Kernel_escuchar_cpu_interrupt(int *conexion){
uint32_t cod_op;
int socket_interrupt = *conexion;


    while (socket_interrupt != -1) {

        if (recv(socket_interrupt, &cod_op, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
            log_info(logger_kernel, "DISCONNECT! INTERRUPT");
            return;
        }   
   switch (cod_op)
   {
   default:
      log_warning(logger_kernel, "Operacion desconocida de cpu - interrupt");
      break;
   }
}

}

void Kernel_escuchar_memoria(int *conexion){

   int socket_memoria = *conexion;

   bool control_key = 1;
   while (control_key)
   {
      uint32_t cod_op = recibir_operacion(socket_memoria);
      switch (cod_op)
      {
      case -1:
         log_error(logger_kernel, "Desconexion de memoria");
         control_key = 0;
         break;
      case FINALIZAR_PROCESO_FIN:
         t_list* valores = recibir_paquete(socket_memoria);
         sem_post(&sem_confirmacion_memoria);
         break;
      case CREAR_PROCESO_KERNEL_FIN:
      printf("LLEGA PROCESO CREADO DE MEMORIA");
      t_list* lista_paquete_crear_proceso_fin = recibir_paquete(socket_memoria);
      //buscar_pcb_en_lista(planificador->cola_new,*(uint32_t*)list_get(lista_paquete_crear_proceso_fin, 0))

      
      sem_post(&sem_rta_crear_proceso);

   
         break;
      default:
         log_warning(logger_kernel, "Operacion desconocida de memoria");
         break;
      }
   }

}




t_pcb* buscar_pcb_en_lista(t_list* lista_de_pcb, uint32_t pid){
   t_pcb* pcb_de_lista;
   for (uint32_t i = 0; i < lista_de_pcb->elements_count; i++)
   {
      pcb_de_lista = list_get(lista_de_pcb,i);
      if(pcb_de_lista->pid == pid){
         return pcb_de_lista;
      }
   }
   
   return NULL;
}

bool interfaz_permite_operacion(t_tipo_interfaz_enum tipo_interfaz, tipo_instruccion instruccion){
   switch (tipo_interfaz)
   {
   case GENERICA:
      return (instruccion == IO_GEN_SLEEP);
      break;
   case STDIN:
      return (instruccion == IO_STDIN_READ);
      break;
   case STDOUT:
      return (instruccion == IO_STDOUT_WRITE);
      break;
   case DIALFS:
      return (instruccion == IO_FS_CREATE || instruccion == IO_FS_DELETE || instruccion == IO_FS_TRUNCATE || instruccion == IO_FS_READ || instruccion == IO_FS_WRITE);
      break;
   
   default:
   printf("INTERFAZ NO ENCONTRADA");
      break;
   }
}


 t_recurso* deserializar_recurso(t_list*  lista_paquete ){

    t_recurso* recurso = malloc(sizeof(t_recurso));
    recurso->pcb->pid = *(uint32_t*)list_get(lista_paquete, 0);
    recurso->pcb->program_counter = *(uint32_t*)list_get(lista_paquete, 1); 
    recurso->pcb->path_length = *(uint32_t*)list_get(lista_paquete, 2); 
    recurso->pcb->path = list_get(lista_paquete, 3);
    recurso->pcb->registros_cpu.PC = *(uint32_t*)list_get(lista_paquete, 4);
    recurso->pcb->registros_cpu.AX = *(uint32_t*)list_get(lista_paquete, 5);
    recurso->pcb->registros_cpu.BX = *(uint32_t*)list_get(lista_paquete, 6); 
    recurso->pcb->registros_cpu.CX = *(uint32_t*)list_get(lista_paquete, 7); 
    recurso->pcb->registros_cpu.DX = *(uint32_t*)list_get(lista_paquete, 8);
    recurso->pcb->registros_cpu.EAX = *(uint32_t*)list_get(lista_paquete, 9);
    recurso->pcb->registros_cpu.EBX = *(uint32_t*)list_get(lista_paquete, 10);
    recurso->pcb->registros_cpu.ECX = *(uint32_t*)list_get(lista_paquete, 11);
    recurso->pcb->registros_cpu.EDX = *(uint32_t*)list_get(lista_paquete, 12); 
    recurso->pcb->registros_cpu.SI = *(uint32_t*)list_get(lista_paquete, 13);
    recurso->pcb->registros_cpu.DI = *(uint32_t*)list_get(lista_paquete, 14);  
    recurso->pcb->estado = *(uint32_t*)list_get(lista_paquete, 15);
    recurso->pcb->tiempo_ejecucion = *(uint32_t*)list_get(lista_paquete, 16);
    recurso->pcb->quantum = *(uint32_t*)list_get(lista_paquete, 17);
    recurso->nombre_recurso_length = *(uint32_t*)list_get(lista_paquete, 18); 
    recurso->nombre_recurso = list_get(lista_paquete, 19); 
    
	return recurso;
}


uint32_t buscar_indice_primer_valor_no_nulo(t_list* lista){
   for (size_t i = 0; i < lista->elements_count; i++)
   {
      if(list_get(lista,i) != NULL){
         return i;
      }
   }
   
}




void actualizar_pcb_proceso_bloqueado(t_planificador* planificador,char* interface, t_pcb* proceso_bloqueado) {
   int pid = proceso_bloqueado->pid;
   log_info(logger_kernel,"Interfaz de proceso bloqueado: %s",interface);
   t_list* list_io = dictionary_get(planificador->cola_blocked,interface);
   int indice = encontrar_indice_proceso_data_por_pid(list_io,pid);
   t_proceso_data*  proceso_data =  list_remove(list_io,indice); 
   proceso_data->pcb = proceso_bloqueado;
   list_add_in_index(list_io, indice,proceso_data ); 
   dictionary_put(planificador->cola_blocked,interface,list_io); // mutex por afuera

}

