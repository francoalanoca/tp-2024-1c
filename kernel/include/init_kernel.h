#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#define KERNEL "KERNEL"
//#define IP_ESCUCHA "127.0.0.1" 
//#define PUERTO_ESCUCHA 5000 // Puerto de escucha para conexiones entrantes

#include <utils/utils.h>

#include <semaphore.h>
#include <commons/temporal.h>

int checkProperties(char *path_config);
int cargar_configuracion(char *path);
int init(char *path_config);
int hacer_handshake (int socket_cliente);
void cerrar_programa();

t_pcb* buscar_pcb_en_lista(t_list* lista_de_pcb, uint32_t pid);
t_pcb* buscar_pcb_en_lista_de_data(t_list* lista_de_data, uint32_t pid);

bool interfaz_permite_operacion(t_tipo_interfaz_enum tipo_interfaz, tipo_instruccion instruccion);

// Variables Globales
extern t_log *logger_kernel;
extern t_config* file_cfg_kernel;
extern t_pcb* pcb;
extern int conexion_cpu_dispatch;
extern int conexion_cpu_dispatch_interrupciones;
extern int conexion_cpu_interrupt;
extern int conexion_memoria;
extern int socket_servidor;
extern t_dictionary* interfaces;
extern sem_t sem_contexto_ejecucion_recibido;
extern sem_t sem_confirmacion_memoria;
extern sem_t sem_interrupcion_atendida;
extern sem_t sem_io_fs_libre;
extern sem_t sem_cpu_libre;
extern sem_t sem_prioridad_io;
extern sem_t sem_rta_crear_proceso;
extern pthread_mutex_t mutex_cola_ready_prioridad; 
extern pthread_mutex_t mutex_cola_ready;
extern pthread_mutex_t mutex_envio_io;
extern pthread_mutex_t mutex_cola_exec;
extern pthread_mutex_t mutex_cola_blocked;
extern pthread_mutex_t mutex_cola_exit;
extern t_pcb* pcb_actualizado_interrupcion;
extern t_dictionary* procesos_recursos;
extern t_list* recursos;
extern uint32_t contador_pid;
extern sem_t sem_crearServidor;
extern sem_t sem_EscucharMsj;
typedef struct {
    char* PUERTO_ESCUCHA;
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* IP_CPU;
    char* PUERTO_CPU_DISPATCH;
    char* PUERTO_CPU_INTERRUPT;
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

// Enumeración para los algoritmos de planificación
typedef enum {
    FIFO,
    ROUND_ROBIN,
    VIRTUAL_ROUND_ROBIN
} t_algoritmo_planificacion;

typedef struct {
    t_list* cola_new;      
    t_list* cola_ready;   
    t_list* cola_ready_prioridad; 
    t_list* cola_exec;     
    t_dictionary* cola_blocked;  
    t_list* cola_exit;     
    t_algoritmo_planificacion algoritmo;
    int quantum;
    int grado_multiprogramacion;
    int grado_multiprogramacion_actual;
    bool planificacion_detenida;         
} t_planificador;

// ver si falta poner alguna libreria
extern t_planificador* planificador;

typedef struct{
    char* nombre;
    uint32_t tipo;
    uint32_t conexion;

}t_interfaz_diccionario;

typedef struct{
    t_list* nombres_recursos;
    t_list* instancias_recursos;

}t_proceso_recurso_diccionario;

typedef struct{
    t_pcb* pcb;
    void* data;
    op_code op;
}t_proceso_data;

typedef struct{
    uint32_t pid;
    uint32_t nombre_length;
    char* nombre_interfaz;
}t_interfaz_pid;

void liberar_cfg_kernel(t_config_kernel *cfg);

void destruir_diccionario(t_dictionary *dict);

void liberar_memoria_paquete(t_paquete* paquete);

void liberar_memoria_pcb(t_pcb* pcb);

void destruir_diccionario(t_dictionary *dict);

void destruir_elemento(void *elemento);

void liberar_proceso_interrumpido(t_proceso_interrumpido* proceso_interrumpido);

void liberar_memoria_t_io_gen_sleep(t_io_gen_sleep* io_gen_sleep);

void liberar_memoria_t_interfaz_diccionario(t_interfaz_diccionario* interfaz_diccionario);

void liberar_memoria_t_proceso_data(t_proceso_data* proceso_data);

void liberar_memoria_t_io_espera(t_io_espera* io_espera);

void liberar_memoria_t_recurso(t_recurso* recurso);

void liberar_memoria_t_io_direcciones_fisicas(t_io_direcciones_fisicas* io_direcciones_fisicas);

void liberar_memoria_t_io_stdin_stdout(t_io_stdin_stdout* io_stdin_stdout);

void liberar_memoria_t_io_gestion_archivo(t_io_gestion_archivo* io_gestion_archivo);

void liberar_memoria_t_io_crear_archivo(t_io_crear_archivo* io_crear_archivo);

void liberar_memoria_t_io_fs_truncate(t_io_fs_truncate* io_fs_truncate);

void liberar_memoria_t_io_readwrite_archivo(t_io_readwrite_archivo* io_readwrite_archivo);

void liberar_memoria_t_io_fs_write(t_io_fs_write* io_fs_write);

void liberar_memoria_t_proceso_recurso_diccionario(t_proceso_recurso_diccionario* proceso_recurso_diccionario);

void liberar_memoria_t_interfaz(t_interfaz* interfaz);

void liberar_memoria_t_interfaz_pid(t_interfaz_pid* interfaz_pid);

t_pcb* encontrar_proceso_pid(t_list * lista_procesos , uint32_t pid);



#endif /* INIT_KERNEL_H_ */