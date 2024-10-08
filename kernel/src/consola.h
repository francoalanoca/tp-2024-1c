#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>
#include <init_kernel.h>
#include <utils/utils.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>
#include <string.h>

extern int identificador_pid;
extern int contador_pcbs;
extern pthread_mutex_t mutex_process_id = PTHREAD_MUTEX_INITIALIZER;
extern uint32_t process_id = 0;

void iniciar_consola_interactiva(int conexion);
bool validacion_de_instruccion_de_consola(char* leido);
void atender_instruccion_validada(char* leido);
void f_iniciar_proceso(t_buffer* un_buffer);
t_pcb* crear_pcb(char* path);
void imprimir_pcb(t_pcb* un_pcb);
void enviar_pcb_a_cpu_por_dispatch(t_pcb* una_pcb);
void destruir_pcb(t_pcb* un_pcb);
void cambiar_estado(t_pcb* un_pcb, estado_pcb prox_estado);

#endif