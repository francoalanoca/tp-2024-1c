#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#define KERNEL "KERNEL"
//#define IP_ESCUCHA "127.0.0.1" 
//#define PUERTO_ESCUCHA 5000 // Puerto de escucha para conexiones entrantes

#include <utils/utils.h>
#include <planificador.h>



typedef struct {
    int PUERTO_ESCUCHA;
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_CPU;
    int PUERTO_CPU_DISPATCH;
    int PUERTO_CPU_INTERRUPT;
    char* ALGORITMO_PLANIFICACION;
    int QUANTUM;
    char** RECURSOS;
    char ** INSTANCIAS_RECURSOS;
    int GRADO_MULTIPROGRAMACION;
} t_config_kernel;

// Variables Globales
extern t_log *logger_kernel;
extern t_config* file_cfg_kernel;
extern t_config_kernel* cfg_kernel; 

int conexion_consola;
int conexion_cpu_dispatch;
int conexion_memoria;
int conexion_fs;
int socket_servidor;
int contador_pid;
t_planificador* planificador;

// Prototipos de funciones
void iniciar_logger();

//void terminar_programa1(int conexion,t_log* logger,t_config* config);
void cerrar_programa()
#endif /* INIT_KERNEL_H_ */