#include "planificador.h"
// ver si tengo que incluir la libreria donde esta el pcb
//#include "../include/servidorCpu.c"
//#include "../include/servidorCpu.h"


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

t_planificador* inicializar_planificador(t_algoritmo_planificacion algoritmo, int quantum) {
    t_planificador* planificador = malloc(sizeof(t_planificador));
    planificador->cola_new = list_create();
    planificador->cola_ready = list_create();
    planificador->cola_exec = list_create();
    planificador->cola_blocked = list_create();
    planificador->cola_exit = list_create();
    planificador->algoritmo = algoritmo;
    planificador->quantum = quantum;
    return planificador;
}

void destruir_planificador(t_planificador* planificador) {
    list_destroy_and_destroy_elements(planificador->cola_new, free);
    list_destroy_and_destroy_elements(planificador->cola_ready, free);
    list_destroy_and_destroy_elements(planificador->cola_exec, free);
    list_destroy_and_destroy_elements(planificador->cola_blocked, free);
    list_destroy_and_destroy_elements(planificador->cola_exit, free);
    free(planificador);
}



bool agregar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_add(planificador->cola_new, proceso);
    return true;
}

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

void desalojar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_ready, proceso);
}

void bloquear_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_blocked, proceso);
}

void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_blocked, proceso);
    list_add(planificador->cola_ready, proceso);
}

void finalizar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_exit, proceso);
    free(proceso);
}
