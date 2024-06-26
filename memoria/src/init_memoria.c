#include "../include/init_memoria.h"


// Definición de la variable global
t_log *logger_memoria; 
t_config *file_cfg_memoria;

t_config_memoria *cfg_memoria;

int fd_memoria;
int fd_cpu;
int fd_kernel;
int fd_entradasalida;

void* memoria;
t_list* lista_tablas_de_paginas;
t_list* lista_miniPCBs;
pthread_mutex_t mutex_memoria;
uint32_t cantidad_frames;       //seria tam_memoria / tam_pagina
uint32_t cantidad_page_fault;       
t_bitarray *bitmap_frames;

t_dictionary* instrucciones_de_procesos;


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
    cfg_memoria->PUERTO_ESCUCHA = config_get_int_value(file_cfg_memoria, "PUERTO_ESCUCHA");
    log_info(logger_memoria, "PUERTO_ESCUCHA cargado correctamente: %d", cfg_memoria->PUERTO_ESCUCHA);

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




//Funcion que inicia las variables necesarias para el funcionamiento de memoria (tablas, paginas, usuario, etc)
void inicializar_memoria(){


	memoria = malloc(cfg_memoria->TAM_MEMORIA);             //posiblemente represente el espacio del usuario, ver
	lista_tablas_de_paginas = list_create();                //lista en en donde se almacenara la tabla de paginas de un proceso 
	lista_miniPCBs = list_create();
    instrucciones_de_procesos = dictionary_create();        //memoria de instrucciones
	pthread_mutex_init(&mutex_memoria, NULL);
	cantidad_page_fault = 0;
	cantidad_frames = cfg_memoria->TAM_MEMORIA / cfg_memoria->TAM_PAGINA;
	bitmap_frames = crear_bitmap(cantidad_frames);   
}




//Funcion que redondea el valor al multiplo cercano de base y retorna
int redondear_a_multiplo_mas_cercano_de(int base, int valor){
    int v = valor == 0 ? 1 : valor;
    return (int) ceil((float) v / (float) base) * base;
}

//Funcion que en base a la cantidad de frames crea bitmap
t_bitarray *crear_bitmap(int entradas){

    int ent = entradas;
    // si la cantidad de entradas es menor que lo que puede ocupar un Byte * N, entonces redondeamos
    // al multiplo mas cercano mayor que el valor. Entonces si son 4 entradas -> 8, 15 -> 16, etc.
    if (ent % 8 != 0){
        ent = redondear_a_multiplo_mas_cercano_de(8, ent); 
        log_trace(logger_memoria, "tamanio inusual de memoria/pagina causo conflicto, redondeando al multiplo de 8 mas cercano: %i", ent);
    }

    void *puntero = malloc(ent / 8);
    t_bitarray *bitmap = bitarray_create_with_mode(puntero, ent / 8, LSB_FIRST);

    for (int i = 0; i < bitarray_get_max_bit(bitmap); i++){
        bitarray_clean_bit(bitmap, i);
    }

    return bitmap;
}




void cerrar_programa() {


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_memoria);
    log_info(logger_memoria,"TERMINADA_LA_CONFIG");
    log_info(logger_memoria, "TERMINANDO_EL_LOG");
    log_destroy(logger_memoria);
}