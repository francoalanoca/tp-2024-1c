
#include "../include/protocolo_kernel.h"

sem_t *sem_planificar;
t_pcb* pcb_actualizado_interrupcion;

//Funcion que crea hilos para cada modulo y los va atendiendo

void Escuchar_Msj_De_Conexiones(){

//Escuchar los msj de memoria
   pthread_t hilo_kernel_memoria;
   //pthread_create(&hilo_kernel_memoria, NULL, (void*)Kernel_escuchar_memoria, NULL);
   pthread_detach(hilo_kernel_memoria);

//Escuchar los msj de cpu - dispatch
   pthread_t hilo_cpu_dispatch;
  // pthread_create(&hilo_cpu_dispatch, NULL, (void*)Kernel_escuchar_cpu_dispatch, NULL);
   pthread_detach(hilo_cpu_dispatch);

//Escuchar los msj de cpu - interrupt
   pthread_t hilo_cpu_interrupt;
   //pthread_create(&hilo_cpu_interrupt, NULL, (void*)Kernel_escuchar_cpu_interrupt, NULL);
   pthread_join(hilo_cpu_interrupt, NULL);

}


void Kernel_escuchar_cpu_dispatch(){

bool control_key = 1;
t_list* lista_paquete =  malloc(sizeof(t_list));
while (control_key)
{
   int cod_op = recibir_operacion(conexion_cpu_dispatch);
   switch (cod_op)
   {
   case INTERRUPCION_CPU:
      //Recibo t_proceso_interrumpido(pcb y motivo) desde cpu(funcion check_interrupt)
      log_info(logger_kernel,"Recibo INTERRUPCION_CPU desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_proceso_interrumpido* proceso_interrumpido = malloc(sizeof(t_proceso_interrumpido));
      proceso_interrumpido = deserializar_proceso_interrumpido(lista_paquete);
      
      //Detecto motivo de interrupcion y dependiendo de este se decide que es lo que se hace 
      switch (proceso_interrumpido->motivo_interrupcion)
      {
      case INTERRUPCION_OUT_OF_MEMORY:
         if(proceso_interrumpido->pcb != NULL){
            poner_en_cola_exit(proceso_interrumpido->pcb);                     
            mandar_proceso_a_finalizar(proceso_interrumpido->pcb->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: OUT_OF_MEMORY ", proceso_interrumpido->pcb->pid);
         }
         else{
            log_info(logger_kernel,"El proceso recibido es nulo");
         }
      case INSTRUCCION_EXIT:
         if(proceso_interrumpido->pcb != NULL){
            poner_en_cola_exit(proceso_interrumpido->pcb);                     
            mandar_proceso_a_finalizar(proceso_interrumpido->pcb->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: SUCCESS", proceso_interrumpido->pcb->pid);
         }
         else{
            log_info(logger_kernel,"El proceso recibido es nulo");
         }
         break;
      
      case FIN_QUANTUM:
         printf("Se recibió una interrupción de FIN DE QUANTUM");              
         lista_paquete = recibir_paquete(conexion_cpu_dispatch);       
         proceso_interrumpido = deserializar_proceso_interrumpido(lista_paquete);
         log_info(logger_kernel, "PID: %U - Desalojado por fin de Quantum", proceso_interrumpido->pcb->pid); 
         log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: READY", proceso_interrumpido->pcb->pid);
         sem_wait(mutex_cola_ready);
         list_add(planificador->cola_ready,  proceso_interrumpido->pcb); // envolver con mutex 
         sem_post(mutex_cola_ready);
         free(proceso_interrumpido);// crear funcion para liberar pcb
         break;

      
      case ELIMINAR_PROCESO:
         pcb_actualizado_interrupcion = proceso_interrumpido->pcb;
         sem_post(&sem_contexto_ejecucion_recibido);
         break;
      case INTERRUPCION_IO:
         printf("Se recibió una interrupción de IO");    
                 
         lista_paquete = recibir_paquete(conexion_cpu_dispatch);       
         proceso_interrumpido = deserializar_proceso_interrumpido(lista_paquete);
         if (temporal_gettime(cronometro) > 0) { // verifico si hay un cronometro andando
            
          desalojar_proceso_vrr(proceso_interrumpido>pcb);
         } 
         log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO",  proceso_interrumpido->pcb->pid);
        
         free(proceso_interrumpido);// crear funcion para liberar pcb
         sem_post(sem_io);
      default:
      printf("Motivo de interrupcion desconocido. Se finaliza el proceso");
      mandar_proceso_a_finalizar(proceso_interrumpido->pcb->pid);
      
         break;
      }
      break;
   case ENVIO_INTERFAZ:
      //Recibo PID, interfaz y unidades de trabajo de cpu, debo pedir a kernel que realice la instruccion IO_GEN_SLEEP (comprobar interfaz en diccionaro de interfaces antes)         
      log_info(logger_kernel,"Recibo ENVIO_INTERFAZ desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_gen_sleep* io_gen_sleep = malloc(sizeof(t_io_gen_sleep));
      io_gen_sleep = deserializar_io_gen_sleep(lista_paquete);

      //Verifico que la interfaz exista y este conectada
      if(dictionary_has_key(interfaces,io_gen_sleep->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_gen_sleep->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_GEN_SLEEP)){
         
            enviar_io_gen_sleep(io_gen_sleep,interfaz_encontrada->conexion);

            t_pcb* pcb_a_bloquear_io_gen_sleep = malloc(sizeof(t_pcb));
            pcb_a_bloquear_io_gen_sleep = buscar_pcb_en_lista(planificador->cola_exec,io_gen_sleep->pid);
            if(pcb_a_bloquear_io_gen_sleep != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_io_gen_sleep,interfaz_encontrada->nombre);
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               mandar_proceso_a_finalizar(io_gen_sleep->pid);
               log_info(logger_kernel, "Finaliza el proceso %u - Motivo: NO SE ENCUENTRA PROCESO ", io_gen_sleep->pid);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_gen_sleep->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_gen_sleep->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_gen_sleep->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_gen_sleep->pid);
      }

      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_gen_sleep->pid));
      
      
      
      break;

   case ENVIO_WAIT_A_KERNEL:
      //Recibo t_recurso(pcb y nombre recurso) desde cpu, debo asignar una instancia del recurso al proceso(verificar recursos disponibles)
      log_info(logger_kernel,"Recibo ENVIO_WAIT_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
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
         replanificar_y_ejecutar(recurso_recibido_wait->pcb);
      }
      break;
   case ENVIO_SIGNAL_A_KERNEL:
      //Recibo t_recurso(pcb y nombre recurso) desde cpu, debo liberar una instancia del recurso al proceso(verificar recursos disponibles)
      log_info(logger_kernel,"Recibo ENVIO_SIGNAL_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
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
            }
         

         t_list* lista_bloqueados_correspondiente = dictionary_get(planificador->cola_blocked,recurso_recibido_signal->nombre_recurso);
         if(lista_bloqueados_correspondiente->elements_count > 0){
            uint32_t indice_primer_valor = malloc(sizeof(uint32_t));
            indice_primer_valor = buscar_indice_primer_valor_no_nulo(lista_bloqueados_correspondiente);
               t_pcb* pcb_desbloqueado = malloc(sizeof(t_pcb));
               pcb_desbloqueado = list_get(lista_bloqueados_correspondiente,indice_primer_valor);
               desbloquear_proceso(planificador,pcb_desbloqueado,recurso_recibido_signal->nombre_recurso);
               
         }
          enviar_proceso_a_cpu(recurso_recibido_signal->pcb,conexion_cpu_dispatch);
      }
      else{
         //NO EXISTE RECURSO, MANDAR A EXIT
         mandar_proceso_a_finalizar(recurso_recibido_signal->pcb->pid);
      }
      break;
   case SOLICITUD_IO_STDIN_READ:
      //Recibo pid, interfaz, direccion y tamanio desde cpu, se solicita a IO que haga la operacion IO_STDIN_READ(hay que bloquear el porceso)
      log_info(logger_kernel,"Recibo SOLICITUD_IO_STDIN_READ desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_stdin_stdout* io_stdin_read = malloc(sizeof(t_io_stdin_stdout));
      io_stdin_read = deserializar_io_stdin_stdout(lista_paquete);
      if(dictionary_has_key(interfaces,io_stdin_read->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
         interfaz_encontrada = dictionary_get(interfaces,io_stdin_read->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_STDIN_READ)){
            // el pedido debe agregar la estructura 
            //bloquear proceso(con estructura correspondiente)
            t_proceso_data* proceso_data_stdin_read = malloc(sizeof(t_proceso_data));
            proceso_data_stdin_read->pcb = buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid);
            proceso_data_stdin_read->data = io_stdin_read;
            bloquear_proceso(planificador,proceso_data_stdin_read,interfaz_encontrada->nombre);
            enviar_interrupcion_a_cpu(buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid),INTERRUPCION_IO,conexion_cpu_interrupt);

            sem_wait(sem_io_fs_libre);// USAR SEMAFOROS para ordenar el envio a la interfaz, ya que solo atiende de a 1 pedido.
            //obtener proximo proceso en la lista de bloqueados de ese tipo de interfaz y enviar ese a IO
            void* a_enviar_a_io = list_get(dictionary_get(planificador->cola_blocked,interfaz_encontrada->nombre),0);//Obtengo el primer valor(es decir el primero que llego) de la lista de bloqueados correspondiente
            //COMO SE DE QUE TIPO ES LO QUE OBTENGO DE LA LISTA DE BLOQUEADOS?
            enviar_io_stdin_read(io_stdin_read,socket_servidor);

            t_pcb* pcb_a_bloquear_stdout_read = malloc(sizeof(t_pcb));
            pcb_a_bloquear_stdout_read = buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid);
            if(pcb_a_bloquear_stdout_read != NULL){
               sem_wait(sem_io);// agregar antes de los bloques de io en TODOS
               //antes de bloquear crear el  t_proceso_data como hice mas arriba
               bloquear_proceso(planificador,pcb_a_bloquear_stdout_read,interfaz_encontrada->nombre);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            //ENVIAR PROCESO A EXIT
            mandar_proceso_a_finalizar(io_stdin_read->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdin_read->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         //ENVIAR PROCESO A EXIT?
         mandar_proceso_a_finalizar(io_stdin_read->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdin_read->pid);
      }
      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid));
      break;
   case SOLICITUD_IO_STDOUT_WRITE:

      //Recibo pid, interfaz, direccion y tamanio desde cpu, se solicita a IO que haga la operacion IO_STDOUT_WRITE(hay que bloquear el porceso)
      log_info(logger_kernel,"Recibo SOLICITUD_IO_STDOUT_WRITE desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_stdin_stdout* io_stdout_write = malloc(sizeof(t_io_stdin_stdout));
      io_stdout_write = deserializar_io_stdin_stdout(lista_paquete);
      if(dictionary_has_key(interfaces,io_stdout_write->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_stdout_write->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_STDOUT_WRITE)){
            enviar_io_stdout_write(io_stdout_write,socket_servidor);
            t_pcb* pcb_a_bloquear_stdout_write = malloc(sizeof(t_pcb));
            pcb_a_bloquear_stdout_write = buscar_pcb_en_lista(planificador->cola_exec,io_stdout_write->pid);
            if(pcb_a_bloquear_stdout_write != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_stdout_write,interfaz_encontrada->nombre);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            //ENVIAR PROCESO A EXIT
            mandar_proceso_a_finalizar(io_stdout_write->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdout_write->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         //ENVIAR PROCESO A EXIT?
         mandar_proceso_a_finalizar(io_stdout_write->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_stdout_write->pid);
      }
      //mandar_interrupcion_a_cpu();
      break;
   
   case SOLICITUD_IO_FS_CREATE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_CREATE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_crear_archivo* io_crear_archivo = malloc(sizeof(t_io_crear_archivo));
      io_crear_archivo = deserializar_io_crear_archivo(lista_paquete);
      if(dictionary_has_key(interfaces,io_crear_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_crear_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_CREATE)){
            enviar_creacion_archivo(io_crear_archivo,socket_servidor);

            t_pcb* pcb_a_bloquear = malloc(sizeof(t_pcb));
            pcb_a_bloquear = buscar_pcb_en_lista(planificador->cola_exec,io_crear_archivo->pid);
            if(pcb_a_bloquear != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear,interfaz_encontrada->nombre);
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               mandar_proceso_a_finalizar(io_crear_archivo->pid);
               log_info(logger_kernel, "Finaliza el proceso %u - Motivo: NO SE ENCONTRO EL PROCESO ", io_crear_archivo->pid);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_crear_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_crear_archivo->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_crear_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_crear_archivo->pid);
      }

      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_crear_archivo->pid));
      break;
   case SOLICITUD_IO_FS_DELETE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_DELETE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_crear_archivo* io_delete_archivo = malloc(sizeof(t_io_crear_archivo));
      io_delete_archivo = deserializar_io_crear_archivo(lista_paquete);
      if(dictionary_has_key(interfaces,io_delete_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_delete_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_DELETE)){
            enviar_delete_archivo(io_delete_archivo,socket_servidor);

            t_pcb* pcb_a_bloquear_delete = malloc(sizeof(t_pcb));
            pcb_a_bloquear_delete = buscar_pcb_en_lista(planificador->cola_exec,io_delete_archivo->pid);
            if(pcb_a_bloquear_delete != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_delete,interfaz_encontrada->nombre);
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               mandar_proceso_a_finalizar(io_delete_archivo->pid);
               log_info(logger_kernel, "Finaliza el proceso %u - Motivo: NO SE ENCONTRO PROCESO ", io_delete_archivo->pid);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_delete_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_delete_archivo->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_delete_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_delete_archivo->pid);
      }
      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_delete_archivo->pid));
      break;
   case SOLICITUD_IO_FS_TRUNCATE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_TRUNCATE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_fs_truncate* io_truncate_archivo = malloc(sizeof(t_io_fs_truncate));
      io_truncate_archivo = deserializar_io_truncate_archivo(lista_paquete);

      if(dictionary_has_key(interfaces,io_truncate_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_truncate_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_TRUNCATE)){
            enviar_truncate_archivo(io_truncate_archivo,socket_servidor);

            t_pcb* pcb_a_bloquear_truncate = malloc(sizeof(t_pcb));
            pcb_a_bloquear_truncate = buscar_pcb_en_lista(planificador->cola_exec,io_truncate_archivo->pid);
            if(pcb_a_bloquear_truncate != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_truncate,interfaz_encontrada->nombre);
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               mandar_proceso_a_finalizar(io_truncate_archivo->pid);
               log_info(logger_kernel, "Finaliza el proceso %u - Motivo: NO SE ENCONTRO PROCESO ", io_truncate_archivo->pid);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_truncate_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_truncate_archivo->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_truncate_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_truncate_archivo->pid);
      }

      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_truncate_archivo->pid));
      break;
   case SOLICITUD_IO_FS_WRITE_A_KERNEL:
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_WRITE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_fs_write* io_write_archivo = malloc(sizeof(t_io_fs_write));
      io_write_archivo = deserializar_io_write_archivo(lista_paquete);
      if(dictionary_has_key(interfaces,io_write_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_write_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_WRITE)){
            enviar_write_archivo(io_write_archivo,socket_servidor);

            t_pcb* pcb_a_bloquear_write = malloc(sizeof(t_pcb));
            pcb_a_bloquear_write = buscar_pcb_en_lista(planificador->cola_exec,io_write_archivo->pid);
            if(pcb_a_bloquear_write != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_write,interfaz_encontrada->nombre);
               
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               mandar_proceso_a_finalizar(io_write_archivo->pid);
               log_info(logger_kernel, "Finaliza el proceso %u - Motivo: NO SE ENCONTRO PROCESO ", io_write_archivo->pid);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            mandar_proceso_a_finalizar(io_write_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_write_archivo->pid);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         mandar_proceso_a_finalizar(io_write_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_write_archivo->pid);
      }

      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_write_archivo->pid));
      break;
   case SOLICITUD_IO_FS_READ_A_KERNEL:

      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_READ_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_fs_write* io_read_archivo = malloc(sizeof(t_io_fs_write));
      io_read_archivo = deserializar_io_write_archivo(lista_paquete);

      if(dictionary_has_key(interfaces,io_read_archivo->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_read_archivo->nombre_interfaz);
         if(interfaz_permite_operacion(interfaz_encontrada->tipo,IO_FS_READ)){
            
            enviar_read_archivo(io_read_archivo,socket_servidor);

            t_pcb* pcb_a_bloquear_read = malloc(sizeof(t_pcb));
            pcb_a_bloquear_read = buscar_pcb_en_lista(planificador->cola_exec,io_read_archivo->pid);
            if(pcb_a_bloquear_read != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_read,interfaz_encontrada->nombre);
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               sem_wait(&sem_planificar);
               mandar_proceso_a_finalizar(io_read_archivo->pid);
               log_info(logger_kernel, "Finaliza el proceso %u - Motivo: NO SE ENCONTRO PROCESO ", io_read_archivo->pid);
               sem_post(&sem_planificar);
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            sem_wait(&sem_planificar);
            mandar_proceso_a_finalizar(io_read_archivo->pid);
            log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_read_archivo->pid);
            sem_post(&sem_planificar);
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
         sem_wait(sem_planificar);
         mandar_proceso_a_finalizar(io_read_archivo->pid);
         log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INVALID_INTERFACE ", io_read_archivo->pid);
         sem_post(sem_planificar);
      }
      
      replanificar_y_ejecutar(buscar_pcb_en_lista(planificador->cola_exec,io_read_archivo->pid));
      break;
   case -1:
      log_error(logger_kernel, "Desconexion de cpu - Dispatch");
      control_key = 0;
      break;
   default:
      log_warning(logger_kernel, "Operacion desconocida de cpu - Dispatch");
      break;
   }
}

}

