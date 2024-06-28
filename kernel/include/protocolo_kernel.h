#ifndef PROTOCOLO_KERNEL_H
#define PROTOCOLO_KERNEL_H
#include <utils/utils.h>
#include <pthread.h>
#include <commons/string.h>
#include </home/utnso/tp-2024-1c-Pasaron-cosas/kernel/include/init_kernel.h>
#include </home/utnso/tp-2024-1c-Pasaron-cosas/kernel/include/consola.h>
//#include <utils/utils.h>
//#include <pthread.h>
//#include <commons/string.h>
//#include <init_kernel.h>


//void Escuchar_Msj_De_Conexiones();
//void Kernel_atender_cpu_dispatch();
//void Kernel_atender_cpu_interrupt();
//void Kernel_atender_memoria();
void Escuchar_Msj_De_Conexiones();
void Kernel_escuchar_cpu_dispatch();
void Kernel_escuchar_cpu_interrupt();
void Kernel_escuchar_memoria();
void enviar_creacion_de_proceso_a_memoria(t_pcb* pcb, int socket_memoria);
//void enviar_creacion_de_proceso_a_memoria(pcb, socket_memoria);  // Enviar a memoria
t_pcb* encontrar_proceso_pid(t_list * lista_procesos , uint32_t pid);
uint32_t encontrar_indice_proceso_pid(t_list * lista_procesos , t_pcb* pcb);
t_recurso* deserializar_recurso(t_list*  lista_paquete );
uint32_t buscar_indice_recurso(char* nombre_recurso);
uint32_t buscar_indice_primer_valor_no_nulo(t_list* lista);



#endif
