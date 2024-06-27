#ifndef SERVIDOR_KERNEL_H_
#define SERVIDOR_KERNEL_H_

#include "../include/init_kernel.h"
#include <utils/utils.h>
#include <pthread.h>

void* crearServidor();
int server_escuchar(t_log *logger, char *server_name, int server_socket);
void procesar_conexion(void *void_args) ;
void Empezar_conexiones();

#endif /* SERVIDOR_KERNEL_H_ */