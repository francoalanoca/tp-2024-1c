#ifndef INIT_MEMORIA_H_
#define INIT_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>


extern t_log *logger_memoria;
extern t_config *file_cfg_memoria;

extern int fd_memoria;
extern int fd_cpu;
extern int fd_kernel;
extern int fd_entradasalida;


typedef struct
{
    char* PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_PAGINA;
    char* PATH_INSTRUCCIONES;
    int RETARDO_RESPUESTA;
} t_config_memoria;

extern t_config_memoria *cfg_memoria;




static t_config_memoria *cfg_memoria_start(){

    t_config_memoria *cfg = malloc(sizeof(t_config_memoria));
    return cfg;
}



int init(char *path_config);
int checkProperties(char *path_config);
int cargar_configuracion(char *path);
void cerrar_programa();


#endif