#ifndef TP_2024_1C_PASARONCOSAS_PROTOCOLO_H
#define TP_2024_1C_PASARONCOSAS_PROTOCOLO_H

#include <utils/utils.h>
#include <pthread.h>
#include <commons/string.h>

#include "../include/init_memoria.h"
#include "../include/paginas.h"
#include "../include/instrucciones.h"




void memoria_atender_cliente(void* socket_cliente);
#endif