
#include </home/utnso/tp-2024-1c-Pasaron-cosas/kernel/include/protocolo_kernel.h>
//#include <protocolo_kernel.h>




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


void Kernel_atender_cpu_dispatch(){

bool control_key = 1;
t_list* lista_paquete =  malloc(sizeof(t_list));
while (control_key)
{
   int cod_op = recibir_operacion(conexion_cpu_dispatch);
   switch (cod_op)
   {
   case INTERRUPCION_CPU:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo t_proceso_interrumpido(pcb y motivo?) desde cpu(funcion check_interrupt)
      log_info(logger_kernel,"Recibo INTERRUPCION_CPU desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_proceso_interrumpido* proceso_interrumpido = malloc(sizeof(t_proceso_interrumpido));
      proceso_interrumpido = deserializar_proceso_interrumpido(lista_paquete);
      //poner proceso recibido en lista de procesos_interrumpidos? y activar semaforo para que el planificador continue con la finalizacion del proceso  
      break;
   case ENVIO_INTERFAZ:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA 
      //Recibo PID,nombre de la interfaz y unidades de trabajo de cpu, debo pedir a kernel que realice la instruccion IO_GEN_SLEEP (comprobar interfaz en diccionaro de interfaces antes)         
      log_info(logger_kernel,"Recibo ENVIO_INTERFAZ desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_gen_sleep* io_gen_sleep = malloc(sizeof(t_io_gen_sleep));
      io_gen_sleep = deserializar_io_gen_sleep(lista_paquete);

      //Verifico que la interfaz exista y este conectada
      if(dictionary_has_key(interfaces,io_gen_sleep->interfaz->nombre)){
         if(interfaz_permite_operacion(io_gen_sleep->interfaz->tipo,IO_GEN_SLEEP)){
            t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
            interfaz_encontrada = dictionary_get(interfaces,io_gen_sleep->interfaz->nombre);
               //AHORA DEBO ENVIAR A IO LO NECESARIO
            enviar_io_gen_sleep(io_gen_sleep,interfaz_encontrada->conexion);

            //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
            t_pcb* pcb_a_bloquear_io_gen_sleep = malloc(sizeof(t_pcb));
            pcb_a_bloquear_io_gen_sleep = buscar_pcb_en_lista(planificador->cola_exec,io_gen_sleep->pid);
            if(pcb_a_bloquear_io_gen_sleep != NULL){
               bloquear_proceso(planificador,pcb_a_bloquear_io_gen_sleep);
            }
            else{
               log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
               //ENVIAR PROCESO A EXIT
               
            }
         }
         else{
            log_info(logger_kernel,"ERROR: LA INTERFAZ NO PERMITE EL TIPO DE OPERACION");
            //ENVIAR PROCESO A EXIT
         }
         
      }
      else{
         log_info(logger_kernel,"ERROR: LA INTERFAZ NO EXISTE O NO ESTA CONECTADA");
      }

      
      
      break;
   case ENVIAR_ERROR_MEMORIA_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo t_proceso(ver si seria solo el pcb) desde cpu al recibir un error de out of memory en instruccion resize
      log_info(logger_kernel,"Recibo ENVIAR_ERROR_MEMORIA_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_pcb* proceso_recibido_error_memoria = malloc(sizeof(t_pcb));
      proceso_recibido_error_memoria = deserializar_pcb(lista_paquete);

      if(proceso_recibido_error_memoria != NULL){
         desalojar_proceso(planificador,proceso_recibido_error_memoria);//esta bien en este caso que pase de exec a ready?
      }
      else{
         log_info(logger_kernel,"El proceso recibido es nulo");
      }
      break;
   case ENVIO_WAIT_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo t_proceso(debe ser solo pcb?) y recurso desde cpu, debo asignar una instancia del recurso al proceso(verificar recursos disponibles)
      break;
   case ENVIO_SIGNAL_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo t_proceso(debe ser solo pcb?) y recurso desde cpu, debo liberar una instancia del recurso al proceso(verificar recursos disponibles)
      break;
   case SOLICITUD_IO_STDIN_READ:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo interfaz(solo el nombre y su length?), direccion y tamanio desde cpu, se solicita a IO que haga la operacion IO_STDIN_READ(hay que bloquear el porceso)
      log_info(logger_kernel,"Recibo SOLICITUD_IO_STDIN_READ desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_stdin_stdout* io_stdin_read = malloc(sizeof(t_io_stdin_stdout));
      io_stdin_read = deserializar_io_stdin_stdout(lista_paquete);

      enviar_io_stdin_read(io_stdin_read,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear_stdout_read = malloc(sizeof(t_pcb));
      pcb_a_bloquear_stdout_read = buscar_pcb_en_lista(planificador->cola_exec,io_stdin_read->pid);
      if(pcb_a_bloquear_stdout_read != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear_stdout_read);
      }
      break;
      break;
   case SOLICITUD_IO_STDOUT_WRITE:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo pid, interfaz(solo el nombre y su length?), direccion y tamanio desde cpu, se solicita a IO que haga la operacion IO_STDOUT_WRITE(hay que bloquear el porceso)
      log_info(logger_kernel,"Recibo SOLICITUD_IO_STDOUT_WRITE desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_stdin_stdout* io_stdout_write = malloc(sizeof(t_io_stdin_stdout));
      io_stdout_write = deserializar_io_stdin_stdout(lista_paquete);

      enviar_io_stdout_write(io_stdout_write,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear_stdout_write = malloc(sizeof(t_pcb));
      pcb_a_bloquear_stdout_write = buscar_pcb_en_lista(planificador->cola_exec,io_stdout_write->pid);
      if(pcb_a_bloquear_stdout_write != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear_stdout_write);
      }
      break;
   case SOLICITUD_EXIT_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      //Recibo PCB desde cpu, se debe finalizar el proceso
      log_info(logger_kernel,"Recibo SOLICITUD_EXIT_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_pcb* proceso_recibido = malloc(sizeof(t_pcb));
      proceso_recibido = deserializar_pcb(lista_paquete);

      if(proceso_recibido != NULL){
         finalizar_proceso(planificador,proceso_recibido);
      }
      else{
         log_info(logger_kernel,"El proceso recibido es nulo");
      }

      break;
   case SOLICITUD_IO_FS_CREATE_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_CREATE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_crear_archivo* io_crear_archivo = malloc(sizeof(t_io_crear_archivo));
      io_crear_archivo = deserializar_io_crear_archivo(lista_paquete);

      //AHORA DEBO ENVIAR A IO LO NECESARIO
      enviar_creacion_archivo(io_crear_archivo,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear = malloc(sizeof(t_pcb));
      pcb_a_bloquear = buscar_pcb_en_lista(planificador->cola_exec,io_crear_archivo->pid);
      if(pcb_a_bloquear != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear);
      }
      else{
         log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
      }
      
      break;
   case SOLICITUD_IO_FS_DELETE_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_DELETE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_crear_archivo* io_delete_archivo = malloc(sizeof(t_io_crear_archivo));
      io_delete_archivo = deserializar_io_crear_archivo(lista_paquete);

      //AHORA DEBO ENVIAR A IO LO NECESARIO
      enviar_delete_archivo(io_delete_archivo,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear_delete = malloc(sizeof(t_pcb));
      pcb_a_bloquear_delete = buscar_pcb_en_lista(planificador->cola_exec,io_delete_archivo->pid);
      if(pcb_a_bloquear_delete != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear_delete);
      }
      else{
         log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
      }
      break;
   case SOLICITUD_IO_FS_TRUNCATE_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_TRUNCATE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_fs_truncate* io_truncate_archivo = malloc(sizeof(t_io_fs_truncate));
      io_truncate_archivo = deserializar_io_truncate_archivo(lista_paquete);

      //AHORA DEBO ENVIAR A IO LO NECESARIO
      enviar_truncate_archivo(io_truncate_archivo,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear_truncate = malloc(sizeof(t_pcb));
      pcb_a_bloquear_truncate = buscar_pcb_en_lista(planificador->cola_exec,io_truncate_archivo->pid);
      if(pcb_a_bloquear_truncate != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear_truncate);
      }
      else{
         log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
      }
      break;
   case SOLICITUD_IO_FS_WRITE_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_WRITE_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_fs_write* io_write_archivo = malloc(sizeof(t_io_fs_write));
      io_write_archivo = deserializar_io_write_archivo(lista_paquete);

      //AHORA DEBO ENVIAR A IO LO NECESARIO
      enviar_write_archivo(io_write_archivo,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear_write = malloc(sizeof(t_pcb));
      pcb_a_bloquear_write = buscar_pcb_en_lista(planificador->cola_exec,io_write_archivo->pid);
      if(pcb_a_bloquear_write != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear_write);
      }
      else{
         log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
      }
      break;
   case SOLICITUD_IO_FS_READ_A_KERNEL:
      //TODO
      //EMPAQUETAR, DESEREALIZAR Y ENVIAR RTA SI APLICA
      log_info(logger_kernel,"Recibo SOLICITUD_IO_FS_READ_A_KERNEL desde CPU");
      lista_paquete = recibir_paquete(conexion_cpu_dispatch);
      
      t_io_fs_write* io_read_archivo = malloc(sizeof(t_io_fs_write));
      io_read_archivo = deserializar_io_write_archivo(lista_paquete);

      //AHORA DEBO ENVIAR A IO LO NECESARIO
      enviar_read_archivo(io_read_archivo,socket_servidor);

      //TODO:MODIFICAR PCB PARA QUE EL ESTADO SEA "EN IO"(O AGREGAR A LISTA)?
      t_pcb* pcb_a_bloquear_read = malloc(sizeof(t_pcb));
      pcb_a_bloquear_read = buscar_pcb_en_lista(planificador->cola_exec,io_read_archivo->pid);
      if(pcb_a_bloquear_read != NULL){
         bloquear_proceso(planificador,pcb_a_bloquear_read);
      }
      else{
         log_info(logger_kernel,"No se encontro el proceso en la lista de ejecutados");
      }
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