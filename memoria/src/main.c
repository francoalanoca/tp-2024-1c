#include "../include/main.h"


int main(int argc, char* argv[]) {

	//char* path_config = argv[1];
	char* path_config = "memoria.config";


	if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar Memoria");
        return EXIT_FAILURE;

    }

	log_info(logger_memoria, "Se iniciaron correctamente las configuraciones");


	/*
	bool flag_conexion_activa;
	char* nombre = "cliente";

	// Hacer los if correspondientes en caso de fallar
    logger_memoria = log_create("memoria.log", "Memoria LOG", 1, LOG_LEVEL_DEBUG);
	config = iniciar_config(pathconf, logger_memoria);
	log_info(logger_memoria, "Se iniciaron correctamente las configuraciones iniciales");


	if (config_save(config) == -1) {
        printf("Error al guardar el archivo de configuración.\n");
    }

	puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	*/

	//------------------Conexiones------------------------
	//int server_fd = iniciar_servidor_memoria(logger_memoria,puerto);
	//int fd_memoria = iniciar_servidor(logger_memoria,"SERVER MEMORIA", "8002",puerto_escucha);
	//log_info(logger_memoria, "Servidor listo para recibir al cliente");

	//iniciar_conexiones();
	//log_info(logger_memoria, "Se iniciaron correctamente las conexiones");


	//------------------Hilos------------------------------
	//escuchar_modulos();

	//terminar_programa();
	return 0;
}

// void iterator(char* value) {
// 	log_info(logger_memoria, "%s", value);
// }
