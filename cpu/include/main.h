#ifndef TP_2024_1C_PASARONCOSAS_MAIN_H
#define TP_2024_1C_PASARONCOSAS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>



//----------------BASICOS--------------------------------

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_proceso* proceso);
instr_t* fetch(int conexion, t_log* logger, t_config* config, t_proceso* proceso);
tipo_instruccion decode(instr_t* instr);
void execute(t_log* logger, t_config* config, instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso);
void check_interrupt();
instr_t* pedir_instruccion(t_proceso* proceso);
void set(uint32_t registro, uint32_t valor, t_proceso* proceso);
void sum(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso);
void sub(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso);
void jnz(uint32_t registro, uint32_t inst, t_proceso* proceso);
void io_gen_sleep(char* interfaz, int unidades_de_trabajo, t_proceso* proceso);
instr_t* pedir_inst_a_memoria(int pc, int valor);
bool verificar_interrupcion_kernel();
void generar_interrupcion_a_kernel(t_proceso* proceso_actual);
t_interfaz elegir_interfaz(char* interfaz,t_proceso* proceso);



#endif //TP_2024_1C_PASARONCOSAS_MAIN_H