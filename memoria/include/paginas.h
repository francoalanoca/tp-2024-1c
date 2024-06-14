#ifndef TP_2024_1C_PASARONCOSAS_PAGINAS_H
#define TP_2024_1C_PASARONCOSAS_PAGINAS_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

#include "../include/init_memoria.h"


void crear_proceso(int proceso_pid, int tamanio_proceso);
t_tabla_de_paginas *crear_tabla_pagina(int pid, int tamanio_proceso);
int calcular_marcos(int tamanio_proceso);
//void escribir_memoria(int proceso_pid, int direccion_fisica, void* valor, int tamanio);
//void* leer_memoria(int direccion_fisica, int tamanio);



#endif