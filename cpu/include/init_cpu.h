#ifndef TP_2024_1C_PASARONCOSAS_INIT_CPU_H
#define TP_2024_1C_PASARONCOSAS_INIT_CPU_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>


extern t_proceso* proceso_actual;
extern t_proceso_interrumpido* proceso_interrumpido_actual;
extern bool interrupcion_kernel;

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
    int PUERTO_MEMORIA;
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