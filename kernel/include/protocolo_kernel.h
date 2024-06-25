#ifndef PROTOCOLO_KERNEL_H
#define PROTOCOLO_KERNEL_H

#include <utils/utils.h>
#include <pthread.h>
#include <commons/string.h>
#include </home/utnso/tp-2024-1c-Pasaron-cosas/kernel/include/init_kernel.h>


void Escuchar_Msj_De_Conexiones();
void Kernel_atender_cpu_dispatch();
void Kernel_atender_cpu_interrupt();
void Kernel_atender_memoria();
void enviar_creacion_de_proceso_a_memoria(t_m_crear_proceso* pcb, int socket_memoria);


#endif
