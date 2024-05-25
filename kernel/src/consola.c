#include "consola.h"

 int conexion_memoria;
 int identificador_pid;
 pthread_mutex_t mutex_pid;

//Funcion que implementa el inicio de la consola iterativa
void iniciar_consola_interactiva(int conexion){
    conexion_memoria = conexion;
    char* leido;
    //Asignamos el caracter > a la variable leido
	leido = readline("> ");
	log_info(logger_kernel, leido);
    bool validacion_leido;

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while (strcmp(leido, "\0") != 0){

        //paso lo leido a validar
        validacion_leido = validacion_de_instruccion_de_consola(leido);

        //Si lo validado no fue reconocido pasa por el if
        if (!validacion_leido){

            log_error(logger_kernel, "Comando de CONSOLA no reconocido");
            free(leido);
            leido = readline("> ");
            log_info(logger_kernel, leido);
            continue;   //salto el resto del while
        }
        
        //Una vez validado paso a atender lo leido
        atender_instruccion_validada(leido);
		free(leido);
        leido = readline("> ");
	}

	free(leido);
}


//FUncnion que evalua lo ingresado por consola y verifica si es alguno de los protocolos
bool validacion_de_instruccion_de_consola(char* leido){
    bool resultado_validacion = false;

    //Paso lo leido por consola a un vector para saber luego si es un comando de consola
    char** comando_consola = string_split(leido, "");

    //Falta hacer mas controles de validacion en el caso de los parametros, que no llegue vacio, sea la cantidad indicada y no mas por ejemplo
    //Comparo si lo leido coincide con alguno de los comandos
    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){
        resultado_validacion = true;
    }else{
        log_error(logger_kernel, "Comando no reconocido");
        resultado_validacion = false;
    }
    
    string_array_destroy(comando_consola);

    return resultado_validacion;
}



void atender_instruccion_validada(char* leido){
    char** comando_consola = string_split(leido, "");
    t_buffer* un_buffer = crear_buffer();

    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){    //EJECUTAR_SCRIPT [PATH]
        cargar_string_al_buffer(un_buffer, comando_consola[1]);     //[PATH]

        //Procedo a ejecuratar el script

    }else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){  //INICIAR_PROCESO [PATH]
        cargar_string_al_buffer(un_buffer, comando_consola[1]);     //[PATH]
        //cargar_string_al_buffer(un_buffer, comando_consola[2]);   //Si el protocolo tuviera un 2do parametro [SIZE] se descomenta esto
        //cargar_string_al_buffer(un_buffer, comando_consola[3]);   //Si el protocolo tuviera un 3er parametro [PRIORIDAD] se descomenta esto

        //Procedo a iniciar el proceso
        f_iniciar_proceso(un_buffer);

    }else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){    //FINALIZAR_PROCESO [PID]
        
    }else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){    //DETENER_PLANIFICACION
        
    }else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){    //INICIAR_PLANIFICACION
        
    }else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){    //MULTIPROGRAMACION [VALOR]
        
    }else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){   //PROCESO_ESTADO
        
    }else{
        log_error(logger_kernel, "Comando no reconocido que logro pasar el filtro!!!");
        exit(EXIT_FAILURE);
    }
    
    string_array_destroy(comando_consola);
    
}

//Falta logica para el funcionamiento
//Funcion que carga las instrucciones de path para iniciar un proceso en New
void f_iniciar_proceso(t_buffer* un_buffer){

    //extraemos del buffer
    char* path = extraer_string_del_buffer(un_buffer);
    //char* size = extraer_string_del_buffer(un_buffer);    //si el protocolo tuviera parametro size
    //char* prioridad = extraer_string_del_buffer(un_buffer);   //sie l protocolo tuviera parametro prioridad
    log_trace(logger_kernel, "BUFFER (%d): [PATH: &s]", path);
    eliminar_buffer(un_buffer);

    int pid = asignar_pid();
    //int size_num = atoi(size);

    t_paquete* un_paquete = crear_paquete(CREAR_PROCESO_KERNEL);



    enviar_paquete(un_paquete, conexion_memoria);
    eliminar_paquete(un_paquete);

}


//Funcion que asigna un pid
int asignar_pid(){
    
    int valor_pid;
    //Se crea un mutex en el caso de que ingrese mas de un proceso
    pthread_mutex_lock(&mutex_pid);
    valor_pid = identificador_pid;
    identificador_pid++;
    pthread_mutex_unlock(&mutex_pid);

    return valor_pid;
}