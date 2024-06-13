#ifndef TP_2024_1C_PASARONCOSAS_PROTOCOLO_H
#define TP_2024_1C_PASARONCOSAS_PROTOCOLO_H

#include <utils/utils.h>
#include <pthread.h>
#include <commons/string.h>
#include "../include/init_memoria.h"




void escuchar_modulos();
void memoria_atender_kernel();
void memoria_atender_cpu();
void memoria_atender_io();

#endif