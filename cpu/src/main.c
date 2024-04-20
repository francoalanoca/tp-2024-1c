#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>

int main(int argc, char* argv[]) {
    decir_hola("CPU");

	int conexion;
	char* ip;
	char* puerto;

	t_log* logger;
	t_config* config;

    logger = log_create("log.log", "CPU", 1, LOG_LEVEL_DEBUG);

	config = iniciar_config_cpu();

	config = config_create("/home/utnso/tp-2024-1c-Pasaron-cosas/cpu/cpu.config");
	
	 if (config == NULL) {
        printf("Error al crear el config, finaliza el programa.\n");
        return 1; 
    }
	

	/*config_set_value(config, "IP_MEMORIA", "127.0.0.1");
    config_set_value(config, "PUERTO_MEMORIA", "8002");
    config_set_value(config, "PUERTO_ESCUCHA_DISPATCH", "8006");
    config_set_value(config, "PUERTO_ESCUCHA_INTERRUPT", "8007");
    config_set_value(config, "CANTIDAD_ENTRADAS_TLB", "32");
    config_set_value(config, "ALGORITMO_TLB", "FIFO");*/

	 if (config_save(config) == -1) {
        printf("Error al guardar el archivo de configuración.\n");
    }

   // log_info(logger, "Valor de la clave 'IP_MEMORIA': %s", config_get_string_value(config, "IP_MEMORIA"));
	conexion = crear_conexion(config_get_string_value(config,"IP_MEMORIA"), config_get_string_value(config,"PUERTO_MEMORIA"));

    //inicio handshake
   // enviar_handshake(conexion);
    size_t bytes;

int32_t handshake = 1;
int32_t result;

bytes = send(conexion, &handshake, sizeof(int32_t), 0);
bytes = recv(conexion, &result, sizeof(int32_t), MSG_WAITALL);

if (result == 0) {
    // Handshake OK
    log_info(logger,"Handshake OK");
} else {
    // Handshake ERROR
    log_info(logger,"Handshake ERROR");
}
    //fin handshake

    enviar_mensaje("MENSAJE A MEMORIA",conexion);
	

	terminar_programa(conexion, logger, config);
    return 0;
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	return nuevo_logger;
}

void enviar_handshake(int conexion)
{
    
}

