#include "../include/generica.h"


void iniciar_interfaz_generica (int socket_kernel) {
    
    uint32_t response;
    op_code cop;

    log_info(logger_entrada_salida, "Interfaz %s de tipo GENERICA iniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  
    
    while (socket_kernel != -1) {

        if (recv(socket_kernel, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger_entrada_salida, "DISCONNECT!");

            break;
        }
    switch (cop) {
            
            case HANDSHAKE :

                log_info(logger_entrada_salida, "Handshake realizado con Kernel");
                response = HANDSHAKE_OK;
                if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Error al enviar respuesta de handshake a kernel");
                    break;
                }
                break;

            case HANDSHAKE_OK :

                log_info(logger_entrada_salida, "Handshake recibido handshake exitosamente con Kernel");
 
                
                break;                
            
            case IO_K_GEN_SLEEP :

                esperar(5);
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

void esperar(int unidades){
    log_info(logger_entrada_salida, "Se solicitaron  %d unidades de trabajo",unidades);
    sleep (unidades * cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO);
    log_info(logger_entrada_salida, "Se completaron  %d unidades de trabajo",unidades);
}
