#ifndef PROTOCOLO_KERNEL_H
#define PROTOCOLO_KERNEL_H
#include <utils/utils.h>
#include <pthread.h>
#include <commons/string.h>
#include "../include/init_kernel.h"
#include "../include/consola.h"
//#include "../include/planificador.h"

typedef struct {
    int* conexion_cpu_dispatch;
    int* conexion_cpu_interrupt;
} t_kernel_escuchar_cpu;


//void Escuchar_Msj_De_Conexiones();
//void Kernel_atender_cpu_dispatch();
//void Kernel_atender_cpu_interrupt();
//void Kernel_atender_memoria();
void Escuchar_Msj_De_Conexiones();
void Kernel_escuchar_cpu_dispatch(void* args);
void Kernel_escuchar_cpu_interrupt(int *conexion);
void Kernel_escuchar_memoria(int *conexion);
void enviar_creacion_de_proceso_a_memoria(t_pcb* pcb, int socket_memoria);
//void enviar_creacion_de_proceso_a_memoria(pcb, socket_memoria);  // Enviar a memoria
//t_pcb* encontrar_proceso_pid(t_list * lista_procesos , uint32_t pid);
t_recurso* deserializar_recurso(t_list*  lista_paquete );
//uint32_t buscar_indice_recurso(t_list* lista_recursos,char* nombre_recurso);
uint32_t buscar_indice_primer_valor_no_nulo(t_list* lista);
//void mandar_proceso_a_finalizar(uint32_t pid);
//void enviar_proceso_a_cpu(t_pcb* pcb, int conexion);
//void replanificar_y_ejecutar();



#endif
