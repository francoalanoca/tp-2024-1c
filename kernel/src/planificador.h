#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include <stdbool.h>
#include <utils/utils.h>
// ver si falta poner alguna libreria

// Enumeración para los algoritmos de planificación
typedef enum {
    FIFO,
    ROUND_ROBIN,
    VIRTUAL_ROUND_ROBIN
} t_algoritmo_planificacion;

typedef struct {
    t_list* cola_new;      
    t_list* cola_ready;    
    t_list* cola_exec;     
    t_list* cola_blocked;  
    t_list* cola_exit;     
    t_algoritmo_planificacion algoritmo; 
    int quantum;           
} t_planificador;

// Inicializa un nuevo planificador
t_planificador* inicializar_planificador(t_algoritmo_planificacion algoritmo, int quantum);

// Destruye el planificador y libera la memoria
void destruir_planificador(t_planificador* planificador);

// Agrega un nuevo proceso al planificador
bool agregar_proceso(t_planificador* planificador, t_pcb* proceso);

// Devuelve un t_algoritmo a partir de la config cargada
t_algoritmo_planificacion obtener_algoritmo_planificador(const char* algoritmo_planificacion);

// Obtiene el próximo proceso a ejecutar 
t_pcb* obtener_proximo_proceso(t_planificador* planificador);

// Desaloja un proceso de la cola de ejecución y lo pone en la cola de listos
void desalojar_proceso(t_planificador* planificador, t_pcb* proceso);

// Bloquea un proceso y lo mueve a la cola de bloqueados
void bloquear_proceso(t_planificador* planificador, t_pcb* proceso);

// Desbloquea un proceso y lo mueve a la cola de listos
void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso);

// Finaliza un proceso y libera su memoria
void finalizar_proceso(t_planificador* planificador, t_pcb* proceso);

#endif /* PLANIFICADORES_H_ */