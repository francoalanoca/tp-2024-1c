#ifndef TP_2024_1C_PASARONCOSAS_MAIN_H
#define TP_2024_1C_PASARONCOSAS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>
#include "../include/cpu_utils.h"
#include "../include/servidorCpu.h"


typedef struct {
    int socket_memoria;
    t_pcb *proceso_actual;
    t_list *tlb;
    t_list* lista_conexion_kernel_dispatch;
    int conexion_kernel_interrupt;
} ciclo_params_t;


void ejecutar_ciclo() ;
void ciclo_de_instrucciones(int *conexion, t_pcb* proceso, t_list* tlb, int *socket_dispatch,int *socket_dispatch_interrupciones, int *socket_interrupt);





#endif //TP_2024_1C_PASARONCOSAS_MAIN_H