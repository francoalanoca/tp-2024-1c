#ifndef TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H
#define TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <utils/utils.h>
#include "../include/init_cpu.h"

bool generar_conexiones();
void *crear_servidor_dispatch();
int server_escuchar(t_log *logger, char *server_name, int server_socket);
void procesar_conexion(int cliente_socket);
int hacer_handshake (int socket_cliente);

//void *conectarConMemoria();
//void cortar_conexiones();
//void cerrar_servers();
//void procesoTerminado();

#endif //TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H