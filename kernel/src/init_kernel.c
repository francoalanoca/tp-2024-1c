#include "../include/init_kernel.h"

t_log *logger_kernel; // Definición de la variable global
t_config_kernel *cfg_kernel;
t_config *file_cfg_kernel;
t_pcb* pcb;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_memoria;
int socket_servidor;
t_dictionary* interfaces; //Diccionario donde se encuentran las interfaces que van llegando de IO
t_dictionary* procesos_recursos; //Diccionario donde se vinculan los procesos con sus recursos asignados

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

