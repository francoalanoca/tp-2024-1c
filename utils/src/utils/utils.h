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
#include<pthread.h>


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
    PEDIDO_MARCO_A_MEMORIA = 60,
    MARCO_RECIBIDO = 65,
    PETICION_VALOR_MEMORIA = 70, //CPU pide a memoria que le de el valor asociado a una direccion fisica
    PETICION_VALOR_MEMORIA_RTA = 75, //Memoria envia a CPU el valor asociado a la direccion fisica 
    GUARDAR_EN_DIRECCION_FISICA = 80, //CPU le manda a memoria dir fisica y valor y memoria debe guardar dicho valor en la dir fisica indicada
    SOLICITUD_RESIZE = 85, // CPU pide a memora que haga un resize del proceso
    SOLICITUD_RESIZE_RTA = 90, // Memoria responde el resultado de la operacion de resize
    ENVIAR_ERROR_MEMORIA_A_KERNEL = 95, //CPU le manda a kernel el proceso loego de que memoria tire error de out of memory
    ENVIO_COPY_STRING_A_MEMORIA = 100, //CPU solicita a memoria que guarde el valor en la direccion pasada por parametro
    ENVIO_WAIT_A_KERNEL =105, //CPU solicita a kernel que se asigne una instancia del recurso al proceso
    ENVIO_SIGNAL_A_KERNEL =110, //CPU solicita a kernel que se libere una instancia del recurso al proceso
    SOLICITUD_IO_STDIN_READ = 115, // CPU solicita a kernel hacer la operacion IO_STDIN_READ a partir de la interfaz, direccion y tamanio pasado
    SOLICITUD_IO_STDOUT_WRITE = 120, // CPU solicita a kernel hacer la operacion IO_STDOUT_WRITE a partir de la interfaz, direccion y tamanio pasado
    SOLICITUD_EXIT_KERNEL = 125, //CPU solicita a kernel la finalización del proceso
    SOLICITUD_TAMANIO_PAGINA =130,//CPU solicita a memoria el tamanio de pagina
    SOLICITUD_TAMANIO_PAGINA_RTA =135,//Memoria envia a CPU el tamanio de pagina

 //---------------ENTRADASALIDA-KERNEL-------------------
    INTERFAZ_ENVIAR,            // EntradaSalida, avisa que envía la interfaz creada
    INTERFAZ_RECIBIDA,          // Es el ok del kernel al recibir la interfaz
    OPERACION_INVALIDA,         // EntradaSalida, avisa que envía la operacion es invalida
    IO_K_GEN_SLEEP,             // Kernel solicita realizar esta operación (usar esta para otros modulos tambien)
    IO_K_GEN_SLEEP_FIN,         // EntradaSalida, avisa que envía que finalizo la operacion IO_GEN_SLEEP
    IO_K_STDIN,
    IO_K_STDIN_FIN,
    IO_K_STDOUT,
    IO_K_STDOUT_FIN,
//----------------KERNEL-MEMORIA
    CREAR_PROCESO_KERNEL,       // Kerner le solicita a Memoria crear las estructuras necesarias
    CREAR_PROCESO_KERNEL_FIN,
    FINALIZAR_PROCESO,          // Kernel le solicita a Memoria liberar el espacio en memoria del proceso
 //---------------ENTRADASALIDA-MEMORIA-------------------
    IO_M_STDIN,                 // entradasalida envia input a memoria
    IO_M_STDIN_FIN,              // Memoria guardó con éxito el input
    IO_M_STDOUT,
    IO_M_STDOUT_FIN,
    IO_FS_WRITE_M, // Memoria devuelve el valor a escribir en el archivo con un  t_io_output
    IO_FS_READ_M // Es el ok de que se guardo bien el valor en memoria, pero no sé si hace falta
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
    char* path;
    t_list* lista_recursos_pcb;
    pthread_mutex_t mutex_lista_recursos;
    t_registros_CPU registros_cpu;
    int estado;
}t_pcb;

typedef enum {
    ESTADO_NEW,
    ESTADO_READY,
    ESTADO_RUNNING,
    ESTADO_BLOCKED,
    ESTADO_EXIT
} estado_pcb;
typedef enum {
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS 
} t_tipo_interfaz_enum;

