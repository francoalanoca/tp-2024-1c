#include "../include/generica.h"


void iniciar_interfaz_generica (int socket_kernel) {
    
    uint32_t response;
    op_code cop;
 
   

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
                    
                    break;
                }
                break;

            case HANDSHAKE_OK :

                log_info(logger_entrada_salida, "Handshake recibido handshake exitosamente con Kernel");
                
                break;                
            
            case IO_K_GEN_SLEEP :
                
                log_info(logger_entrada_salida, "IO_K_GEN_SLEEP recibida desde Kernel");
                t_list* lista_paquete = list_create();   
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_espera* io_espera = deserializar_espera (lista_paquete);
                esperar(io_espera->pid, io_espera->tiempo_espera);
                list_destroy_and_destroy_elements(lista_paquete, free);
                free(io_espera);
                response = IO_K_GEN_SLEEP_FIN;

                enviar_respuesta_io (socket_kernel,response,io_espera->pid, cfg_entrada_salida->NOMBRE_INTERFAZ);
                list_destroy_and_destroy_elements(lista_paquete, free);

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
       io_espera->pid = *(uint32_t*)list_get(lista_paquete, 0);
       io_espera->tiempo_espera = *(uint32_t*)list_get(lista_paquete, 1);

    return io_espera;    
}

void esperar(int32_t pid, int32_t unidades ){

    log_info(logger_entrada_salida, "PID: %d - Operacion: IO_GEN_SLEEP ",pid);
    log_info(logger_entrada_salida, "Se solicitaron  %d unidades de trabajo",unidades);
    sleep (unidades * cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO);
    log_info(logger_entrada_salida, "Se completaron  %d unidades de trabajo",unidades);
}

