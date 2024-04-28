#ifndef TP_2024_1C_PASARONCOSAS_MAIN_H
#define TP_2024_1C_PASARONCOSAS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>

typedef enum
{
 //----------------BASICOS--------------------------------
    SET,
	SUM,
	SUB,
	JNZ,
	IO_GEN_SLEEP
}tipo_instruccion;

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config);
instr_t* fetch(int conexion, t_log* logger, t_config* config);
tipo_instruccion decode(instr_t* inst);
void execute(t_log* logger, t_config* config, instr_t* inst,tipo_instruccion tipo_inst);
void check_interrupt();
instr_t* pedir_instruccion(t_pcb* pcb_actual);
void set(uint32_t registro, uint32_t valor);
void sum(uint32_t registro_destino, uint32_t registro_origen);
void sub(uint32_t registro_destino, uint32_t registro_origen);
void jnz(uint32_t registro, uint32_t inst);
void io_gen_sleep();
instr_t* pedir_inst_a_memoria(int pc, int valor);
bool verificar_interrupcion_kernel();
void generar_interrupcion_a_kernel(t_pcb* pcb_actual);



#endif //TP_2024_1C_PASARONCOSAS_MAIN_H