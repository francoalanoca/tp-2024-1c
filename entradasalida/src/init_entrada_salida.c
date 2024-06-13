#include "../include/init_entrada_salida.h"

t_log *logger_entrada_salida; // Definición de la variable global
t_config_entrada_salida *cfg_entrada_salida;
t_config *file_cfg_entrada_salida;
t_interfaz* interfaz;


int cargar_configuracion(char *nombre_interfaz_custom, char *path) {

    file_cfg_entrada_salida = config_create(path);


    cfg_entrada_salida->NOMBRE_INTERFAZ = strdup(nombre_interfaz_custom);
    log_info(logger_entrada_salida, "NOMBRE_INTERFAZ cargado correctamente: %s", cfg_entrada_salida->NOMBRE_INTERFAZ);

    cfg_entrada_salida->TIPO_INTERFAZ = strdup(config_get_string_value(file_cfg_entrada_salida, "TIPO_INTERFAZ"));
    log_info(logger_entrada_salida, "TIPO_INTERFAZ cargado correctamente: %s", cfg_entrada_salida->TIPO_INTERFAZ);
    
    if (cfg_entrada_salida->TIPO_INTERFAZ == "DIALFS" || cfg_entrada_salida->TIPO_INTERFAZ == "GENERICA" ) {
        
        cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(file_cfg_entrada_salida, "TIEMPO_UNIDAD_TRABAJO");
        log_info(logger_entrada_salida, "TIEMPO_UNIDAD_TRABAJO cargado correctamente: %d", cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO);
     } 
    
    cfg_entrada_salida->IP_KERNEL = strdup(config_get_string_value(file_cfg_entrada_salida, "IP_KERNEL"));
    log_info(logger_entrada_salida, "IP_KERNEL cargado correctamente: %s", cfg_entrada_salida->IP_KERNEL);

    cfg_entrada_salida->PUERTO_KERNEL = strdup(config_get_string_value(file_cfg_entrada_salida, "PUERTO_KERNEL"));
    log_info(logger_entrada_salida, "PUERTO_KERNEL cargado correctamente: %s", cfg_entrada_salida->PUERTO_KERNEL);
   
    if (cfg_entrada_salida->TIPO_INTERFAZ != "GENERICA") {        
        
        cfg_entrada_salida->IP_MEMORIA = strdup(config_get_string_value(file_cfg_entrada_salida, "IP_MEMORIA"));
        log_info(logger_entrada_salida, "IP_MEMORIA cargado correctamente: %s", cfg_entrada_salida->IP_MEMORIA);

        cfg_entrada_salida->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_entrada_salida, "PUERTO_MEMORIA"));
        log_info(logger_entrada_salida, "PUERTO_MEMORIA cargado correctamente: %s", cfg_entrada_salida->PUERTO_MEMORIA);
    }    

    if (cfg_entrada_salida->TIPO_INTERFAZ == "DIALFS") {

    
        cfg_entrada_salida->PATH_BASE_DIALFS = strdup(config_get_string_value(file_cfg_entrada_salida, "PATH_BASE_DIALFS"));
        log_info(logger_entrada_salida, "PATH_BASE_DIALFS cargado correctamente: %s", cfg_entrada_salida->PATH_BASE_DIALFS);

        cfg_entrada_salida->BLOCK_SIZE = config_get_int_value(file_cfg_entrada_salida, "BLOCK_SIZE");
        log_info(logger_entrada_salida, "BLOCK_SIZE cargado correctamente: %d", cfg_entrada_salida->BLOCK_SIZE);
    
        cfg_entrada_salida->BLOCK_COUNT = config_get_int_value(file_cfg_entrada_salida, "BLOCK_COUNT");
        log_info(logger_entrada_salida, "BLOCK_COUNT cargado correctamente: %d", cfg_entrada_salida->BLOCK_COUNT);

        cfg_entrada_salida->RETRASO_COMPACTACION = config_get_int_value(file_cfg_entrada_salida, "RETRASO_COMPACTACION");
        log_info(logger_entrada_salida, "RETRASO_COMPACTACION cargado correctamente: %d", cfg_entrada_salida->RETRASO_COMPACTACION);
        
    }




    log_info(logger_entrada_salida, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_entrada_salida);
    return true;
}
 


