#ifndef TP_2024_1C_PASARONCOSAS_DIALFS_H
#define TP_2024_1C_PASARONCOSAS_DIALFS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/init_entrada_salida.h"


/////////////////////////////////////////////////INICIAR FS////////////////////////////////////
///////////////////////////////////////////////PETICIONES//////////////////////////////////////
//////////////////////////////////////////////ESTRUCTURAS//////////////////////////////////////

extern t_config *file_superbloque;

typedef struct
{
    int BLOCK_SIZE;
    int BLOCK_COUNT;
} t_cfg_superbloque;

extern t_cfg_superbloque *cfg_superbloque;

static t_cfg_superbloque *cfg_superbloque_start()
{
    t_cfg_superbloque *cfg = malloc(sizeof(t_cfg_superbloque));
    return cfg;
}


//BITMAP
extern FILE* archivo_bitmap;
extern void* bitmap;
extern t_bitarray* bitarray;  // estructura para el manejo del bitmap
extern  bitmap_size_in_bytes;
extern block_count;
extern fd_bitmap;

// ARCHIVO DE BLOQUES
extern FILE *archivo_bloques; //
extern int fd_archivo_bloques;


// FCB ARCHIVO DE METADATA
extern t_config *file_fcb;

typedef struct
{
    char *nombre_archivo;
    uint32_t  tamanio_archivo;
    uint32_t  primer_bloque; // bloque inicio
} t_FCB;

extern t_FCB *fcb;

t_FCB* inicializar_fcb(char* nombre_archivo, uint32_t tamanio_archivo, uint32_t primer_bloque);
t_FCB* cargar_fcb(t_config *file_fcb);

 

char* uint32_to_string (uint32_t number);

//diccionario para guardar los fcb tiempo de ejecución
extern t_dictionary* fcb_dict;

///////////////////////////////////////////////FUNCIONALIDADES//////////////////////////////////////
uint32_t  abrir_archivo(char* nombre);
uint32_t  crear_archivo(char* nombre);
uint32_t  truncar_archivo(char* nombre, uint32_t tamanio);
int leer_archivo(char* nombre, t_list* direcciones_memoria, int tamanio_lectura);
int escribir_archivo(char* nombre, t_list* direcciones_memoria, int tamanio_escritura);
void achicar_archivo(uint32_t tamanio,t_FCB* fcb);
void agrandar_archivo(uint32_t tamanio,t_FCB* fcb);
uint32_t encontrar_bit_libre(t_bitarray* bitarray);
t_FCB* buscar_cargar_fcb(char* nombre);

//leer bloque contiguo
//escribir bloque contiguo
// lista de bloques vacios
// hay espacio necesario -- es decir los bloques contiguos suman el espacio necesario
//compactar

void iniciar_interfaz_dialfs (int socket_kernel, int socket_memoria); 


#endif //TP_2024_1C_PASARONCOSAS_DIALFS_H
