#include "../include/init_cpu.h"

t_log *logger_cpu; // Definición de la variable global
t_config_cpu *cfg_cpu;
t_config *file_cfg_cpu;


int checkProperties(char *path_config) {
    // config valida
    t_config *config = config_create(path_config);
    if (config == NULL) {
        log_error(logger_cpu, "Ocurrió un error al intentar abrir el archivo config");
        return false;
    }

    char *properties[] = {
            "IP_MEMORIA",
            "PUERTO_MEMORIA",
            "PUERTO_ESCUCHA_DISPATCH",
            "PUERTO_ESCUCHA_INTERRUPT",
            "CANTIDAD_ENTRADAS_TLB",
            "ALGORITMO_TLB",
            NULL
            };

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties)) {
        log_error(logger_cpu, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}

int cargar_configuracion(char *path) {

    file_cfg_cpu = config_create(path);

    cfg_cpu->IP_MEMORIA = strdup(config_get_string_value(file_cfg_cpu, "IP_MEMORIA"));
    log_info(logger_cpu, "IP_MEMORIA cargado correctamente: %s", cfg_cpu->IP_MEMORIA);

    cfg_cpu->PUERTO_MEMORIA = config_get_int_value(file_cfg_cpu, "PUERTO_MEMORIA");
    log_info(logger_cpu, "PUERTO_MEMORIA cargado correctamente: %d", cfg_cpu->PUERTO_MEMORIA);

    cfg_cpu->PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_ESCUCHA_DISPATCH"));
    log_info(logger_cpu, "PUERTO_ESCUCHA_DISPATCH cargado correctamente: %s", cfg_cpu->PUERTO_ESCUCHA_DISPATCH);

    cfg_cpu->PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_ESCUCHA_INTERRUPT"));
    log_info(logger_cpu, "PUERTO_ESCUCHA_INTERRUPT cargado correctamente: %s", cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);

    cfg_cpu-> CANTIDAD_ENTRADAS_TLB = config_get_int_value(file_cfg_cpu, "CANTIDAD_ENTRADAS_TLB");
    log_info(logger_cpu, "CANTIDAD_ENTRADAS_TLB cargado correctamente: %d", cfg_cpu->CANTIDAD_ENTRADAS_TLB);

    cfg_cpu->ALGORITMO_TLB = strdup(config_get_string_value(file_cfg_cpu, "ALGORITMO_TLB"));
    log_info(logger_cpu, "ALGORITMO_TLB cargado correctamente: %s", cfg_cpu->ALGORITMO_TLB);

    log_info(logger_cpu, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_cpu);
    return true;
}



int init(char *path_config) {
    //inicializo estructura de configuracion
    cfg_cpu = cfg_cpu_start();

    logger_cpu = log_create("entradasalida.log", "EntradaSalida", true, LOG_LEVEL_INFO);
    if (logger_cpu == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    //inicializo el archivo de configuracion
    file_cfg_cpu = iniciar_config(path_config,logger_cpu);

    return checkProperties(path_config);
}




   



void cerrar_programa() {


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_cpu);
    log_info(logger_cpu,"TERMINADA_LA_CONFIG");
    log_info(logger_cpu, "TERMINANDO_EL_LOG");
    log_destroy(logger_cpu);
}