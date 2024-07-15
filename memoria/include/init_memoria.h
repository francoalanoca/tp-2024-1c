#ifndef INIT_MEMORIA_H_
#define INIT_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <utils/utils.h>

#include <commons/bitarray.h>

extern t_log *logger_memoria;
extern t_config *file_cfg_memoria;

extern int fd_memoria;
extern int fd_cpu;
extern int fd_kernel;
extern int fd_entradasalida;


typedef struct
{
    uint32_t PUERTO_ESCUCHA;
    uint32_t TAM_MEMORIA;
    uint32_t TAM_PAGINA;
    char* PATH_INSTRUCCIONES;
    uint32_t RETARDO_RESPUESTA;
} t_config_memoria;

extern t_config_memoria *cfg_memoria;





typedef struct{
    uint32_t id;
    t_list *lista_de_paginas;
}t_tabla_de_paginas;


typedef struct{
    uint32_t marco;
    uint32_t posicion;
}t_pagina;


typedef struct{
    uint32_t pid;
    t_list *lista_de_instrucciones;
} t_miniPCB;


extern void* memoria;
extern t_list* lista_tablas_de_paginas;     
extern t_list* lista_miniPCBs;
//extern pthread_mutex_t mutex_memoria;
extern uint32_t cantidad_frames_memoria;
extern uint32_t cantidad_page_fault;  
extern t_bitarray *bitmap_frames;
extern t_dictionary* instrucciones_de_procesos;






static t_config_memoria *cfg_memoria_start();

static t_config_memoria *cfg_memoria_start(){

    t_config_memoria *cfg = malloc(sizeof(t_config_memoria));
    return cfg;
}



int init(char *path_config);
int checkProperties(char *path_config);
int cargar_configuracion(char *path_config);
void inicializar_memoria();
int redondear_a_multiplo_mas_cercano_de(int base, int valor);
t_bitarray *crear_bitmap(int entradas);
void cerrar_programa();


#endif