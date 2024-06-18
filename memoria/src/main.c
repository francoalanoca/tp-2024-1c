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


	//-------------------Variables---------------------------
	inicializar_memoria();
	log_info(logger_memoria, "Se inicio correctamente la Memoria");


	//------------------Conexiones------------------------
	iniciar_conexiones();
	log_info(logger_memoria, "Se iniciaron correctamente las conexiones");


	//------------------Hilos------------------------------
	//escuchar_modulos();

	//terminar_programa(fd_memoria, logger_memoria, file_cfg_memoria);
	return 0;
}

// void iterator(char* value) {
// 	log_info(logger_memoria, "%s", value);
// }
