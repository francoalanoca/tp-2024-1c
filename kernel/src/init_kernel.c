#include "../include/init_kernel.h"

t_log *logger_kernel; // Definición de la variable global
t_config_kernel *cfg_kernel;
t_config *file_cfg_kernel;
t_pcb* pcb;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_cpu_dispatch_interrupciones;
int conexion_memoria;
int socket_servidor;
t_dictionary* interfaces; //Diccionario donde se encuentran las interfaces que van llegando de IO
t_dictionary* procesos_recursos; //Diccionario donde se vinculan los procesos con sus recursos asignados
sem_t sem_contexto_ejecucion_recibido;
sem_t sem_confirmacion_memoria;
sem_t sem_io_fs_libre;
sem_t sem_cpu_libre;
sem_t sem_prioridad_io;
sem_t sem_interrupcion_atendida;
sem_t sem_rta_crear_proceso;
sem_t sem_crearServidor;
sem_t sem_EscucharMsj;
pthread_mutex_t mutex_cola_ready_prioridad; 
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_envio_io;
t_list* recursos;
t_planificador* planificador;
uint32_t contador_pid;

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

    cfg_kernel->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_ESCUCHA"));
    log_info(logger_kernel, "PUERTO_ESCUCHA cargado correctamente: %d", cfg_kernel->PUERTO_ESCUCHA);

    cfg_kernel->IP_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "IP_MEMORIA"));
    log_info(logger_kernel, "IP_MEMORIA cargado correctamente: %s", cfg_kernel->IP_MEMORIA);

    cfg_kernel->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_MEMORIA"));
    log_info(logger_kernel, "PUERTO_MEMORIA cargado correctamente: %s", cfg_kernel->PUERTO_MEMORIA);

    cfg_kernel->IP_CPU = strdup(config_get_string_value(file_cfg_kernel, "IP_CPU"));
    log_info(logger_kernel, "IP_CPU cargado correctamente: %s", cfg_kernel->IP_CPU);

    cfg_kernel->PUERTO_CPU_DISPATCH = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_CPU_DISPATCH"));
    log_info(logger_kernel, "PUERTO_CPU_DISPATCH cargado correctamente: %s", cfg_kernel->PUERTO_CPU_DISPATCH);

    cfg_kernel->PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_CPU_INTERRUPT"));
    log_info(logger_kernel, "PUERTO_CPU_INTERRUPT cargado correctamente: %s", cfg_kernel->PUERTO_CPU_INTERRUPT);
    
    cfg_kernel->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(file_cfg_kernel, "ALGORITMO_PLANIFICACION"));
    log_info(logger_kernel, "ALGORITMO_PLANIFICACION cargado correctamente: %s", cfg_kernel->ALGORITMO_PLANIFICACION);

    cfg_kernel->QUANTUM = config_get_int_value(file_cfg_kernel, "QUANTUM");
    log_info(logger_kernel, "QUANTUM cargado correctamente: %d", cfg_kernel->QUANTUM);

    cfg_kernel->RECURSOS =  config_get_array_value(file_cfg_kernel, "RECURSOS");
    log_info(logger_kernel, "RECURSOS cargado correctamente:%s ",cfg_kernel->RECURSOS[1]);// por ahora no logueamos lo cargado

    cfg_kernel->INSTANCIAS_RECURSOS = config_get_array_value(file_cfg_kernel, "INSTANCIA_RECURSOS");
    log_info(logger_kernel, "INSTANCIAS_RECURSOS cargado correctamente: "); // por ahora no logueamos lo cargado

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

    procesos_recursos = dictionary_create();

    return checkProperties(path_config);
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}


/*void cerrar_programa() {

    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_kernel);
    log_info(logger_kernel,"TERMINADA_LA_CONFIG");
    log_info(logger_kernel, "TERMINANDO_EL_LOG");
    log_destroy(logger_kernel);
}*/




void liberar_cfg_kernel(t_config_kernel *cfg) {
    free(cfg->PUERTO_ESCUCHA);
    free(cfg->IP_MEMORIA);
    free(cfg->IP_CPU);
    free(cfg->ALGORITMO_PLANIFICACION);

    for (int i = 0; cfg->RECURSOS[i] != NULL; i++) {
        free(cfg->RECURSOS[i]);
    }
    free(cfg->RECURSOS);

    for (int i = 0; cfg->INSTANCIAS_RECURSOS[i] != NULL; i++) {
        free(cfg->INSTANCIAS_RECURSOS[i]);
    }
    free(cfg->INSTANCIAS_RECURSOS);

    free(cfg);
}

