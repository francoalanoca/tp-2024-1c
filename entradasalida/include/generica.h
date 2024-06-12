#ifndef TP_2024_1C_PASARONCOSAS_GENERICA_H
#define TP_2024_1C_PASARONCOSAS_GENERICA_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>
#include "../include/init_entrada_salida.h"

void iniciar_interfaz_generica (int); 
 t_io_espera* deserializar_espera (t_list*  lista_paquete );
 void esperar(int32_t pid, int32_t unidades);
#endif //TP_2024_1C_PASARONCOSAS_GENERICA_H
