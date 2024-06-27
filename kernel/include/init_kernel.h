#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#define KERNEL "KERNEL"
//#define IP_ESCUCHA "127.0.0.1" 
//#define PUERTO_ESCUCHA 5000 // Puerto de escucha para conexiones entrantes

#include <utils/utils.h>
#include "../include/planificador.h"

int checkProperties(char *path_config);
int cargar_configuracion(char *path);
int init(char *path_config);
int hacer_handshake (int socket_cliente);
void cerrar_programa();

t_pcb* buscar_pcb_en_lista(t_list* lista_de_pcb, uint32_t pid);

bool interfaz_permite_operacion(t_tipo_interfaz_enum tipo_interfaz, tipo_instruccion instruccion);


// Variables Globales
extern t_log *logger_kernel;
extern t_config* file_cfg_kernel;
extern t_pcb* pcb;
extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int conexion_memoria;
extern int socket_servidor;
extern t_dictionary* interfaces;
typedef struct {
    char* PUERTO_ESCUCHA;
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_CPU;
    int PUERTO_CPU_DISPATCH;
    int PUERTO_CPU_INTERRUPT;
    char* ALGORITMO_PLANIFICACION;
    int QUANTUM;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;
    int GRADO_MULTIPROGRAMACION;
} t_config_kernel;

extern t_config_kernel* cfg_kernel; 

static t_config_kernel *cfg_kernel_start()
{
    t_config_kernel *cfg = malloc(sizeof(t_config_kernel));
    return cfg;
}

typedef struct{
    char* nombre;
    uint32_t tipo;
    uint32_t conexion;

}t_interfaz_diccionario;

#endif /* INIT_KERNEL_H_ */