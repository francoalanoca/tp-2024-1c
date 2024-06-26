#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include <stdbool.h>
#include <utils/utils.h>

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
    t_dictionary* cola_blocked;  
    t_list* cola_exit;     
    t_algoritmo_planificacion algoritmo;
    int quantum;
    int grado_multiprogramacion;
    int grado_multiprogramacion_actual;
    bool planificacion_detenida;         
} t_planificador;



// ver si falta poner alguna libreria
extern t_planificador* planificador;

// Inicializa un nuevo planificador
t_planificador* inicializar_planificador(t_algoritmo_planificacion algoritmo, int quantum, int grado_multiprogramacion);
// Destruye el planificador y libera la memoria

void destruir_planificador(t_planificador* planificador);
bool agregar_proceso(t_planificador* planificador, t_pcb* proceso);
t_algoritmo_planificacion obtener_algoritmo_planificador(const char* algoritmo_planificacion);
t_pcb* obtener_proximo_proceso(t_planificador* planificador);
void desalojar_proceso(t_planificador* planificador, t_pcb* proceso);
void bloquear_proceso(t_planificador* planificador, t_pcb* proceso);
void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso);
void finalizar_proceso(t_planificador* planificador, t_pcb* proceso);
void crear_proceso(t_planificador* planificador, char* path_pseudocodigo);
void eliminar_proceso(t_planificador* planificador, t_pcb* proceso);
void detener_planificacion(t_planificador* planificador);

#endif /* PLANIFICADORES_H_ */
