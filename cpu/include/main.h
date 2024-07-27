#ifndef TP_2024_1C_PASARONCOSAS_MAIN_H
#define TP_2024_1C_PASARONCOSAS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>
#include "../include/cpu_utils.h"
#include "../include/servidorCpu.h"



void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_pcb* proceso, t_list* tlb);





#endif //TP_2024_1C_PASARONCOSAS_MAIN_H