void Kernel_escuchar_cpu_interrupt(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_cpu_interrupt);
   switch (cod_op)
   {
   case MENSAJE:
      //
      break;
   case PAQUETE:
      //
      break;
   case -1:
      log_error(logger_kernel, "Desconexion de cpu - interrupt");
      control_key = 0;
      break;
   default:
      log_warning(logger_kernel, "Operacion desconocida de cpu - interrupt");
      break;
   }
}

}

void Kernel_escuchar_memoria(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_memoria);
   switch (cod_op)
   {
   case -1:
      log_error(logger_kernel, "Desconexion de memoria");
      control_key = 0;
      break;
   case FINALIZAR_PROCESO_FIN:
      sem_post(&sem_confirmacion_memoria);
      break;
   default:
      log_warning(logger_kernel, "Operacion desconocida de memoria");
      break;
   }
}

}


//Kernel le envia a memoria lo que pide para crear un proceso
void enviar_creacion_de_proceso_a_memoria(t_pcb* pcb, int conexion_memoria) {
    t_paquete* paquete_enviar_creacion_de_proceso = crear_paquete(CREAR_PROCESO_KERNEL);

    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &pcb->pid, sizeof(uint32_t));
    //int nombre_len = strlen(pcb->nombre_proceso) + 1;
    //agregar_a_paquete(paquete_enviar_creacion_de_proceso, &nombre_len, sizeof(nombre_len));
    //agregar_a_paquete(paquete_enviar_creacion_de_proceso, pcb->nombre_proceso, nombre_len);

    enviar_paquete(paquete_enviar_creacion_de_proceso, conexion_memoria);

    printf("Se envió PCB\n");
    free(paquete_enviar_creacion_de_proceso);
}

