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
    tipo_inst = decode(inst);//TODO: ver como funciona
    execute(logger, config,inst,tipo_inst);
    check_interrupt();
    pcb_actual->program_counter += 1;
}

instr_t fetch(int conexion, t_log* logger, t_config* config){
        instr_t *prox_inst = malloc(sizeof(instr_t));
       prox_inst = pedir_instruccion(pcb_actual);
       return prox_inst;
}

void decode(instr_t instr){
    return instr->id;
}

void execute(t_log* logger, t_config* config, instr_t inst,char* tipo_inst){

    switch(tipo_inst){
        case "SET":
        {
            set(inst->param1, inst->param2);
            break;
        }
        case "SUM":
        {
            sum(inst->param1, inst->param2);
            break;
        }
        case "SUB":
        {
            sub(inst->param1, inst->param2);
            break;
        }
        case "JNZ":
        {
            jnz(inst->param1, inst->param2);
            break;
        }
        case "IO_GEN_SLEEP":
        {
            io_gen_sleep(inst->param1, inst->param2);
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

void set(uint32_t registro, uint32_t valor){
    registro = valor;
}

void sum(uint32_t registro_destino, uint32_t registro_origen){
    registro_destino = registro_destino + registro_origen;
}

void sub(uint32_t registro_destino, uint32_t registro_origen){
    registro_destino = registro_destino - registro_origen;
}

void jnz(uint32_t registro, uint32_t inst){
    if(registro != 0){
        pcb_actual-> program_counter = inst;
    }
}
void io_gen_sleep(Interfaz interfaz, int unidades_de_trabajo){
    solicitar_envio_interfaz(interfaz,unidades_de_trabajo);
}

