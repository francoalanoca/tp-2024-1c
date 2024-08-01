#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include <stdbool.h>
#include <stdlib.h>
#include <utils/utils.h>
#include "../include/init_kernel.h"
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>


extern sem_t sem_planificar;
extern t_temporal* cronometro;

// Inicializa un nuevo planificador
t_planificador* inicializar_planificador(t_algoritmo_planificacion algoritmo, int quantum, int grado_multiprogramacion);
// Destruye el planificador y libera la memoria

typedef struct {
    int quantum;
    int pid;
}t_args_fin_q;

void destruir_planificador(t_planificador* planificador);
bool agregar_proceso(t_planificador* planificador, t_pcb* proceso);
t_algoritmo_planificacion obtener_algoritmo_planificador( char* algoritmo_planificacion);
t_pcb* obtener_proximo_proceso(t_planificador* planificador);
void desalojar_proceso(t_planificador* planificador, t_pcb* proceso);
void bloquear_proceso(t_planificador* planificador, t_proceso_data* proceso_data, char* nombre_lista);
void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso, char* nombre_lista);
void finalizar_proceso(t_planificador* planificador, t_pcb* proceso);
void crear_proceso(t_planificador* planificador, char* path_pseudocodigo);
void eliminar_proceso(t_planificador* planificador, t_pcb* proceso);
void detener_planificacion(t_planificador* planificador);
uint32_t encontrar_indice_proceso_pid(t_list * lista_procesos , t_pcb* pcb);
uint32_t encontrar_indice_proceso_data_pid(t_list * lista_procesos_data , t_pcb* pcb);
void enviar_interrupcion_a_cpu(int pid, motivo_interrupcion motivo_interrupcion,char* nombre_interface, int conexion);
void liberar_proceso_memoria(uint32_t pid);
bool list_contains(t_list* lista_de_procesos, uint32_t pid);
void poner_en_cola_exit(t_pcb* proceso);
void enviar_proceso_a_cpu(t_pcb* pcb, int conexion);
void replanificar_y_ejecutar(t_pcb* proceso_ejecutando);
void planificar_y_ejecutar();
void largo_plazo_nuevo_ready();
void lanzar_interrupcion_fin_quantum (void* args);
void crear_listas_recursos();
void actualizar_quantum(t_pcb* proceso);
uint32_t buscar_indice_recurso(t_list* lista_recursos,char* nombre_recurso);
int encontrar_indice_proceso_data_por_pid(t_list * lista_procesos_data , int pid );
void mandar_proceso_a_finalizar(t_pcb* proceso_finalizar);
#endif /* PLANIFICADORES_H_ */
