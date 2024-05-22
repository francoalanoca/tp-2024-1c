#include "../include/init_kernel.h"

t_log *logger_kernel; // Definición de la variable global
t_config_kernel *cfg_kernel;
t_config *file_cfg_kernel;

int checkProperties(char *path_config) {
    // config valida
    t_config *config = config_create(path_config);
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
            "ALGORTIMO_PLANIFICACION",
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

    file_cfg_kernel = config_create(path);

    cfg_kernel->PUERTO_ESCUCHA = strdup(config_get_int_value(file_cfg_kernel, "PUERTO_ESCUCHA"));
    log_info(logger_kernel, "PUERTO_ESCUCHA cargado correctamente: %d", cfg_kernel->PUERTO_ESCUCHA);

    cfg_kernel->IP_MEMORIA = config_get_string_value(file_cfg_kernel, "IP_MEMORIA");
    log_info(logger_kernel, "IP_MEMORIA cargado correctamente: %s", cfg_kernel->IP_MEMORIA);

    cfg_kernel->PUERTO_MEMORIA = strdup(config_get_int_value(file_cfg_kernel, "PUERTO_MEMORIA"));
    log_info(logger_kernel, "PUERTO_MEMORIA cargado correctamente: %d", cfg_kernel->PUERTO_MEMORIA);

    cfg_kernel->IP_CPU = strdup(config_get_string_value(file_cfg_kernel, "IP_CPU"));
    log_info(logger_kernel, "IP_CPU cargado correctamente: %s", cfg_kernel->IP_CPU);

    cfg_kernel->PUERTO_CPU_DISPATCH = strdup(config_get_int_value(file_cfg_kernel, "PUERTO_CPU_DISPATCH"));
    log_info(logger_kernel, "PUERTO_CPU_DISPATCH cargado correctamente: %d", cfg_kernel->PUERTO_CPU_DISPATCH);

    cfg_kernel->PUERTO_CPU_INTERRUPT = config_get_int_value(file_cfg_kernel, "PUERTO_CPU_INTERRUPT");
    log_info(logger_kernel, "PUERTO_CPU_INTERRUPT cargado correctamente: %d", cfg_kernel->PUERTO_CPU_INTERRUPT);
    
    cfg_kernel->ALGORTIMO_PLANIFICACION = config_get_string_value(file_cfg_kernel, "ALGORITMO_PLANIFICACION");
    log_info(logger_kernel, "ALGORITMO_PLANIFICACION cargado correctamente: %s", cfg_kernel->ALGORTIMO_PLANIFICACION);

    cfg_kernel->QUANTUM = config_get_int_value(file_cfg_kernel, "QUANTUM");
    log_info(logger_kernel, "QUANTUM cargado correctamente: %d", cfg_kernel->QUANTUM);

    cfg_kernel->RECURSOS = config_get_string_value(file_cfg_kernel, "RECURSOS");//?
    log_info(logger_kernel, "RECURSOS cargado correctamente: %s", cfg_kernel->RECURSOS);

    cfg_kernel->INSTANCIA_RECURSOS = config_get_string_value(file_cfg_kernel, "INSTANCIA_RECURSOS");//?
    log_info(logger_kernel, "INSTANCIA_RECURSOS cargado correctamente: %s", cfg_kernel->INSTANCIA_RECURSOS);

    cfg_kernel->GRADO_MULTIPROGRAMACION = config_get_int_value(file_cfg_kernel, "GRADO_MULTIPROGRAMACION");
    log_info(logger_kernel, "GRADO_MULTIPROGRAMACION cargado correctamente: %d", cfg_kernel->GRADO_MULTIPROGRAMACION);

    log_info(logger_kernel, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_kernel);
    return true;
}



int init(char *path_config) {
    //inicializo estructura de configuracion
    cfg_kernel = cfg_kernel_start();

    logger_kernel = log_create("entradasalida.log", "EntradaSalida", true, LOG_LEVEL_INFO);//
    if (logger_kernel == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    //inicializo el archivo de configuracion
    file_cfg_kernel = iniciar_config(path_config);

    return checkProperties(path_config);
}
int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

t_PUERTO_ESCUCHA_enum obtener_PUERTO_ESCUCHA_enum (const char* PUERTO_ESCUCHA_str) {
    if (strcmp(PUERTO_ESCUCHA_str, "GENERICA") == 0) {
        return GENERICA ;
    } else if (strcmp(PUERTO_ESCUCHA_str, "STDIN") == 0) {
        return STDIN ;
    } else if (strcmp(PUERTO_ESCUCHA_str, "STDOUT") == 0) {
        return STDOUT ;
    } else if (strcmp(PUERTO_ESCUCHA_str, "DIALFS") == 0) {
        return DIALFS ;    
    } else {
        // Manejo de error para tipos de interfaz desconocidos
        return -1; // O podrías lanzar una excepción o manejar el error de otra manera
    }
}

    //INICIAR INTERFACE CORRESPONDIENTE
void iniciar_interface(char* PUERTO_ESCUCHA_str, int socket_kernel, int socket_memoria){
      
      
      switch (obtener_PUERTO_ESCUCHA_enum (PUERTO_ESCUCHA_str)) {
            
            case GENERICA :

                log_info(logger_kernel, "Interfaz GENERICA iniciada");
                break;
            
            case STDIN :

                log_info(logger_kernel, "Interfaz STDIN iniciada");
                break;    

            case STDOUT :

                log_info(logger_kernel, "Interfaz STDOUT iniciada");
                break;  

            case DIALFS :

                log_info(logger_kernel, "Interfaz DIALFS iniciada");
                break;                             

            default:
                log_error(logger_kernel, "Algo anduvo mal en el inicio de ");
                break;
        }
}





void cerrar_programa() {


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_kernel);
    log_info(logger_kernel,"TERMINADA_LA_CONFIG");
    log_info(logger_kernel, "TERMINANDO_EL_LOG");
    log_destroy(logger_kernel);
}