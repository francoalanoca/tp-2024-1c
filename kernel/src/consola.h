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


extern int identificador_pid;
extern int contador_pcbs;
extern pthread_mutex_t mutex_pid;

void iniciar_consola_interactiva(int conexion);
bool validacion_de_instruccion_de_consola(char* leido);
void atender_instruccion_validada(char* leido);
void f_iniciar_proceso(t_buffer* un_buffer);
int asignar_pid();

#endif