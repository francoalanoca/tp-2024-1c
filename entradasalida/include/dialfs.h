#ifndef TP_2024_1C_PASARONCOSAS_DIALFS_H
#define TP_2024_1C_PASARONCOSAS_DIALFS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/string.h>
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
//busca el archivo fcb.txt en el directiorio de fcb a partir del nombre de archivo
t_FCB* buscar_cargar_fcb(char* nombre);

//leer bloque contiguo
//escribir bloque contiguo
// lista de bloques vacios
// hay espacio necesario -- es decir los bloques contiguos suman el espacio necesario
//compactar

void iniciar_interfaz_dialfs (int socket_kernel, int socket_memoria); 
int crear_bitmap (char * path_archivo_bitmap);
int crear_archivo_bloques (char * path_archivo_bloques, int block_size, int block_count) ;
void cerrar_bitmap();
void cargar_directorio_fcbs(char* path_fcb );
//para verificar la terminación del archivo y determinar si es un fcb
bool termina_en_txt(const char *nombre) ;
//guarda la estructura fcb en un archivo fisico terminado en txt
void persistir_fcb(t_FCB *fcb); 
//devuelve la posicion de un bit libre en un bit array
uint32_t encontrar_bit_libre(t_bitarray* bitarray_in); 

//desearliza un paquete de creacion de archivo
t_io_crear_archivo* deserializar_fs_creacion (t_list* lista_paquete);
#endif //TP_2024_1C_PASARONCOSAS_DIALFS_H
