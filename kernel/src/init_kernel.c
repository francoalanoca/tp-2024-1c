#include <init_kernel.h>

t_log *logger_kernel; // Definición de la variable global
t_config_kernel *cfg_kernel;
t_config *file_cfg_kernel;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_memoria;
int socket_servidor;
t_dictionary* interfaces; //Diccionario donde se encuentran las interfaces que van llegando de IO

int checkProperties(char *path_config) {
    // config valida
    t_config *config = config_create(path_config); //"/Documents/tp_operativos/tp-2024-1c-Pasaron-cosas/kernel/config/kernel.config"
    if (config == NULL) {
        log_error(logger_kernel, "Ocurrió un error al intentar abrir el archivo config");
        return false;
    }

    char *properties[] = {
            "PUERTO_ESCUCHA",
            "IP_MEMORIA",
            "PUERTO_MEMORIA",
            "IP_CPU",
            "PUERTO_CPU_DISPATCH",
            "PUERTO_CPU_INTERRUPT",
            "ALGORITMO_PLANIFICACION",
            "QUANTUM",
            "RECURSOS",
            "INSTANCIA_RECURSOS",
            "GRADO_MULTIPROGRAMACION",
            NULL};

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties)) {
        log_error(logger_kernel, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}

int cargar_configuracion(char *path) {

    file_cfg_kernel = config_create(path); //"/Documents/tp_operativos/tp-2024-1c-Pasaron-cosas/kernel/config/kernel.config"

    cfg_kernel->PUERTO_ESCUCHA = config_get_int_value(file_cfg_kernel, "PUERTO_ESCUCHA");
    log_info(logger_kernel, "PUERTO_ESCUCHA cargado correctamente: %d", cfg_kernel->PUERTO_ESCUCHA);

    cfg_kernel->IP_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "IP_MEMORIA"));
    log_info(logger_kernel, "IP_MEMORIA cargado correctamente: %s", cfg_kernel->IP_MEMORIA);

    cfg_kernel->PUERTO_MEMORIA = config_get_int_value(file_cfg_kernel, "PUERTO_MEMORIA");
    log_info(logger_kernel, "PUERTO_MEMORIA cargado correctamente: %d", cfg_kernel->PUERTO_MEMORIA);

    cfg_kernel->IP_CPU = strdup(config_get_string_value(file_cfg_kernel, "IP_CPU"));
    log_info(logger_kernel, "IP_CPU cargado correctamente: %s", cfg_kernel->IP_CPU);

    cfg_kernel->PUERTO_CPU_DISPATCH = config_get_int_value(file_cfg_kernel, "PUERTO_CPU_DISPATCH");
    log_info(logger_kernel, "PUERTO_CPU_DISPATCH cargado correctamente: %d", cfg_kernel->PUERTO_CPU_DISPATCH);

    cfg_kernel->PUERTO_CPU_INTERRUPT = config_get_int_value(file_cfg_kernel, "PUERTO_CPU_INTERRUPT");
    log_info(logger_kernel, "PUERTO_CPU_INTERRUPT cargado correctamente: %d", cfg_kernel->PUERTO_CPU_INTERRUPT);
    
    cfg_kernel->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(file_cfg_kernel, "ALGORITMO_PLANIFICACION"));
    log_info(logger_kernel, "ALGORITMO_PLANIFICACION cargado correctamente: %s", cfg_kernel->ALGORITMO_PLANIFICACION);

    cfg_kernel->QUANTUM = config_get_int_value(file_cfg_kernel, "QUANTUM");
    log_info(logger_kernel, "QUANTUM cargado correctamente: %d", cfg_kernel->QUANTUM);

    cfg_kernel->RECURSOS = strdup(config_get_string_value(file_cfg_kernel, "RECURSOS"));
    log_info(logger_kernel, "RECURSOS cargado correctamente: ");// por ahora no logueamos lo cargado

    //cfg_kernel->INSTANCIAS_RECURSOS = strdup(config_get_string_value(file_cfg_kernel, "INSTANCIAS_RECURSOS"));
    //log_info(logger_kernel, "INSTANCIAS_RECURSOS cargado correctamente: "); // por ahora no logueamos lo cargado

    cfg_kernel->GRADO_MULTIPROGRAMACION = config_get_int_value(file_cfg_kernel, "GRADO_MULTIPROGRAMACION");
    log_info(logger_kernel, "GRADO_MULTIPROGRAMACION cargado correctamente: %d", cfg_kernel->GRADO_MULTIPROGRAMACION);

    log_info(logger_kernel, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_kernel);
    return true;
}



