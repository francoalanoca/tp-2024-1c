#include "../include/init_memoria.h"


// Definición de la variable global
t_log *logger_memoria; 
t_config *file_cfg_memoria;

t_config_memoria *cfg_memoria;

int fd_memoria;
int fd_cpu;
int fd_kernel;
int fd_entradasalida;




//Funcion que hace la inicializacion de las config y logger
int init(char *path_config) {
    //inicializo estructura de configuracion
    cfg_memoria = cfg_memoria_start();

    logger_memoria = log_create("memoria.log", "Memoria", true, LOG_LEVEL_INFO);

    //Si hubo un error al crear el logger se informara por consola
    if (logger_memoria == NULL) {
        printf("No pude crear el logger");
        return false;
    }

    //inicializo el archivo de configuracion
    file_cfg_memoria = iniciar_config(path_config,logger_memoria);

    //Retorna lo que devuelve la funcion check luego de validar la propierties
    return checkProperties(path_config);
}




//Funcion que checkea las propiedades
int checkProperties(char *path_config) {
    
    //Creamos una config y verificamos si puede ser abierto
    t_config *config = config_create(path_config);

    //Si no pudo ser abierto el config sera informado por consola
    if (config == NULL) {
        log_error(logger_memoria, "Ocurrió un error al intentar abrir el archivo config");
        return false;
    }

    char *properties[] = {
            "PUERTO_ESCUCHA",
            "TAM_MEMORIA",
            "TAM_PAGINA",
            "PATH_INSTRUCCIONES",
            "RETARDO_RESPUESTA",
            NULL
            };

    //Verifico si falta alguna de las propiedades en confid
    if (!config_has_all_properties(config, properties)) {
        log_error(logger_memoria, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}


//Cargo todas las configuracionde en las variables tipo config
int cargar_configuracion(char *path_config) {

    file_cfg_memoria = config_create(path_config);

    //Cargo en la variable tipo config las configuraciones iniciales
    cfg_memoria->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_memoria, "PUERTO_ESCUCHA"));
    log_info(logger_memoria, "PUERTO_ESCUCHA cargado correctamente: %s", cfg_memoria->PUERTO_ESCUCHA);

    cfg_memoria->TAM_MEMORIA = config_get_int_value(file_cfg_memoria, "TAM_MEMORIA");
    log_info(logger_memoria, "TAM_MEMORIA cargado correctamente: %d", cfg_memoria->TAM_MEMORIA);

    cfg_memoria->TAM_PAGINA = config_get_int_value(file_cfg_memoria, "TAM_PAGINA");
    log_info(logger_memoria, "TAM_PAGINA cargado correctamente: %d", cfg_memoria->TAM_PAGINA);

    cfg_memoria->PATH_INSTRUCCIONES = strdup(config_get_string_value(file_cfg_memoria, "PATH_INSTRUCCIONES"));
    log_info(logger_memoria, "PATH_INSTRUCCIONES cargado correctamente: %s", cfg_memoria->PATH_INSTRUCCIONES);

    cfg_memoria-> RETARDO_RESPUESTA = config_get_int_value(file_cfg_memoria, "RETARDO_RESPUESTA");
    log_info(logger_memoria, "RETARDO_RESPUESTA cargado correctamente: %d", cfg_memoria->RETARDO_RESPUESTA);

    

    log_info(logger_memoria, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_memoria);
    return true;
}


/*
//Funcion que inicia las variables necesarias para el funcionamiento de memoria (tablas, paginas, usuario, etc)
void inicializar_memoria(char* path_config){

    memoria_config = iniciar_config(config_path);
	//iniciar_memoria_principal(memoria_config->TAM_MEMORIA, memoria_config->TAM_PAGINA);
	memoria = malloc(memoria_config->TAM_MEMORIA);
	tablas_de_paginas = list_create();
	archivos_swap = list_create();
	instrucciones_de_procesos = dictionary_create();
	pthread_mutex_init(&mutex_swap, NULL);
	pthread_mutex_init(&mutex_memoria, NULL);
	cantidad_acceso_disco = 0;
	cantidad_page_fault = 0;
	cantidad_frames = memoria_config->TAM_MEMORIA / memoria_config->TAM_PAGINA;
	bitmap_frames = bitarray_create_with_mode(memoria, (size_t)(cantidad_frames / 8), LSB_FIRST);
}
*/



void cerrar_programa() {


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_memoria);
    log_info(logger_memoria,"TERMINADA_LA_CONFIG");
    log_info(logger_memoria, "TERMINANDO_EL_LOG");
    log_destroy(logger_memoria);
}