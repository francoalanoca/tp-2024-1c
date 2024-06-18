#include "planificador.h"
// ver si tengo que incluir la libreria donde esta el pcb
//#include "../include/servidorCpu.c"
//#include "../include/servidorCpu.h"

// Devuelve un t_algoritmo a partir de la config cargada
t_algoritmo_planificacion obtener_algoritmo_planificador(const char* algoritmo_planificacion) {
    if (strcmp(algoritmo_planificacion, "FIFO") == 0) {
        return FIFO;
    } else if (strcmp(algoritmo_planificacion, "ROUND ROBIN") == 0) {
        return ROUND_ROBIN;
    } else if (strcmp(algoritmo_planificacion, "VIRTUAL ROUND ROBIN") == 0) {
        return VIRTUAL_ROUND_ROBIN;
    } else {
        // Manejo de error para tipos de algoritmos desconocidos
        return -1;
    }
}

//Inicializa un nuevo planificador
t_planificador* inicializar_planificador(t_algoritmo_planificacion algoritmo, int quantum) {
    t_planificador* planificador = malloc(sizeof(t_planificador));
    planificador->cola_new = list_create();
    planificador->cola_ready = list_create();
    planificador->cola_exec = list_create();
    planificador->cola_blocked = list_create();
    planificador->cola_exit = list_create();
    planificador->algoritmo = algoritmo;
    planificador->quantum = quantum;
    planificador->grado_multiprogramacion = grado_multiprogramacion;
    planificador->grado_multiprogramacion_actual = 0;
    return planificador;
}

// Destruye el planificador y libera la memoria
void destruir_planificador(t_planificador* planificador) {
    list_destroy_and_destroy_elements(planificador->cola_new, free);
    list_destroy_and_destroy_elements(planificador->cola_ready, free);
    list_destroy_and_destroy_elements(planificador->cola_exec, free);
    list_destroy_and_destroy_elements(planificador->cola_blocked, free);
    list_destroy_and_destroy_elements(planificador->cola_exit, free);
    free(planificador);
}

// Agrega un nuevo proceso al planificador
bool agregar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_add(planificador->cola_new, proceso);
    if (planificador->grado_multiprogramacion_actual < planificador->grado_multiprogramacion) {
        t_pcb* proceso_nuevo = list_remove(planificador->cola_new, 0);
        list_add(planificador->cola_ready, proceso_nuevo);
        planificador->grado_multiprogramacion_actual++;
    }
    return true;
}

// Obtiene el próximo proceso a ejecutar
t_pcb* obtener_proximo_proceso(t_planificador* planificador) {
    t_pcb* proceso;
    if (planificador->algoritmo == FIFO) {
        proceso = list_remove(planificador->cola_ready, 0);
    } else if (planificador->algoritmo == ROUND_ROBIN) {
        proceso = list_get(planificador->cola_ready, 0);
        list_remove(planificador->cola_ready, 0);
        list_add(planificador->cola_ready, proceso);
    } else { // Virtual Round Robin
        proceso = list_remove(planificador->cola_ready, 0);
        if (proceso->tiempo_ejecucion >= planificador->quantum) {
            proceso->tiempo_ejecucion -= planificador->quantum;
            list_add(planificador->cola_ready, proceso);
        }
    }
    list_add(planificador->cola_exec, proceso);
    return proceso;
}

// Desaloja un proceso de la cola de ejecución y lo pone en la cola de listos
void desalojar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_ready, proceso);
}

//Bloquea un proceso y lo mueve a la cola de bloqueados
void bloquear_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_blocked, proceso);
}

//  Desbloquea un proceso y lo mueve a la cola de listos
void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_blocked, proceso);
    list_add(planificador->cola_ready, proceso);
}

// Finaliza un proceso y libera su memoria
void finalizar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_exit, proceso);
    free(proceso);
     planificador->grado_multiprogramacion_actual--;
    if (!list_is_empty(planificador->cola_new)) {
        t_pcb* proceso_nuevo = list_remove(planificador->cola_new, 0);
        list_add(planificador->cola_ready, proceso_nuevo);
        planificador->grado_multiprogramacion_actual++;
    }
}

// Crea un nuevo proceso
void crear_proceso(t_planificador* planificador, char* path_pseudocodigo) {
    t_pcb* nuevo_proceso = crear_pcb(path_pseudocodigo);
    agregar_proceso(planificador, nuevo_proceso);
    // Notificar a la memoria para crear el proceso
}

// elimina un proceso
void eliminar_proceso(t_planificador* planificador, t_pcb* proceso) {
    if (list_contains(planificador->cola_exec, proceso)) {
        // Enviar señal de interrupción a la CPU
        // Esperar a que la CPU retorne el Contexto de Ejecución
    }
    // Notificar a la memoria para liberar las estructuras del proceso
    finalizar_proceso(planificador, proceso);
}
