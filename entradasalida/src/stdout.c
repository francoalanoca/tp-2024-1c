#include "../include/stdout.h"


void iniciar_interfaz_stdout (int socket_kernel, int socket_memoria) {
    
    op_code response;
    op_code cop;
     t_list* lista_paquete =  malloc(sizeof(t_list));

    log_info(logger_entrada_salida, "Interfaz %s de tipo STDOUTiniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  
    
    while (socket_kernel != -1) {

        if (recv(socket_kernel, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
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
            case IO_K_STDOUT:{
                        
                log_info(logger_entrada_salida, "Recibido IO_K_STDOUT desde kernel");
                
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_direcciones_fisicas* io_stdout = deserializar_io_df(lista_paquete);
                
                //reenvio la solicitud a memoria
                enviar_io_df(io_stdout, socket_memoria, IO_M_STDOUT);
               
                //Espero respuesta de memoria
                if (recv(socket_memoria, &cop, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                  log_info(logger_entrada_salida, "DISCONNECT!");
                 }

                 if (cop == IO_M_STDOUT_FIN) {

                    log_info(logger_entrada_salida, "Recibido IO_M_STDOUT_FIN desde Memoria");
                   
                    t_list* lista_paquete_nueva = list_create();            
                    
                    lista_paquete_nueva = recibir_paquete(socket_memoria);
              
                    
                    t_io_output* io_output_recibido = deserializar_output(lista_paquete_nueva);
                    printf("Output recibido:  %s \n",io_output_recibido->output);
                 
                    
                    free(io_output_recibido);
                    response = IO_K_STDOUT_FIN;
                    
                    enviar_respuesta_io (socket_kernel,response,io_stdout->pid, cfg_entrada_salida->NOMBRE_INTERFAZ);
                    free_io_output(io_output_recibido);                    
                    list_destroy_and_destroy_elements(lista_paquete_nueva, free);
                    free_io_direcciones_fisicas(io_stdout);
                 }

                

                 
                
                break;
                }
            default:
                response = OPERACION_INVALIDA;
                if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Operacion invalida enviada desde kernel");
                    break;
                }
                break;
        }
}
list_destroy_and_destroy_elements(lista_paquete, free);
}

 
 


