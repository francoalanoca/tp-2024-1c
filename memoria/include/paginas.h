#ifndef TP_2024_1C_PASARONCOSAS_PAGINAS_H
#define TP_2024_1C_PASARONCOSAS_PAGINAS_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

#include "../include/init_memoria.h"


void crear_proceso(uint32_t proceso_pid);
t_tabla_de_paginas *crear_tabla_pagina(uint32_t pid);
uint32_t calcular_marcos(uint32_t tamanio_proceso);
t_tabla_de_paginas *busco_tabla_de_paginas_por_PID(uint32_t proceso_pid);
t_pagina *busco_pagina_por_marco(t_list *lista_de_paginas, uint32_t marco);
void* escribir_memoria(uint32_t proceso_pid, uint32_t direccion_fisica, char* valor, uint32_t tamanio);
void* leer_memoria(uint32_t proceso_pid, uint32_t direccion_fisica, uint32_t tamanio);
void* copiar_solicitud(uint32_t proceso_pid, uint32_t direccion_fisica, char* valor);
uint32_t buscar_marco_pagina(uint32_t proceso_pid, uint32_t numero_de_pagina);
void finalizar_proceso(uint32_t proceso_pid);
op_code administrar_resize(uint32_t proceso_pid, uint32_t tamanio_proceso);
// Devuelve la totalidad bytes disponibles en la memoria
int espacio_disponible();
// devuelve el primer marco libre que encuentra desde la posicion 0 del bitmap
uint32_t obtener_marco_libre();
#endif