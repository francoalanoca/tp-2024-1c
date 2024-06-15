#include "../include/instrucciones.h"


//Vector de operaciones para comparar
char *op_strings[] = {"SET", "SUM", "SUB", "MOV_IN", "MOV_OUT", "RESIZE", "JNZ", "COPY_STRING", 
                        "IO_GEN_SLEEP", "IO_STDIN_READ", "IO_STDOUT_WRITE", "IO_FS_CREATE", "IO_FS_DELETE", 
                        "IO_FS_TRUNCATE", "IO_FS_WRITE", "IO_FS_READ", "WAIT", "SIGNAL", "EXIT"};


/*
//Funcion que crea la lista de instrucciones dado un puntero al archivo de un proceso
t_list *leer_instrucciones(char* nombre_archivo) {

    //creamos una variable que guarda el archivo
    FILE* archivo = leer_archivo(nombre_archivo);
    log_info(logger_memoria, "%s", nombre_archivo);

	char *linea, *tokens;
	instr_t *inst;
	t_list *instrucciones = list_create();
	size_t len;

    //mientras no sea el fin del archivo
	while(!feof(archivo)) {
		linea = string_new();
		tokens = string_array_new();
		len = 0;

        //devuelve la cnatidad de stram leidos
		getline(&linea, &len, archivo);
		//log_info(logger_memoria, "%s", linea);
        //separo por el " " n veces posibles y lo guardo en la variable
		tokens = string_n_split(linea, 3, " ");

        //dependiendo del tamaño del vector creamos las instrucciones
        //puede llegar a haber hasta 5 parametros pero de momento probamos con 2
		switch(string_array_size(tokens)) {
		case 1:
			inst = crear_instruccion(string_a_op_code(tokens[0]), "\0", "\0");
			break;
		case 2:
			inst = crear_instruccion(string_a_op_code(tokens[0]), tokens[1], "\0");
			break;
		case 3:
			inst = crear_instruccion(string_a_op_code(tokens[0]), tokens[1], tokens[2]);
/*
            break;
		case 4:
			inst = crear_instruccion(string_a_op_code(tokens[0]), tokens[1], tokens[2], tokens[3], "\0");
            break;
		case 5:
			inst = crear_instruccion(string_a_op_code(tokens[0]), tokens[1], tokens[2], tokens[3], tokens[4]); 

		}

        //pasamos las instrucciones creadas a la variable para luego retornarla
		list_add(instrucciones, inst);
		free(linea);
		string_array_destroy(tokens);
	}

	return instrucciones;
}


//Funcion que abre y devuelve el archivo en modo lectura
FILE* leer_archivo(char* nombre){
    FILE *archivo = fopen(nombre, "r");

    return archivo;
}



//Funcion que crea una instruccion a partir de la estructura y parametros ingresados
instr_t *crear_instruccion(tipo_instruccion op_code, char* param1, char* param2) {
	instr_t *instruccion = malloc(sizeof(instr_t));
	instruccion->identificador = op_code;
	instruccion->param1 = string_duplicate(param1);
	instruccion->param2 = string_duplicate(param2);
    // instruccion->segundo_operando = string_duplicate(tercer_operando);
    // instruccion->segundo_operando = string_duplicate(cuarto_operando);
    // instruccion->segundo_operando = string_duplicate(quinto_operando);

	return instruccion;
}






tipo_instruccion string_a_op_code(char* str) {

    //creamos una variable que guarda el tamaño del array
	int op_codes_size = string_array_size(op_strings);
	tipo_instruccion i;

	for(i = 0; i < op_codes_size; i++) {
        
        //si los string son iguales retorna la instruccion
		if(string_equals_ignore_case(str, op_strings[i]))
			return i;
	}
	return -1;
}
*/