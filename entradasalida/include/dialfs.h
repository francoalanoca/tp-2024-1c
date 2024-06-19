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
//crea un archivo a partir de su nombre: Crea un fcb, lo persiste y lo agrega al diccionario de fbc.
uint32_t crear_archivo(char* nombre);
//Borra un archivo a partir de su nombre:Libera bloques en el bitmap, elimina el fcb fisico, y lo saca del diccionario de fcb.
uint32_t borrar_archivo(char* nombre);
//Agranda o achica un archivo a partir del tamaño dado, actualizando todas las estructuras
uint32_t truncar_archivo(char* nombre, uint32_t tamanio);
int leer_archivo(char* nombre, t_list* direcciones_memoria, int tamanio_lectura);
int escribir_archivo(char* nombre, t_list* direcciones_memoria, int tamanio_escritura);
//Limpia los bits asignados en el bitmap hasta ajustar el tamaño solicitado
void achicar_archivo(uint32_t tamanio,t_FCB* fcb);
//verifica que haya espacio y mueve el archivo a la nueva posicion determinada por la compactacion
void agrandar_archivo(uint32_t tamanio,t_FCB* fcb);

//Devuelve la posicion del primer bit libre que encuentra.
uint32_t encontrar_bit_libre(t_bitarray* bitarray);
//Busca el archivo fcb.txt en el directiorio de fcb a partir del nombre de archivo
t_FCB* buscar_cargar_fcb(char* nombre);
//Mueve el archivo a la posicion en bloques indicada, copia bloque por bloque, actualiza bitrarray, bitmap,fcb fisico, y dicccionario fcb
void mover_archivo(t_FCB* fcb_archivo, int nueva_posicion_inicial);

// devuelve la posicion desde la cual hay espacio disponible si no, está contiguo compacta, si no hay espacio devuelve -1
int hay_espacio_disponible(int espacio_necesario); 
// devuelve si hay espacio disponible no importa si está contiguo
bool hay_espacio_total_disponible(int espacio_necesario);
//devulve la posicion desde la cual el espacio esta disponible y contiguo
int hay_espacio_contiguo_disponible(int espacio_necesario);
//Desplaza los bloques en el fs hasta encontrar el primer hueco que tenga el espacio necesario y devuelve la posición.
int compactar(int espacio_necesario);
//Mueve bloques en direccion a la posicion 0 del bitmap: actualiza bitarray, bitmap, y archivo de bloques
void  mover_archivo_izquierda(t_FCB* fcb_archivo, int posiciones);

//inicia la interfaz 
void iniciar_interfaz_dialfs (int socket_kernel, int socket_memoria); 
int crear_bitmap (char * path_archivo_bitmap);
// sincroniza el bitarray en memoria y con el archivo fisico.
void sincronizar_bitmap ();
//crea el archivo de bloques fisico en la ruta especificada.
int crear_archivo_bloques (char * path_archivo_bloques, int block_size, int block_count) ;
void cerrar_bitmap();
// carga en un diccionario todos los archivos fcb persistidos en una ruta de carpeta
void cargar_directorio_fcbs(char* path_fcb );
//para verificar la terminación del archivo y determinar si es un fcb
bool termina_en_txt(const char *nombre) ;
//guarda la estructura fcb en un archivo fisico terminado en txt
void persistir_fcb(t_FCB *fcb); 
//devuelve la posicion de un bit libre en un bit array
uint32_t encontrar_bit_libre(t_bitarray* bitarray_in); 

//desearliza un paquete de creacion/eliminacion de archivo
t_io_crear_archivo* deserializar_fs_gestion (t_list* lista_paquete);

#endif //TP_2024_1C_PASARONCOSAS_DIALFS_H
