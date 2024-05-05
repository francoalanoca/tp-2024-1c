#ifndef TP_2024_1C_PASARONCOSAS_MAIN_H
#define TP_2024_1C_PASARONCOSAS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>


//t_proceso* proceso_actual = NULL;

//----------------BASICOS--------------------------------

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_proceso* proceso);
instr_t* fetch(int conexion, t_log* logger, t_config* config, t_proceso* proceso);
tipo_instruccion decode(instr_t* instr);
void execute(t_log* logger, t_config* config, instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso);
void check_interrupt();
instr_t* pedir_instruccion(t_proceso* proceso,int conexion);
void set(uint32_t registro, uint32_t valor, t_proceso* proceso);
void sum(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso);
void sub(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso);
void jnz(uint32_t registro, uint32_t inst, t_proceso* proceso);
void io_gen_sleep(char* interfaz, int unidades_de_trabajo, t_proceso* proceso);
instr_t* pedir_inst_a_memoria(int pc, int valor);
bool verificar_interrupcion_kernel();
void generar_interrupcion_a_kernel(t_proceso* proceso_actual);
t_interfaz elegir_interfaz(char* interfaz,t_proceso* proceso);
t_buffer *buffer_create(uint32_t size);
void buffer_destroy(t_buffer *buffer);
void buffer_add(t_buffer *buffer, void *data, uint32_t size);
void buffer_add_uint32(t_buffer *buffer, uint32_t data);
void buffer_add_uint8(t_buffer *buffer, uint8_t data);
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
void buffer_add_pcb(t_buffer* buffer,  t_pcb* pcb);
void buffer_add_instruccion(t_buffer* buffer, instr_t* instruccion);
void calcularTamanioInstruccion(instr_t* instruccion);
void calcularTamanioInterfaz(t_interfaz* interfaz);
void buffer_add_interfaz(t_buffer* buffer, t_interfaz* interfaz);
t_buffer *proceso_serializar(t_proceso* proceso);




#endif //TP_2024_1C_PASARONCOSAS_MAIN_H