int init(char *path_config) {
    //inicializo estructura de configuracion
    cfg_kernel = cfg_kernel_start();

    logger_kernel = log_create("logger_kernel.log", "Kernel", true, LOG_LEVEL_INFO);//
    if (logger_kernel == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    //inicializo el archivo de configuracion
    file_cfg_kernel = iniciar_config(path_config,logger_kernel); //"/Documents/tp_operativos/tp-2024-1c-Pasaron-cosas/kernel/config/kernel.config"

    interfaces = dictionary_create();

    return checkProperties(path_config);
}
 void Empezar_conexiones(){

    //conexion con cpu-dispatch
    conexion_cpu_dispatch = crear_conexion(logger_kernel, "KERNEL", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_DISPATCH);
    
    log_info(logger_kernel, "Socket de KERNEL : %d\n",conexion_cpu_dispatch);  

    //conexion con cpu-interrupt
    conexion_cpu_interrupt = crear_conexion(logger_kernel, "KERNEL", cfg_kernel->IP_CPU, cfg_kernel->PUERTO_CPU_INTERRUPT);
    
    log_info(logger_kernel, "Socket de KERNEL : %d\n",conexion_cpu_interrupt);

    //conexion con memoria
    conexion_memoria = crear_conexion(logger_kernel, "MEMORIA", cfg_kernel->IP_MEMORIA, cfg_kernel->PUERTO_MEMORIA);
    
    log_info(logger_kernel, "Socket de MEMORIA : %d\n",conexion_memoria); 

}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

void AtenderMsjDeConexiones(){

//Atender los msj de memoria
   pthread_t hilo_kernel_memoria;
   pthread_create(&hilo_kernel_memoria, NULL, (void*)Kernel_atender_memoria, NULL);
   pthread_detach(hilo_kernel_memoria);

//Atender los msj de cpu - dispatch
   pthread_t hilo_cpu_dispatch;
   pthread_create(&hilo_cpu_dispatch, NULL, (void*)Kernel_atender_cpu_dispatch, NULL);
   pthread_detach(hilo_cpu_dispatch);

//Atender los msj de cpu - interrupt
   pthread_t hilo_cpu_interrupt;
   pthread_create(&hilo_cpu_interrupt, NULL, (void*)Kernel_atender_cpu_interrupt, NULL);
   pthread_join(hilo_cpu_interrupt, NULL);

}

void cerrar_programa() {

    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_kernel);
    log_info(logger_kernel,"TERMINADA_LA_CONFIG");
    log_info(logger_kernel, "TERMINANDO_EL_LOG");
    log_destroy(logger_kernel);
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

      
      if(dictionary_has_key(interfaces,io_gen_sleep->nombre_interfaz)){
         t_interfaz_diccionario* interfaz_encontrada = malloc(sizeof(t_interfaz_diccionario));
         interfaz_encontrada = dictionary_get(interfaces,io_gen_sleep->nombre_interfaz);
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
         }
      }
      else{
         //ver si la interfaz esta conectada y si permite la operacion
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

void Kernel_atender_cpu_interrupt(){

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

void Kernel_atender_memoria(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_memoria);
   switch (cod_op)
   {
   case MENSAJE:
      //
      break;
   case PAQUETE:
      //
      break;
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


