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
int server_escuchar(t_log *logger, char *server_name, int server_socket);
void procesar_conexion(void *v_args);
int hacer_handshake (int socket_cliente);
void buffer_read(t_buffer *buffer, void *data, uint32_t size);
uint8_t buffer_read_uint8(t_buffer *buffer);
uint32_t buffer_read_uint32(t_buffer *buffer);
t_pcb *buffer_read_pcb(t_buffer *buffer, uint32_t *length);
instr_t *buffer_read_instruccion(t_buffer *buffer, uint32_t *length);
t_interfaz *buffer_read_interfaz(t_buffer *buffer, uint32_t *length);
char *buffer_read_string(t_buffer *buffer, uint32_t *length);
t_proceso *proceso_deserializar(t_buffer *buffer);
void* crear_servidor_interrupt(char* ip_cpu);
t_proceso_interrumpido *proceso_interrumpido_deserializar(t_buffer *buffer);
instr_t* instruccion_deserializar(t_buffer *buffer);
tipo_instruccion buffer_read_tipo_instruccion(t_buffer *buffer);

//void *conectarConMemoria();
//void cortar_conexiones();
//void cerrar_servers();
//void procesoTerminado();

#endif //TP_2024_1C_PASARONCOSAS_SERVIDORCPU_H