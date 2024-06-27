#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>
#include "../include/init_kernel.h"
//#include <init_kernel.h>
#include <utils/utils.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>
#include <string.h>
#include "../include/planificador.h"
//#include <planificador.h>


extern int identificador_pid;
extern int contador_pcbs;
//extern pthread_mutex_t mutex_process_id = PTHREAD_MUTEX_INITIALIZER;
extern pthread_mutex_t mutex_lista_procesos; // Mutex para proteger el acceso a la lista de procesos
extern t_list* lista_procesos; // Lista global de procesos
extern int grado_multiprogramacion; // Variable global que almacena el grado de multiprogramacion
extern t_list* lista_procesos_listos; // Lista global de procesos en estado listo
extern pthread_mutex_t mutex_lista_procesos_listos; // Mutex para proteger el acceso a la lista de procesos listos
//extern uint32_t process_id = 0;
extern pthread_mutex_t mutex_process_id; // Declaración externa
extern int process_id; // Declaración externa

void iniciar_consola_interactiva(int conexion);
bool validacion_de_instruccion_de_consola(char* leido);
void atender_instruccion_validada(char* leido);
void f_ejecutar_script(char* path);
//t_planificador *inicializar_planificador(t_algoritmo_planificacion algoritmo, uint32_t quantum);
void detener_planificacion(t_planificador* planificador);
void mostrar_estado_proceso(pid_t pid);
void ajustar_multiprogramacion(int nuevo_valor);
void f_iniciar_proceso(t_buffer* un_buffer);
char* extraer_string_del_buffer(un_buffer);
t_pcb* crear_pcb(char* path,char* nombre);
void imprimir_pcb(t_pcb* pcb);
void enviar_pcb_a_cpu_por_dispatch(t_pcb* pcb);
void destruir_pcb(t_pcb* pcb);
void cambiar_estado(t_pcb* un_pcb, estado_pcb prox_estado);
bool encontrar_por_pid(void* elemento, void* pid_ptr);
t_pcb* obtener_proximo_proceso(t_planificador* planificador);



#endif