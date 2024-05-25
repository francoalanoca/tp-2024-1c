#ifndef KERNEL_H_
#define KERNEL_H_

#define KERNEL "KERNEL"
//#define IP_ESCUCHA "127.0.0.1" 
//#define PUERTO_ESCUCHA 5000 // Puerto de escucha para conexiones entrantes

#include <utils/utils.h>
#include "planificadores.h"

typedef struct {
    int PUERTO_ESCUCHA;
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_CPU;
    int PUERTO_CPU_DISPATCH;
    int PUERTO_CPU_INTERRUPT;
    //char* log_file;
} t_config_kernel;

// Variables Globales
t_config* config;
t_config_kernel* config_kernel;
int conexion_consola;
int conexion_cpu;
int conexion_memoria;
int conexion_fs;
int socket_servidor;
int contador_pid;
t_planificador* planificador;

// Prototipos de funciones
void iniciar_logger();
void iniciar_config_kernel();
//void terminar_programa1(int conexion,t_log* logger,t_config* config);
void atender_cliente(void *arg);
void terminar_programa_kernel(int conexion,t_log* logger,t_config* config);

#endif /* KERNEL_H_ */