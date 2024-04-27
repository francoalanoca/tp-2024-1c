#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"


char *path_config;


int main(int argc, char* argv[]) {
    decir_hola("CPU");

    path_config = argv[1];

	int socket_memoria;
	char* ip;



     printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;

    }
    
	log_info(logger_cpu, "empieza el programa");
    crear_servidor_dispatch();
    log_info(logger_cpu, "se creo el servidor");
   
     socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);
       
     if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con kernel");
    }
    else {
        log_info(logger_cpu, "Error en handshake con kernel");
        return EXIT_FAILURE;
    }

     
   ciclo_de_instrucciones(socket_memoria,logger_cpu,cfg_cpu); //TODO: Crear esta funcion, para el fetch mandar mensaje a memoria usando PROXIMA_INSTRUCCION


	


	terminar_programa(socket_memoria, logger_cpu, cfg_cpu);
    return 0;
}

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config){
    instr_t *inst = malloc(sizeof(instr_t));
    inst = fetch(conexion,logger,config);
    char* tipo_inst; //TODO: ver como hacer malloc
    tipo_inst = decode();//TODO: ver como funciona
    execute(logger, config,tipo_inst);
    check_interrupt();
    pcb_actual->program_counter += 1;
}

instr_t fetch(int conexion, t_log* logger, t_config* config){
        instr_t *prox_inst = malloc(sizeof(instr_t));
       prox_inst = pedir_instruccion(pcb_actual);
       return prox_inst;
}

void decode(int conexion, t_log* logger, t_config* config){

}

void execute(t_log* logger, t_config* config, char* tipo_inst){

    switch(tipo_inst){
        case "SET":
        {

            break;
        }
        case "SUM":
        {
            
            break;
        }
        case "SUB":
        {
            
            break;
        }
        case "IO_GEN_SLEEP":
        {
            
            break;
        }
    }

}

void check_interrupt(int conexion, t_log* logger, t_config* config){
    if(verificar_interrupcion_kernel()){
        generar_interrupcion_a_kernel(pcb_actual);//TODO: en esta funcion no se usara dispatch sino interrupt
    }
}

instr_t pedir_instruccion(pcb pcb_actual){
    return pedir_inst_a_memoria(pcb_actual->program_counter, PROXIMA_INSTRUCCION);
}


