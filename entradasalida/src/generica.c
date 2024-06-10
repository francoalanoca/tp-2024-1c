#include "../include/generica.h"


void iniciar_interfaz_generica (int socket_kernel) {
    
    uint32_t response;
    op_code cop;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    t_list* lista_paquete =  malloc(sizeof(t_list));
    t_io_espera* io_espera = malloc(sizeof(t_io_espera));

    log_info(logger_entrada_salida, "Interfaz %s de tipo GENERICA iniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  
    
    while (socket_kernel != -1) {

        if (recv(socket_kernel, &cop, sizeof(int32_t), MSG_WAITALL) != sizeof(int32_t)) {
            log_info(logger_entrada_salida, "DISCONNECT!");

            break;
        }
    switch (cop) {
            
            case HANDSHAKE :

                log_info(logger_entrada_salida, "Handshake realizado con Kernel");
                response = HANDSHAKE_OK;
                if (send(socket_kernel, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Error al enviar respuesta de handshake a kernel");
                    free(paquete);
                    break;
                }
                break;

            case HANDSHAKE_OK :

                log_info(logger_entrada_salida, "Handshake recibido handshake exitosamente con Kernel");
                
                break;                
            
            case IO_K_GEN_SLEEP :
                
                log_info(logger_entrada_salida, "IO_K_GEN_SLEEP recibida desde Kernel");
                    
                lista_paquete = recibir_paquete(socket_kernel);
                io_espera = deserializar_espera (lista_paquete);
                esperar(io_espera->pid, io_espera->tiempo_espera);

                response = IO_K_GEN_SLEEP_FIN;

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                    break;
                }
                break;

            default:
                response = OPERACION_INVALIDA;
                if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Operacion invalida enviada desde kernel");
                    break;
                }
                break;
        }
}

}

 t_io_espera* deserializar_espera (t_list*  lista_paquete ){

       t_io_espera* io_espera = malloc(sizeof(t_io_espera));
       io_espera->pid = list_get(lista_paquete, 0);
       io_espera->tiempo_espera = list_get(lista_paquete, 1);

    return io_espera;

}

void esperar(int32_t pid, int32_t unidades ){

    log_info(logger_entrada_salida, "PID: %d - Operacion: IO_GEN_SLEEP ",pid);
    log_info(logger_entrada_salida, "Se solicitaron  %d unidades de trabajo",unidades);
    sleep (unidades * cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO);
    log_info(logger_entrada_salida, "Se completaron  %d unidades de trabajo",unidades);
}
