#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/config.h>
#include<string.h>
#include<assert.h>
#include<signal.h>
#include <errno.h>


#define PUERTO "4444"

typedef enum
{
 //----------------BASICOS--------------------------------
    HANDSHAKE = 1,
    HANDSHAKE_OK,
	MENSAJE,
	PAQUETE,
	PCB = 30,
	NUEVO_PROCESO = 35,
    PROXIMA_INSTRUCCION = 40,   // Cpu le solicita a Memoria la proxima instruccion a ejecutar
    INTERRUPCION_CPU = 45,
    ENVIO_INTERFAZ = 50,
 //---------------CPU-MEMORIA-------------------
    INSTRUCCION_RECIBIDA = 55,  // Memoria envia a Cpu la instruccion solicitada
 //---------------ENTRADASALIDA-KERNEL-------------------
    INTERFAZ_ENVIAR,            // EntradaSalida, avisa que envía la interfaz creada
    INTERFAZ_RECIBIDA,          // Es el ok del kernel al recibir la interfaz
    OPERACION_INVALIDA,         // EntradaSalida, avisa que envía la operacion es invalida
    IO_K_GEN_SLEEP,             // Kernel solicita realizar esta operación (usar esta para otros modulos tambien)
    IO_K_GEN_SLEEP_FIN,         // EntradaSalida, avisa que envía que finalizo la operacion IO_GEN_SLEEP
    IO_K_STDIN,
    IO_K_STDIN_FIN,
//----------------KERNEL-MEMORIA
    CREAR_PROCESO_KERNEL,       // Kerner le solicita a Memoria crear las estructuras necesarias
    FINALIZAR_PROCESO,          // Kernel le solicita a Memoria liberar el espacio en memoria del proceso
 //---------------ENTRADASALIDA-MEMORIA-------------------
    IO_M_STDIN,                 // entradasalida envia input a memoria
    IO_M_STDIN_FIN              // Memoria guardó con éxito el input
}op_code; 

typedef struct {
    t_log *log;
    int fd;
    char *server_name;
} t_procesar_conexion_args;

typedef enum
{
    SET,
	SUM,
	SUB,
    MOV_IN,
    MOV_OUT,
    RESIZE,
    JNZ,
    COPY_STRING, 
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE, 
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    WAIT,
    SIGNAL,
    EXIT

}tipo_instruccion;

typedef struct {
    uint8_t idLength;
    tipo_instruccion id; // el id seria el nombre de la instruccion
    uint8_t param1Length;
    char* param1;
    uint8_t param2Length;
    char* param2;
    uint8_t param3Length;
    char* param3;
    uint8_t param4Length;
    char* param4;
    uint8_t param5Length;
    char* param5;
} instr_t;

/*typedef struct
{
	int size;
	void* stream;
} t_buffer;*/

typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;



typedef struct 
{
    uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;


}t_registros_CPU;

typedef struct 
{
   uint32_t pid;
   uint32_t program_counter;
   uint32_t quantum;
   t_registros_CPU* registrosCPU;
}t_pcb;
typedef enum {
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS 
} t_tipo_interfaz_enum;

typedef struct {
    char* nombre;
    //uint8_t nombre_size; creo que no hace falta
    t_tipo_interfaz_enum tipo;//Debe ser un enum?
    //uint8_t tipo_size;  creo que no hace falta
}t_interfaz;


typedef struct {
    t_pcb* pcb; 
   // uint8_t cantidad_instrucciones;
   // t_list* instrucciones;
    t_list*  interfaces;
}t_proceso;

typedef struct{
    t_proceso* proceso;
    uint8_t tamanio_motivo_interrupcion;
    char* motivo_interrupcion;
}t_proceso_interrumpido;

typedef struct{
    uint32_t pid;
    uint32_t program_counter;
}t_proceso_memoria;


typedef struct {
	int32_t nro_pag;
	int32_t desplazamiento;
} t_direccion_logica;

typedef struct {
	int32_t nro_frame;
	int32_t desplazamiento;
} t_direccion_fisica;





void* recibir_buffer(int*, int);
int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
int esperar_cliente(t_log *logger, const char *name, int socket_servidor);
int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void agregar_a_buffer(t_buffer* un_buffer, void* valor, int tamanio);
void eliminar_buffer(t_buffer* un_buffer);
void cargar_int_al_buffer(t_buffer* un_buffer, int tamanio_int);
void cargar_uint32_al_buffer(t_buffer* un_buffer, uint32_t tamanio_uint32);
void cargar_string_al_buffer(t_buffer* un_buffer, char* tamanio_string);
char* extraer_string_del_buffer(t_buffer* un_buffer);
t_paquete* crear_paquete(op_code codigo_operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void iterator(char* value);
void terminar_programa(int conexion, t_log* logger, t_config* config);
t_config* iniciar_config(char* path_config, t_log*);
int recibir_informacion(int conexion, t_log* logger);
void crear_servidor(t_log* logger);
void handshake_cliente(t_config* config, t_log* logger, int conexion);
bool config_has_all_properties(t_config *cfg, char **properties);






#endif /* UTILS_H_ */

