#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"


char *path_config;

extern t_proceso* proceso_actual;


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

     
   ciclo_de_instrucciones(socket_memoria,logger_cpu,cfg_cpu,proceso_actual); //TODO: Crear esta funcion, para el fetch mandar mensaje a memoria usando PROXIMA_INSTRUCCION


	


	terminar_programa(socket_memoria, logger_cpu, cfg_cpu);
    return 0;
}

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_proceso* proceso){
    instr_t *inst = malloc(sizeof(instr_t));
    inst = fetch(conexion,logger,config,proceso);
    tipo_instruccion tipo_inst; //TODO: ver como hacer malloc
    tipo_inst = decode(inst);//TODO: ver como funciona
    execute(logger, config,inst,tipo_inst, proceso);
    check_interrupt();
    proceso_actual->pcb->program_counter += 1;

}

instr_t* fetch(int conexion, t_log* logger, t_config* config, t_proceso* proceso){
        instr_t *prox_inst = malloc(sizeof(instr_t));
       prox_inst = pedir_instruccion(proceso);
       return prox_inst;
}

tipo_instruccion decode(instr_t* instr){
    return instr->id;//TODO: VER IMPLEMENTACION
   // return SET;
}


void execute(t_log* logger, t_config* config, instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso){

    switch(tipo_inst){
        case SET:
        {
            set(inst->param1, inst->param2, proceso);
            break;
        }
        case SUM:
        {
            sum(inst->param1, inst->param2,proceso);
            break;
        }
        case SUB:
        {
            sub(inst->param1, inst->param2,proceso);
            break;
        }
        case JNZ:
        {
            jnz(inst->param1, inst->param2,proceso);
            break;
        }
        case IO_GEN_SLEEP:
        {
            io_gen_sleep(inst->param1, inst->param2,proceso);
            break;
        }
    }

}

void check_interrupt(){
    if(verificar_interrupcion_kernel()){
        generar_interrupcion_a_kernel(proceso_actual);//TODO: en esta funcion no se usara dispatch sino interrupt
    }
}

instr_t* pedir_instruccion(t_proceso* proceso){
    return pedir_inst_a_memoria(proceso->pcb->program_counter, PROXIMA_INSTRUCCION);
}

void set(uint32_t registro, uint32_t valor, t_proceso* proceso){
    registro = valor;
}

void sum(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso){
    registro_destino = registro_destino + registro_origen;
}

void sub(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso){
    registro_destino = registro_destino - registro_origen;
}

void jnz(uint32_t registro, uint32_t inst, t_proceso* proceso){
    if(registro != 0){
        proceso_actual-> pcb-> program_counter = inst;
    }
}
//void io_gen_sleep(Interfaz interfaz, int unidades_de_trabajo){ //TODO: VER PARAMETROS
void io_gen_sleep(char* interfaz, int unidades_de_trabajo, t_proceso* proceso){
   // t_interfaz interfaz_elegida = malloc(sizeof(t_interfaz));//REVISAR
   t_interfaz interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    //enviar_interfaz_a_kernel(interfaz_elegida, unidades_de_trabajo);//VER IMPLEMENTACION
    printf("Entra a io_gen_sleep");
}

instr_t* pedir_inst_a_memoria(int pc, int valor){//TODO:DEFINIR
    instr_t *prox_inst = malloc(sizeof(instr_t));
    return prox_inst;
}


bool verificar_interrupcion_kernel(){
    return false;
}

void generar_interrupcion_a_kernel(t_proceso* proceso_actual){
    printf("entro a generar_interrupcion_a_kernel");
}

t_interfaz elegir_interfaz(char* interfaz,t_proceso* proceso){
    t_interfaz interf;
    return interf;

}