t_pcb* buscar_pcb_en_lista(t_list* lista_de_pcb, uint32_t pid){
   t_pcb* pcb_de_lista = malloc(sizeof(t_pcb));
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

t_pcb* encontrar_proceso_pid(t_list * lista_procesos , uint32_t pid) {
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_pcb* proceso = list_get(lista_procesos, i);
        if (proceso->pid == pid) {
            return proceso;
        }
    }
    return NULL;
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


uint32_t buscar_indice_recurso(t_list* lista_recursos,char* nombre_recurso){
   uint32_t indice_encontrado = malloc(sizeof(uint32_t));
   indice_encontrado = NULL;

   for (size_t i = 0; i < lista_recursos->elements_count; i++)
   {
      if(strcmp(list_get(lista_recursos,i), nombre_recurso) == 0){
         indice_encontrado = i;
      }
   }
   return indice_encontrado;
}

uint32_t buscar_indice_primer_valor_no_nulo(t_list* lista){
   for (size_t i = 0; i < lista->elements_count; i++)
   {
      if(list_get(lista,i) != NULL){
         return i;
      }
   }
   
}



void mandar_proceso_a_finalizar(uint32_t pid){
   t_pcb* pcb_a_procesar = malloc(sizeof(t_pcb));
   pcb_a_procesar = encontrar_proceso_pid(planificador->cola_exec,pid);
   eliminar_proceso(planificador,pcb_a_procesar);
}




