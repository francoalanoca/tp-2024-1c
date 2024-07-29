#ifndef TP_2024_1C_PASARONCOSAS_INIT_CPU_H
#define TP_2024_1C_PASARONCOSAS_INIT_CPU_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/string.h>
#include <utils/utils.h>
#include <semaphore.h>
#include <pthread.h>


extern t_pcb* proceso_actual;
extern t_proceso_interrumpido* proceso_interrumpido_actual;
extern bool interrupcion_kernel;
extern int conexion_kernel_dispatch;
extern int conexion_kernel_interrupt;
extern instr_t *prox_inst;
extern sem_t sem_valor_instruccion;
extern t_list* tlb;
extern uint32_t marco_recibido;
extern sem_t sem_marco_recibido;
extern uint32_t tamanio_pagina;
extern int socket_memoria;
extern char* valor_registro_obtenido;
extern sem_t sem_valor_registro_recibido;
extern int rta_resize;
extern sem_t sem_valor_resize_recibido;
extern sem_t sem_valor_tamanio_pagina;
extern sem_t sem_servidor_creado;
extern sem_t sem_interrupcion_kernel;
extern sem_t sem_check_interrupcion_kernel;
extern sem_t sem_conexion_interrupt_iniciado;
extern sem_t sem_conexion_dispatch_iniciado;
extern pthread_mutex_t mutex_proceso_actual;
extern pthread_mutex_t mutex_proceso_interrumpido_actual;
extern pthread_mutex_t mutex_interrupcion_kernel;
typedef struct {
    uint32_t pid;
    uint32_t nro_pagina;
    uint32_t nro_marco;
}t_registro_tlb;

int checkProperties(char *path_config);

int cargar_configuracion(char *path);

int init(char *path_config);

int hacer_handshake (int socket_cliente);


void cerrar_programa();

extern t_log *logger_cpu;
extern t_config *file_cfg_cpu;

typedef struct
{
    char *IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* PUERTO_ESCUCHA_DISPATCH;
    char* PUERTO_ESCUCHA_INTERRUPT;
    int CANTIDAD_ENTRADAS_TLB;
    char *ALGORITMO_TLB;
} t_config_cpu;

extern t_config_cpu *cfg_cpu;


static t_config_cpu *cfg_cpu_start()
{
    t_config_cpu *cfg = malloc(sizeof(t_config_cpu));
    return cfg;
}


#endif //TP_2024_1C_PASARONCOSAS_INIT_CPU_H