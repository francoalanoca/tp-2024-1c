#include "../include/stdin.h"

#define TAMANIO_INICIAL_BUFFER 10

char *input;
 

void iniciar_interfaz_stdin (int socket_kernel, int socket_memoria) {
    
    uint32_t response;
    op_code cop;


    log_info(logger_entrada_salida, "Interfaz %s de tipo STDIN iniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  
    
    while (socket_kernel != -1) {

        if (recv(socket_kernel, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger_entrada_salida, "DISCONNECT!");

            break;
        }
    switch (cop) {
            
            case HANDSHAKE :

                log_info(logger_entrada_salida, "Handshake realizado con Kernel");
                response = HANDSHAKE;
                if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Error al enviar respuesta de handshake a kernel");
                    break;
                }
                break;
            
            case IO_K_STDIN:

                esperar_ingreso_teclado();
                
                response = enviar_input(socket_memoria);

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar respuesta de STDIN a Kernel");
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

 int esperar_ingreso_teclado(){
  

    size_t size = TAMANIO_INICIAL_BUFFER;
    size_t len = 0;
    int ch;

    input = (char *)malloc(TAMANIO_INICIAL_BUFFER * sizeof(char));
    if (input == NULL) {
        log_error(logger_entrada_salida, "Error al asignar memoria a input");
        return 1;
    }
    

    printf("Por favor, ingresá una cadena de texto: ");
    // levanta caracter a caracter hasta que encuentra un "enter" de la variable stdin
    while ((ch = getchar()) != '\n' && ch != EOF) {
        input[len++] = ch;

        // Si se alcanza el tamaño del buffer, redimensionarlo al doble
        if (len == size) {
            size *= 2;
            char *new_input = (char *)realloc(input, size * sizeof(char));
            if (new_input == NULL) {
                log_error(logger_entrada_salida, "Error al redimensionar memoria.\n");                
                free(input);
                return 1;
            }
            input = new_input;
        }
    }
    input[len] = '\0'; // Agrega el terminador nulo al final de la cadena
    log_info(logger_entrada_salida, "Se ingreso la cadena: %s\n", input);

    //free(input); // Libera la memoria asignada
    return 0;
      
  
}

op_code  enviar_input(int socket_memoria) {
    t_paquete* paquete_input;
 
    paquete_input = crear_paquete(IO_M_STDIN);
 
    agregar_a_paquete(paquete_input, input,  sizeof(input));    
    enviar_paquete(paquete_input, socket_memoria);    
    if ( recibir_operacion(socket_memoria) == IO_M_STDIN_FIN ){
        log_info(logger_entrada_salida, "Se guardo inpunt correctamente en memoria: \n");

        return IO_K_STDIN_FIN;
        
        }
    else {
        
        log_error(logger_entrada_salida, "Error al intentar guardar input en  memoria.\n");
        return -1; 
    }          

}