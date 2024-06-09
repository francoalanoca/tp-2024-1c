#ifndef INIT_MEMORIA_H_
#define INIT_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
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
    char* PUERTO_ESCUCHA;
    uint32_t TAM_MEMORIA;
    uint32_t TAM_PAGINA;
    char* PATH_INSTRUCCIONES;
    uint32_t RETARDO_RESPUESTA;
} t_config_memoria;

extern t_config_memoria *cfg_memoria;


static t_config_memoria *cfg_memoria_start();

static t_config_memoria *cfg_memoria_start(){

    t_config_memoria *cfg = malloc(sizeof(t_config_memoria));
    return cfg;
}



int init(char *path_config);
int checkProperties(char *path_config);
int cargar_configuracion(char *path_config);
void inicializar_memoria();
void cerrar_programa();


#endif