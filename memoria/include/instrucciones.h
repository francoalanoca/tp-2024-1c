#ifndef TP_2024_1C_PASARONCOSAS_INSTRUCCIONES_H
#define TP_2024_1C_PASARONCOSAS_INSTRUCCIONES_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

#include "../include/init_memoria.h"


//t_list *leer_instrucciones(char* nombre_archivo);
char *buscar_instruccion(int proceso_pid, int program_counter);

#endif