#ifndef TP_2024_1C_PASARONCOSAS_PAGINAS_H
#define TP_2024_1C_PASARONCOSAS_PAGINAS_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

#include "../include/init_memoria.h"



t_tabla_de_paginas *crear_tabla_pagina(int pid, int tamanio_proceso);
int calcular_marcos(int tamanio_proceso);

#endif