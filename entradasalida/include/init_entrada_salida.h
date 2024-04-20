#ifndef TP_2024_1C_PASARONCOSAS_INIT_ENTRADA_SALIDA_H
#define TP_2024_1C_PASARONCOSAS_INIT_ENTRADA_SALIDA_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>


int checkProperties(char *path_config);

int cargar_configuracion(char *path);

int init(char *path_config);

int hacer_handshake (int socket_cliente);


void cerrar_programa();

extern t_log *logger_entrada_salida;
extern t_config *file_cfg_entrada_salida;

typedef struct
{
    char *TIPO_INTERFAZ;
    int TIEMPO_UNIDAD_TRABAJO;
    char *IP_MEMORIA;
    char *PUERTO_MEMORIA;
    char *IP_KERNEL;
    char *PUERTO_KERNEL;
    char *PATH_BASE_DIALFS;
    int BLOCK_SIZE;
    int BLOCK_COUNT;
    int RETRASO_COMPACTACION;
} t_config_entrada_salida;

extern t_config_entrada_salida *cfg_entrada_salida;


static t_config_entrada_salida *cfg_entrada_salida_start()
{
    t_config_entrada_salida *cfg = malloc(sizeof(t_config_entrada_salida));
    return cfg;
}

#endif //TP_2024_1C_PASARONCOSAS_INIT_ENTRADA_SALIDA_H
