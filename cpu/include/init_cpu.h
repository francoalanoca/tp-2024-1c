#ifndef TP_2024_1C_PASARONCOSAS_INIT_CPU_H
#define TP_2024_1C_PASARONCOSAS_INIT_CPU_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>

typedef struct 
{
   int pid;
   int program_counter;
   int quantum;
   RegistrosCPU registrosCPU;
}pcb;

typedef struct 
{
    unit32_t PC;
    unit8_t AX;
    unit8_t BX;
    unit8_t CX;
    unit8_t DX;
    unit32_t EAX;
    unit32_t EBX;
    unit32_t ECX;
    unit32_t EDX;
    unit32_t SI;
    unit32_t DI;


}RegistrosCPU;

pcb *pcb_actual;


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
    int PUERTO_ESCUCHA_INTERRUPT;
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