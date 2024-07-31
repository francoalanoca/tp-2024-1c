#ifndef TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H
#define TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <utils/utils.h>
#include "../include/init_cpu.h"

bool generar_conexiones();
void* crear_servidor_dispatch(char* ip_cpu);
int server_escuchar_interrupt(t_log *logger, char *server_name, int server_socket, int *global_socket);
int server_escuchar_dispatch(t_log *logger, char *server_name, int server_socket, int *global_socket);
void procesar_conexion_interrupt(void *v_args);
void procesar_conexion_dispatch(void *v_args);
int hacer_handshake (int socket_cliente);
t_pcb *proceso_deserializar(t_list*  lista_paquete_proceso ) ;
void* crear_servidor_interrupt(char* ip_cpu);
t_proceso_interrumpido *proceso_interrumpido_deserializar(t_list*  lista_paquete_proceso_interrumpido);
instr_t* instruccion_deserializar(t_list* lista_paquete_inst);
uint32_t deserealizar_marco(t_list*  lista_paquete );
char* deserealizar_valor_memoria(t_list*  lista_paquete );
uint32_t deserealizar_tamanio_pag(t_list*  lista_paquete );
t_rta_resize* deserealizar_rta_resize(t_list*  lista_paquete );
void atender_memoria (int *socket);
void armar_instr(instr_t *instr, const char *input);
//void *conectarConMemoria();
//void cortar_conexiones();
//void cerrar_servers();
//void procesoTerminado();

#endif //TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H