void destruir_diccionario(t_dictionary *dict) {
    
    dictionary_destroy_and_destroy_elements(dict, destruir_elemento );
}

void destruir_elemento(void *elemento) {
        list_destroy_and_destroy_elements((t_list *)elemento, free);
}
void cerrar_programa() {
    // Liberar memoria de la configuración
    liberar_cfg_kernel(cfg_kernel);

    // Liberar diccionarios
    destruir_diccionario(interfaces);
    destruir_diccionario(procesos_recursos);

    // Destruir el archivo de configuración y el logger
    config_destroy(file_cfg_kernel);
    log_info(logger_kernel, "TERMINADA_LA_CONFIG");
    log_info(logger_kernel, "TERMINANDO_EL_LOG");
    log_destroy(logger_kernel);
}

void liberar_memoria_paquete(t_paquete* paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void liberar_memoria_pcb(t_pcb* pcb){
    free(pcb->path);
    free(pcb);
}

void liberar_proceso_interrumpido(t_proceso_interrumpido* proceso_interrumpido){
   liberar_memoria_pcb(proceso_interrumpido->pcb);
   free(proceso_interrumpido);
}

void liberar_memoria_t_io_gen_sleep(t_io_gen_sleep* io_gen_sleep){
   free(io_gen_sleep->nombre_interfaz);
   free(io_gen_sleep);
}

void liberar_memoria_t_interfaz_diccionario(t_interfaz_diccionario* interfaz_diccionario){
   free(interfaz_diccionario->nombre);
   free(interfaz_diccionario);
}

void liberar_memoria_t_proceso_data(t_proceso_data* proceso_data){
   liberar_memoria_pcb(proceso_data->pcb);
   free(proceso_data);
}

void liberar_memoria_t_io_espera(t_io_espera* io_espera){
   free(io_espera);
}

void liberar_memoria_t_recurso(t_recurso* recurso){
   liberar_memoria_pcb(recurso->pcb);
   free(recurso->nombre_recurso);
   free(recurso);
}

void liberar_memoria_t_io_direcciones_fisicas(t_io_direcciones_fisicas* io_direcciones_fisicas){
   free(io_direcciones_fisicas);
}

void liberar_memoria_t_io_stdin_stdout(t_io_stdin_stdout* io_stdin_stdout){
   free(io_stdin_stdout->nombre_interfaz);
   free(io_stdin_stdout);
}

void liberar_memoria_t_io_gestion_archivo(t_io_gestion_archivo* io_gestion_archivo){
   free(io_gestion_archivo->nombre_archivo);
   free(io_gestion_archivo);
}

void liberar_memoria_t_io_crear_archivo(t_io_crear_archivo* io_crear_archivo){
   free(io_crear_archivo->nombre_archivo);
   free(io_crear_archivo->nombre_interfaz);
   free(io_crear_archivo);
}

void liberar_memoria_t_io_fs_truncate(t_io_fs_truncate* io_fs_truncate){
   free(io_fs_truncate->nombre_archivo);
   free(io_fs_truncate->nombre_interfaz);
   free(io_fs_truncate);
}

void liberar_memoria_t_io_readwrite_archivo(t_io_readwrite_archivo* io_readwrite_archivo){
   free(io_readwrite_archivo->nombre_archivo);
   free(io_readwrite_archivo);
}

void liberar_memoria_t_io_fs_write(t_io_fs_write* io_fs_write){
   free(io_fs_write->nombre_archivo);
   free(io_fs_write->nombre_interfaz);
   free(io_fs_write);
}

void liberar_memoria_t_proceso_recurso_diccionario(t_proceso_recurso_diccionario* proceso_recurso_diccionario){
   list_destroy_and_destroy_elements(proceso_recurso_diccionario->nombres_recursos,free);
   list_destroy(proceso_recurso_diccionario->instancias_recursos);
   free(proceso_recurso_diccionario);
}

void liberar_memoria_t_interfaz(t_interfaz* interfaz){
    free(interfaz->nombre);
    free(interfaz);
}

void liberar_memoria_t_interfaz_pid(t_interfaz_pid* interfaz_pid){
    free(interfaz_pid->nombre_interfaz);
    free(interfaz_pid);
}


