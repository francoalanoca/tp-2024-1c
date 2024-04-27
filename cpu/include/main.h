#ifndef TP_2024_1C_PASARONCOSAS_MAIN_H
#define TP_2024_1C_PASARONCOSAS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config);
void fetch(int conexion, t_log* logger, t_config* config);
void decode(int conexion, t_log* logger, t_config* config);
void execute(int conexion, t_log* logger, t_config* config);
void check_interrupt(int conexion, t_log* logger, t_config* config);

#endif //TP_2024_1C_PASARONCOSAS_MAIN_H