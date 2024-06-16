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
#include <math.h>


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
uint32_t mmu(uint32_t direccion_logica, uint32_t tamanio_pag, int conexion);
bool verificar_existencia_en_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t indice);
//char* uint32_to_string(uint32_t number);
//char* concatenar_cadenas(const char* str1, const char* str2);
//uint32_t string_a_uint32(const char* str);
void pedir_marco_a_memoria(uint32_t pid, uint32_t nro_pagina, int conexion);
t_buffer* busqueda_marco_serializar(t_busqueda_marco* busqueda_marco);
void agregar_a_tlb(uint32_t pid, uint32_t nro_pag, uint32_t marco);
void mov_in(char* registro_datos, char* registro_direccion, t_proceso* proceso, t_log* logger);
void mov_out(char* registro_direccion, char* registro_datos, t_proceso* proceso, t_log* logger);
void resize(uint32_t tamanio);
void copy_string(uint32_t tamanio);
void wait_inst(char* recurso);
void signal_inst(char* recurso);
void io_stdin_read(char* interfaz, char* registro_direccion, char* registro_tamanio, t_proceso* proceso, t_log* logger);
void io_stdout_write(char* interfaz, char* registro_direccion, char* registro_tamanio, t_proceso* proceso, t_log* logger);
void exit_inst();
void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid);
t_buffer* direccion_fisica_serializar(uint32_t dir_fisica,uint32_t pid);
void guardar_en_direccion_fisica(uint32_t dir_fisica_result, uint32_t valor_registro_datos, uint32_t pid);
t_buffer* direccion_fisica_valor_serializar(uint32_t dir_fisica, uint32_t valor, uint32_t pid);
void solicitar_resize_a_memoria(t_proceso* proceso, uint32_t tamanio);
t_buffer* proceso_resize_serializar(t_proceso* proceso, uint32_t tamanio);
void envia_error_de_memoria_a_kernel(t_proceso* proceso);
void guardar_string_en_memoria(char* valor_a_enviar,uint32_t tamanio_valor,uint32_t direccion, uint32_t pid);
t_buffer* copy_string_serializar(char* valor_a_enviar,uint32_t tamanio_valor,uint32_t direccion, uint32_t pid);
void solicitar_wait_kernel(t_proceso* proceso, char* recurso);
t_buffer* proceso_recurso_serializar(t_proceso* proceso, char* registro);
uint32_t calcular_tamanio_proceso(t_proceso* proceso);
void solicitar_signal_kernel(t_proceso* proceso, char* recurso);
void solicitar_io_stdin_read_a_kernel(char* interfaz, uint32_t direccion, uint32_t tamanio);
t_buffer* direccion_tamanio_interfaz_serializar(char* interfaz, uint32_t direccion, uint32_t tamanio);
void solicitar_io_stdout_write_a_kernel(char* interfaz, uint32_t direccion, uint32_t tamanio);
void solicitar_exit_a_kernel(t_proceso* proceso);
void usar_algoritmo_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t nro_marco);
uint32_t valor_repetido_tlb(uint32_t pid, uint32_t nro_pag);
void obtenerTamanioPagina(int conexion);
void io_fs_create(char* interfaz, char* nombre_archivo, t_proceso* proceso, t_log* logger);
void io_fs_delete(char* interfaz, char* nombre_archivo, t_proceso* proceso, t_log* logger);
void io_fs_truncate(char* interfaz, char* nombre_archivo, char* registro_tamanio, t_proceso* proceso, t_log* logger);
void io_fs_write(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo, t_proceso* proceso, t_log* logger);
void io_fs_read(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo, t_proceso* proceso, t_log* logger);
void enviar_io_fs_create_a_kernel(t_interfaz* interfaz_elegida,char* nombre_archivo,uint32_t pid);
void enviar_io_fs_delete_a_kernel(t_interfaz* interfaz_elegida,char* nombre_archivo,uint32_t pid);
void enviar_io_fs_truncate_a_kernel(t_interfaz* interfaz_elegida,char* nombre_archivo,uint32_t valor_tamanio,uint32_t pid);
void enviar_io_fs_write_a_kernel(t_interfaz* interfaz_elegida,char* nombre_archivo,uint32_t valor_tamanio,uint32_t direccion_fisica,uint32_t puntero_archivo,uint32_t pid);
void enviar_io_fs_read_a_kernel(t_interfaz* interfaz_elegida,char* nombre_archivo,uint32_t valor_tamanio,uint32_t direccion_fisica,uint32_t puntero_archivo,uint32_t pid);

#endif //TP_2024_1C_PASARONCOSAS_CPU_UTILS_H