int init(char *path_config){ 
    //inicializo estructura de configuracion
    cfg_entrada_salida = cfg_entrada_salida_start();

    logger_entrada_salida = log_create("entradasalida.log", "EntradaSalida", true, LOG_LEVEL_INFO);
    if (logger_entrada_salida == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    //inicializo el archivo de configuracion
    file_cfg_entrada_salida = iniciar_config(path_config, logger_entrada_salida);

    return 1;
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;
    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    log_info(logger_entrada_salida, "Enviando HANDSHAKE\n");
    return recibir_operacion(socket_cliente);
}


int  presentar_interfaz(int socket_kernel, t_interfaz* interfaz ){
    t_paquete* paquete_interfaz = malloc(sizeof(t_paquete));;
    
    paquete_interfaz = crear_paquete(INTERFAZ_ENVIAR);
 
    agregar_a_paquete(paquete_interfaz, &interfaz->nombre_length, sizeof(interfaz->nombre_length));  
    agregar_a_paquete(paquete_interfaz, interfaz->nombre, interfaz->nombre_length);
    agregar_a_paquete(paquete_interfaz, &(interfaz->tipo), sizeof(interfaz->tipo));
       
    enviar_paquete(paquete_interfaz, socket_kernel);  
  
    return recibir_operacion(socket_kernel);

}
    //INICIAR INTERFACE CORRESPONDIENTE
void iniciar_interface(char* tipo_interfaz_str, char* nombre_interfaz,  int socket_kernel, int socket_memoria){
 
t_tipo_interfaz_enum tipo_interfaz_enum = obtener_tipo_interfaz_enum (tipo_interfaz_str);   
 interfaz = malloc(sizeof(t_interfaz)); // no olvidar liberar memoria al finalizar
 interfaz->nombre_length = string_length(nombre_interfaz) + 1;
 interfaz->nombre = nombre_interfaz;
 interfaz->tipo = tipo_interfaz_enum;

log_info(logger_entrada_salida, "dentro iniciar interface\n");

//HANDSHAKE//   
    if ( (hacer_handshake (socket_kernel) == HANDSHAKE_OK)){
        log_info(logger_entrada_salida, "Correcto en handshake con kernel\n");      
    }
    else {
        log_info(logger_entrada_salida, "Error en handshake con kernel\n");
        return EXIT_FAILURE;
    }


    if ( (hacer_handshake (socket_memoria) == HANDSHAKE_OK)){
        log_info(logger_entrada_salida, "Correcto en handshake con memoria\n");
    }
    else {
        log_info(logger_entrada_salida, "Error en handshake con memoria\n");
        return EXIT_FAILURE;
    }

// PRESENTAR    
    if ( (presentar_interfaz (socket_kernel, interfaz) == INTERFAZ_RECIBIDA)){
        log_info(logger_entrada_salida, "Interfaz presentada correctamente a kernel\n");      
    }
    else {
        log_info(logger_entrada_salida, "Error al recibir interfaz desde kernel \n");
        return EXIT_FAILURE;
    }

      switch (tipo_interfaz_enum) {
            
            case GENERICA :
                iniciar_interfaz_generica (socket_kernel);
                break;
            
            case STDIN :

                iniciar_interfaz_stdin (socket_kernel, socket_memoria);
                break;    

            case STDOUT :

                iniciar_interfaz_stdout (socket_kernel, socket_memoria);
                break;  

            case DIALFS :

                log_info(logger_entrada_salida, "Interfaz DIALFS iniciada");
                break;                             

            default:
                log_error(logger_entrada_salida, "Algo anduvo mal en el inicio de interface ");
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