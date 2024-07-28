#include "../include/main.h"


int main(int argc, char* argv[]) {

	char* path_config = argv[1];
	


	if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar Memoria");
        return EXIT_FAILURE;

    }

	log_info(logger_memoria, "Se iniciaron correctamente las configuraciones");


	//-------------------Variables---------------------------
	inicializar_memoria();
	log_info(logger_memoria, "Se inicio correctamente la Memoria");


	//------------------Server------------------------
	iniciar_servidor_memoria();

	cerrar_programa();
	return 0;
}
