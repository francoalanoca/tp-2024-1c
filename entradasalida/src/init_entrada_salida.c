#include "../include/init_entrada_salida.h"

t_log *logger_entrada_salida; // Definición de la variable global
t_config_entrada_salida *cfg_entrada_salida;
t_config *file_cfg_entrada_salida;

int checkProperties(char *path_config) {
    // config valida
    t_config *config = config_create(path_config);
    if (config == NULL) {
        log_error(logger_entrada_salida, "Ocurrió un error al intentar abrir el archivo config");
        return false;
    }

    char *properties[] = {
            "TIPO_INTERFAZ",
            "TIEMPO_UNIDAD_TRABAJO",
            "IP_KERNEL",
            "PUERTO_KERNEL",
            "IP_MEMORIA",
            "PUERTO_MEMORIA",
            "PATH_BASE_DIALFS",
            "BLOCK_SIZE",
            "BLOCK_COUNT",
            "RETRASO_COMPACTACION",
            NULL};

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties)) {
        log_error(logger_entrada_salida, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}

int cargar_configuracion(char *path) {

    file_cfg_entrada_salida = config_create(path);

    cfg_entrada_salida->TIPO_INTERFAZ = strdup(config_get_string_value(file_cfg_entrada_salida, "TIPO_INTERFAZ"));
    log_info(logger_entrada_salida, "TIPO_INTERFAZ cargado correctamente: %s", cfg_entrada_salida->TIPO_INTERFAZ);

    cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(file_cfg_entrada_salida, "TIEMPO_UNIDAD_TRABAJO");
    log_info(logger_entrada_salida, "TIEMPO_UNIDAD_TRABAJO cargado correctamente: %d", cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO);

    cfg_entrada_salida->IP_KERNEL = strdup(config_get_string_value(file_cfg_entrada_salida, "IP_KERNEL"));
    log_info(logger_entrada_salida, "IP_KERNEL cargado correctamente: %s", cfg_entrada_salida->IP_KERNEL);

    cfg_entrada_salida->PUERTO_KERNEL = strdup(config_get_string_value(file_cfg_entrada_salida, "PUERTO_KERNEL"));
    log_info(logger_entrada_salida, "PUERTO_KERNEL cargado correctamente: %s", cfg_entrada_salida->PUERTO_KERNEL);

    cfg_entrada_salida->IP_MEMORIA = strdup(config_get_string_value(file_cfg_entrada_salida, "IP_MEMORIA"));
    log_info(logger_entrada_salida, "IP_MEMORIA cargado correctamente: %s", cfg_entrada_salida->IP_MEMORIA);

    cfg_entrada_salida->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_entrada_salida, "PUERTO_MEMORIA"));
    log_info(logger_entrada_salida, "PUERTO_MEMORIA cargado correctamente: %s", cfg_entrada_salida->PUERTO_MEMORIA);

    cfg_entrada_salida->PATH_BASE_DIALFS = strdup(config_get_string_value(file_cfg_entrada_salida, "PATH_BASE_DIALFS"));
    log_info(logger_entrada_salida, "PATH_BASE_DIALFS cargado correctamente: %s", cfg_entrada_salida->PATH_BASE_DIALFS);

    cfg_entrada_salida->BLOCK_SIZE = config_get_int_value(file_cfg_entrada_salida, "BLOCK_SIZE");
    log_info(logger_entrada_salida, "BLOCK_SIZE cargado correctamente: %d", cfg_entrada_salida->BLOCK_SIZE);
    
    cfg_entrada_salida->BLOCK_COUNT = config_get_int_value(file_cfg_entrada_salida, "BLOCK_COUNT");
    log_info(logger_entrada_salida, "BLOCK_COUNT cargado correctamente: %d", cfg_entrada_salida->BLOCK_COUNT);

    cfg_entrada_salida->RETRASO_COMPACTACION = config_get_int_value(file_cfg_entrada_salida, "RETRASO_COMPACTACION");
    log_info(logger_entrada_salida, "RETRASO_COMPACTACION cargado correctamente: %d", cfg_entrada_salida->RETRASO_COMPACTACION);


    log_info(logger_entrada_salida, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_entrada_salida);
    return true;
}



int init(char *path_config) {
    //inicializo estructura de configuracion
    cfg_entrada_salida = cfg_entrada_salida_start();

    logger_entrada_salida = log_create("entradasalida.log", "EntradaSalida", true, LOG_LEVEL_INFO);
    if (logger_entrada_salida == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    //inicializo el archivo de configuracion
    file_cfg_entrada_salida = iniciar_config(path_config, logger_entrada_salida);

    return checkProperties(path_config);
}
int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

t_tipo_interfaz_enum obtener_tipo_interfaz_enum (const char* tipo_interfaz_str) {
    if (strcmp(tipo_interfaz_str, "GENERICA") == 0) {
        return GENERICA ;
    } else if (strcmp(tipo_interfaz_str, "STDIN") == 0) {
        return STDIN ;
    } else if (strcmp(tipo_interfaz_str, "STDOUT") == 0) {
        return STDOUT ;
    } else if (strcmp(tipo_interfaz_str, "DIALFS") == 0) {
        return DIALFS ;    
    } else {
        // Manejo de error para tipos de interfaz desconocidos
        return -1; // O podrías lanzar una excepción o manejar el error de otra manera
    }
}

    //INICIAR INTERFACE CORRESPONDIENTE
void iniciar_interface(char* tipo_interfaz_str, int socket_kernel, int socket_memoria){
      
      
      switch (obtener_tipo_interfaz_enum (tipo_interfaz_str)) {
            
            case GENERICA :

                log_info(logger_entrada_salida, "Interfaz GENERICA iniciada");
                break;
            
            case STDIN :

                log_info(logger_entrada_salida, "Interfaz STDIN iniciada");
                break;    

            case STDOUT :

                log_info(logger_entrada_salida, "Interfaz STDOUT iniciada");
                break;  

            case DIALFS :

                log_info(logger_entrada_salida, "Interfaz DIALFS iniciada");
                break;                             

            default:
                log_error(logger_entrada_salida, "Algo anduvo mal en el inicio de ");
                break;
        }
}





void cerrar_programa() {


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_entrada_salida);
    log_info(logger_entrada_salida,"TERMINADA_LA_CONFIG");
    log_info(logger_entrada_salida, "TERMINANDO_EL_LOG");
    log_destroy(logger_entrada_salida);
}