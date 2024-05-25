#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#define KERNEL "KERNEL"
//#define IP_ESCUCHA "127.0.0.1" 
//#define PUERTO_ESCUCHA 5000 // Puerto de escucha para conexiones entrantes

#include <utils/utils.h>
#include <planificador.h>

int checkProperties(char *path_config);

int cargar_configuracion(char *path);

int init(char *path_config);

int hacer_handshake (int socket_cliente);


void cerrar_programa();

// Variables Globales
extern t_log *logger_kernel;
extern t_config* file_cfg_kernel;
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

extern t_config_kernel* cfg_kernel; 

static t_config_kernel *cfg_kernel_start()
{
    t_config_kernel *cfg = malloc(sizeof(t_config_kernel));
    return cfg;
}


int conexion_consola;
int conexion_cpu_dispatch;
int conexion_memoria;
int conexion_fs;
int socket_servidor;
int contador_pid;
t_planificador* planificador;


#endif /* INIT_KERNEL_H_ */