typedef struct {
    uint32_t nombre_length; 
    char* nombre;   
    t_tipo_interfaz_enum tipo;//es un enum por lo que pesa 4 bytes : uint32_t
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




typedef struct{
    uint32_t pid;
    uint32_t nro_pagina;
}t_busqueda_marco;

typedef struct{
    char* interfaz;
    uint32_t direccion;
    uint32_t tamanio;
}t_direccion_tamanio;



typedef struct {
	uint32_t pid;
	uint32_t tiempo_espera;
} t_io_espera;

//Kernel le manda a IO en operaciones STDIN, STDOUT y tambien IO le manda a memoria para que le pase el contenido a traves de un t_io_output
typedef struct {
	uint32_t pid;
    t_list*  direcciones_fisicas; 
} t_io_direcciones_fisicas;




//IO Le manda a memoria
typedef struct {
	uint32_t pid;
    t_list*  direcciones_fisicas;
    uint32_t input_length; 
    char* input;   
} t_io_input;

//Memoria le manda a IO
typedef struct {
	uint32_t pid;
    uint32_t output_length; 
    char* output;   
} t_io_output;

//Kernel le manda a io para crear/borrar/ truncar archivo
typedef struct {
	uint32_t pid;
    uint32_t nombre_archivo_length; 
    char* nombre_archivo; 
    uint32_t tamanio_archivo;  
} t_io_gestion_archivo;

//Kernel le manda a io para leer o escribir archivo
typedef struct {
	uint32_t pid;
    uint32_t nombre_archivo_length; 
    char* nombre_archivo; 
    t_list*  direcciones_fisicas;
    uint32_t tamanio_operacion;  
    uint32_t puntero_archivo;
} t_io_readwrite_archivo;

//Kernel-Memoria (struct para cop crear proceso)
typedef struct{
    uint32_t pid;                     //pcb del proceso
    uint32_t tamanio;               //tamaño del proceso
    char *archivo_pseudocodigo;     //nombre del proceso
} t_m_crear_proceso;


typedef struct {
    uint32_t pid;
    uint32_t tamanio;
    char* valor;
} t_resize;


typedef struct {
    uint32_t pid;
    uint32_t direccion_fisica;
    char* valor;
} t_copy;

typedef struct{
    uint32_t pid;
    uint32_t direccion_fisica;
    uint32_t tamanio;
    char* valor;
} t_escribir_leer;







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
void imprimir_stream(void* stream, int size);
t_tipo_interfaz_enum obtener_tipo_interfaz_enum (const char* tipo_interfaz_str);
void enviar_espera(t_io_espera* io_espera, int socket);
t_interfaz* deserializar_interfaz(t_list*  lista_paquete );
void enviar_io_df(t_io_direcciones_fisicas* io_df, int socket, op_code codigo_operacion);
t_io_direcciones_fisicas* deserializar_io_df(t_list*  lista_paquete );
void enviar_output(t_io_output* io_output ,int socket_io, uint32_t op_code);
t_io_output* deserializar_output(t_list*  lista_paquete );
t_m_crear_proceso* deserializar_crear_proceso(t_list*  lista_paquete );
void enviar_respuesta_crear_proceso(t_m_crear_proceso* crear_proceso ,int socket_kernel);
t_proceso_memoria* deserializar_proxima_instruccion(t_list*  lista_paquete );
t_busqueda_marco* deserializar_solicitud_marco(t_list*  lista_paquete );
t_io_direcciones_fisicas* deserializar_peticion_valor(t_list*  lista_paquete );
t_escribir_leer* deserializar_peticion_guardar(t_list*  lista_paquete);
t_resize* deserializar_solicitud_resize(t_list*  lista_paquete);
t_copy* deserializar_solicitud_copy(t_list*  lista_paquete);
void enviar_respuesta_instruccion(char* proxima_instruccion ,int socket_cpu);
void enviar_solicitud_marco(int marco ,int socket_cpu);
void enviar_solicitud_tamanio(uint32_t tamanio_pagina ,int socket_cpu);
void enviar_peticion_valor(void* valor ,int socket_cpu);
t_io_input* deserializar_input(t_list*  lista_paquete );
// Kernel envía a IO Crear/Borrar/Truncar Archivo
void  enviar_gestionar_archivo(t_io_gestion_archivo* nuevo_archivo, int socket, uint32_t cod_op);
//Lo pueden usar IOy MEMOMORIA, para enviarse direcciones físicas y los datos contenidos o a guardar
void enviar_input(t_io_input* io_input ,int socket, uint32_t op_code );
// Kernel a IO para leer o escribir archivo
void enviar_io_readwrite(t_io_readwrite_archivo* io_readwrite ,int socket, uint32_t op_code );
//IO recibe una peticion de escrir o leer archivo
t_io_readwrite_archivo* deserializar_io_readwrite(t_list*  lista_paquete );
// Devuelve un out a partit de un pid y un valor char*
t_io_output* armar_io_output(uint32_t pid, char* output);

#endif /* UTILS_H_ */

