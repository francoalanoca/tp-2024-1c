#ifndef TP_2024_1C_PASARONCOSAS_PAGINAS_H
#define TP_2024_1C_PASARONCOSAS_PAGINAS_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

#include "../include/init_memoria.h"







void crear_proceso(int proceso_pid);
t_tabla_de_paginas *crear_tabla_pagina(int pid);
int calcular_marcos(int tamanio_proceso);
t_tabla_de_paginas *busco_tabla_de_paginas_por_PID(int proceso_pid);
t_pagina *busco_pagina_por_marco(t_list *lista_de_paginas, int marco);
void escribir_memoria(int proceso_pid, int direccion_fisica, void* valor, int tamanio);
void* leer_memoria(int direccion_fisica, int tamanio);
int buscar_marco_pagina(int proceso_pid, int numero_de_pagina);
//void finalizar_proceso(int proceso_pid);


#endif