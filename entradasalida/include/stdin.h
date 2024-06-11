#ifndef TP_2024_1C_PASARONCOSAS_STDIN_H
#define TP_2024_1C_PASARONCOSAS_STDIN_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>
#include "../include/init_entrada_salida.h"

void iniciar_interfaz_stdin (int socket_kernel, int socket_memoria); 
op_code  enviar_input(t_io_input* io_input ,int socket_memoria);
#endif //TP_2024_1C_PASARONCOSAS_STDIN_H
