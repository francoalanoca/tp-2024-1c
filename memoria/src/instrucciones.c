#include "../include/instrucciones.h"






//Funcion que crea la lista de instrucciones dado un puntero al archivo de un proceso
void leer_instrucciones(char* nombre_archivo, uint32_t proceso_pid) {

	t_miniPCB *miniPCB = malloc(sizeof(t_miniPCB));

    miniPCB->pid = proceso_pid;
    miniPCB->lista_de_instrucciones = list_create();

	//Creamos una variable que gurada el path entero: path_instrucciones/nombre
	char* path_total = string_new();
	string_append(&path_total, cfg_memoria->PATH_INSTRUCCIONES);
	string_append(&path_total, "/");
	string_append(&path_total, nombre_archivo);

    //creamos una variable que guarda el archivo
    FILE* archivo = fopen(path_total, "r");		//tengo problemas con ese fopen
    log_info(logger_memoria, "%s", nombre_archivo);

	//Si no se puede abrir el archivo marca error
	if (!archivo){
        log_error(logger_memoria, "Error al abrir el archivo %s.", path_total);
        exit(EXIT_FAILURE);
    }

	char *linea;
	//instr_t *inst;
	//t_list *instrucciones = list_create();
	size_t len;

	linea = string_new();
	
	len = 0;

    //mientras no sea el fin del archivo
	//devuelve la cnatidad de stram leidos
	while(getline(&linea, &len, archivo) != -1) {
		
		if (strcmp(linea, "EXIT") == 0)
        {

            int tamanio_del_nombre = strlen(linea) + 1;

            char *linea2 = malloc(tamanio_del_nombre);
            strcpy(linea2, linea);

            list_add(miniPCB->lista_de_instrucciones, linea2);
        }
        else
        {

            int tamanio_del_nombre = strlen(linea);
            linea[tamanio_del_nombre - 1] = '\0';

            char *linea2 = malloc(tamanio_del_nombre);
            strcpy(linea2, linea);

            list_add(miniPCB->lista_de_instrucciones, linea2);
        }
	}
	list_add(lista_miniPCBs, miniPCB);

    fclose(archivo);
}





//Funcion que busca una instruccion en base a un pid y pc de un proceso
char *buscar_instruccion(int proceso_pid, int program_counter){

	//Mientras se menor al tamaño de la lista hace el bucle
    for (int i = 0; i < list_size(lista_miniPCBs); i++){

		//Creamos una variable a la que le asignamos elementos de la lista
        t_miniPCB *miniPCB = list_get(lista_miniPCBs, i);

		//Si la varialbe es igual al proceso buscado
        if (miniPCB->pid == proceso_pid){

			//GUarmamos valor y lo retornamos
            char *valor = list_get(miniPCB->lista_de_instrucciones, program_counter);
            return valor;
        }
    }

    return NULL;
}

