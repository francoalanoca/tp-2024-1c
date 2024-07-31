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

instr_t* fetch(int conexion, t_pcb* proceso);
tipo_instruccion decode(instr_t* instr);
void execute(instr_t* inst,tipo_instruccion tipo_inst, t_pcb* proceso, int conexion,t_list* tlb,  int socket_dispatch, int socket_interrupt);
void check_interrupt( int conexion_kernel);
void pedir_instruccion(t_pcb* proceso,int conexion);
void set(char* registro, uint32_t valor, t_pcb* proceso);
void sum(char* registro_destino, char* registro_origen, t_pcb* proceso);
void sub(char* registro_destino, char* registro_origen, t_pcb* proceso);
void jnz(char* registro, uint32_t inst, t_pcb* proceso);
void io_gen_sleep(char* nombre_interfaz, uint32_t unidades_de_trabajo, t_pcb* proceso, int conexion_kernel );
void generar_interrupcion_a_kernel(int conexion);
//t_proceso_memoria* crear_proceso_memoria(t_proceso* proceso);
void* crear_servidor_dispatch(char* ip_cpu);//
void* crear_servidor_interrupt(char* ip_cpu);//
registros identificarRegistro(char* registro);
uint32_t obtenerValorActualRegistro(registros id_registro, t_pcb* proceso);
//t_interfaz* elegir_interfaz(char* interfaz, t_proceso* proceso);
void enviar_interfaz_a_kernel(char* nombre_interfaz, uint32_t tamanio_nombre, uint32_t unidades_de_trabajo, int conexion);
uint32_t mmu(uint32_t direccion_logica, uint32_t tamanio_pag, int conexion, t_log* logger,t_list* tlb);
bool verificar_existencia_en_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t indice);
//char* uint32_to_string(uint32_t number);
//char* concatenar_cadenas(const char* str1, const char* str2);
//uint32_t string_a_uint32(const char* str);
void pedir_marco_a_memoria(uint32_t pid, uint32_t nro_pagina, int conexion);
void agregar_a_tlb(uint32_t pid, uint32_t nro_pag, uint32_t marco, t_list* tlb);
void mov_in(char* registro_datos, char* registro_direccion, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb);
void mov_out(char* registro_direccion, char* registro_datos, t_pcb* proceso, t_log* logger,int conexion, t_list* tlb);
void resize(uint32_t tamanio, int conexion, int conexion_kernel);
void copy_string(uint32_t tamanio, t_log* logger, int conexion, t_list* tlb);
void wait_inst(char* recurso, int conexion_kernel);
void signal_inst(char* recurso, int conexion_kernel);
void io_stdin_read(char* interfaz, char* registro_direccion, char* registro_tamanio, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb, int conexion_kernel);
void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid, uint32_t tamanio, int conexion);
void io_stdout_write(char* interfaz, char* registro_direccion, char* registro_tamanio, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb, int conexion_kernel);
void exit_inst( int conexion_kernel);
void guardar_en_direccion_fisica(uint32_t dir_fisica_result,uint32_t tamanio_valor_datos,char* valor_registro_datos, uint32_t pid, int conexion);
void solicitar_resize_a_memoria(uint32_t* pid, uint32_t tamanio, int conexion);
void envia_error_de_memoria_a_kernel(t_proceso_interrumpido* proceso, int conexion_kernel);
void guardar_string_en_memoria(char* valor_a_enviar,uint32_t tamanio_valor,uint32_t direccion, uint32_t pid, int conexion);
void solicitar_wait_kernel(t_pcb* pcb,uint32_t recurso_tamanio ,char* recurso, int conexion_kernel);
void solicitar_signal_kernel(t_pcb* pcb,uint32_t recurso_tamanio,char* recurso, int conexion_kernel);
void solicitar_io_stdin_read_a_kernel(uint32_t tamanio_nombre_interfaz,char* nombre_interfaz, uint32_t direccion, uint32_t tamanio, int conexion_kernel);
void solicitar_io_stdout_write_a_kernel(uint32_t tamanio_nombre_interfaz, char* nombre_interfaz, uint32_t direccion, uint32_t tamanio, int conexion_kernel);
void solicitar_exit_a_kernel(t_proceso_interrumpido* proceso, int conexion_kernel);
void usar_algoritmo_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t nro_marco, t_list* tlb);
uint32_t valor_repetido_tlb(uint32_t pid, uint32_t nro_pag);
void obtenerTamanioPagina(int conexion);
void io_fs_create(char* interfaz, char* nombre_archivo, t_pcb* proceso, t_log* logger, int conexion_kernel);
void io_fs_delete(char* interfaz, char* nombre_archivo, t_pcb* proceso, t_log* logger, int conexion_kernel);
void io_fs_truncate(char* interfaz, char* nombre_archivo, char* registro_tamanio, t_pcb* proceso, t_log* logger , int conexion_kernel);
void io_fs_write(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb, int conexion_kernel);
void io_fs_read(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb, int conexion_kernel);
void enviar_io_fs_create_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t pid, int conexion_kernel);
void enviar_io_fs_delete_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t pid, int conexion_kernel);
void enviar_io_fs_truncate_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t tamanio,uint32_t pid, int conexion_kernel);
void enviar_io_fs_write_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t valor_tamanio,uint32_t direccion_fisica,uint32_t puntero_archivo,uint32_t pid, int conexion_kernel);
void enviar_io_fs_read_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t valor_tamanio,uint32_t direccion_fisica,uint32_t puntero_archivo,uint32_t pid, int conexion_kernel);
void limpiarCadena(char* cadena);
uint32_t obtenerTamanioRegistro(registros id_registro);
#endif //TP_2024_1C_PASARONCOSAS_CPU_UTILS_H