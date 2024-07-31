#ifndef SERVIDOR_KERNEL_H_
#define SERVIDOR_KERNEL_H_

//#include "../include/init_kernel.h"
//#include "../include/planificador.h"
//#include <utils/utils.h>
#include "../include/protocolo_kernel.h"
#include <pthread.h>

void* crearServidor();
int server_escuchar(t_log *logger, char *server_name, int server_socket);
void procesar_conexion(void *void_args) ;
t_interfaz_pid* deserializar_interfaz_pid(t_list*  lista_paquete );
void desbloquear_y_agregar_a_ready(t_planificador* planificador,int pid,char* nombre_interfaz);
t_pcb * desbloquear_proceso_io(t_planificador* planificador,int pid,char* nombre_interfaz);
#endif /* SERVIDOR_KERNEL_H_ */