#ifndef TP_2024_1C_PASARONCOSAS_CONEXION_H
#define TP_2024_1C_PASARONCOSAS_CONEXION_H

#include <utils/utils.h>

#include "../include/init_memoria.h"

int iniciar_servidor_memoria(t_log*, char*);
void iniciar_conexiones();
//int hacer_handshake (int socket_cliente);

#endif