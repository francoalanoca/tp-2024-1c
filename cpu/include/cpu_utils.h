#ifndef TP_2024_1C_PASARONCOSAS_CPU_UTILS_H
#define TP_2024_1C_PASARONCOSAS_CPU_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>
#include <utils/hello.h>
#include<commons/config.h>
#include <pthread.h>

typedef enum
{
    PC,
	AX,
	BX,
	CX,
	DX,
    EAX,
    EBX,
    ECX,
    EDX,
    SI,
    DI,
    REG_NO_ENC
}registros;

instr_t* fetch(int conexion, t_log* logger, t_config* config, t_proceso* proceso);
tipo_instruccion decode(instr_t* instr);
void execute(t_log* logger, t_config* config, instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso);
void check_interrupt();
void pedir_instruccion(t_proceso* proceso,int conexion, t_log* logger);
void set(char* registro, uint32_t valor, t_proceso* proceso, t_log *logger);
void sum(char* registro_destino, char* registro_origen, t_proceso* proceso, t_log *logger);
void sub(char* registro_destino, char* registro_origen, t_proceso* proceso, t_log *logger);
void jnz(char* registro, uint32_t inst, t_proceso* proceso, t_log* logger);
void io_gen_sleep(char* interfaz, uint32_t unidades_de_trabajo, t_proceso* proceso);
instr_t* pedir_inst_a_memoria(int pc, int valor);
bool verificar_interrupcion_kernel();
void generar_interrupcion_a_kernel(int conexion);
//t_interfaz elegir_interfaz(char* interfaz,t_proceso* proceso);
t_buffer *buffer_create(uint32_t size);
void buffer_destroy(t_buffer *buffer);
void buffer_add(t_buffer *buffer, void *data, uint32_t size);
void buffer_add_uint32(t_buffer *buffer, uint32_t data);
void buffer_add_uint8(t_buffer *buffer, uint8_t data);
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
void buffer_add_t_tipo_interfaz_enum(t_buffer *buffer, t_tipo_interfaz_enum* data);
void buffer_add_pcb(t_buffer* buffer,  t_pcb* pcb);
void buffer_add_instruccion(t_buffer* buffer, instr_t* instruccion);
void calcularTamanioInstruccion(instr_t* instruccion);
void calcularTamanioInterfaz(t_interfaz* interfaz);
void buffer_add_interfaz(t_buffer* buffer, t_interfaz* interfaz);
t_buffer *proceso_serializar(t_proceso* proceso);
t_proceso_memoria* crear_proceso_memoria(t_proceso* proceso);
t_buffer *proceso_memoria_serializar(t_proceso_memoria* proceso_memoria, t_log* logger);
t_proceso_interrumpido* crear_proceso_interrumpido(t_proceso* proceso, char* motivo);
t_buffer *proceso_interrumpido_serializar(t_proceso_interrumpido* proceso_interrumpido);
void* crear_servidor_dispatch(char* ip_cpu);
void* crear_servidor_interrupt(char* ip_cpu);
registros identificarRegistro(char* registro);
uint32_t obtenerValorActualRegistro(registros id_registro, t_proceso* proceso, t_log* logger);
t_interfaz* elegir_interfaz(char* interfaz, t_proceso* proceso);
void enviar_interfaz_a_kernel(t_interfaz* interfaz_elegida,uint32_t unidades_de_trabajo);
t_buffer* envio_interfaz_serializar(t_interfaz* interfaz_elegida, uint32_t unidades_de_trabajo);


#endif //TP_2024_1C_PASARONCOSAS_CPU_UTILS_H