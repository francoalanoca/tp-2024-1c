#include "../include/stdin.h"

#define TAMANIO_INICIAL_BUFFER 10

char *input;


void iniciar_interfaz_stdin (int socket_kernel, int socket_memoria) {
    
    op_code response;
    op_code cop;
     t_list* lista_paquete =  malloc(sizeof(t_list));

    log_info(logger_entrada_salida, "Interfaz %s de tipo STDIN iniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  
    
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
            case IO_K_STDIN:
                t_io_input* io_input = malloc(sizeof(t_io_input));
                t_io_direcciones_fisicas* io_stdin = malloc(sizeof(t_io_direcciones_fisicas));
                
                log_info(logger_entrada_salida, "Recibido IO_K_STDIN desde kernel");
                
                lista_paquete = recibir_paquete(socket_kernel);
                io_stdin = deserializar_io_df(lista_paquete);
                
                esperar_ingreso_teclado();

                //armo la estructura input para enviar                
                io_input->pid =io_stdin->pid;
                io_input->direcciones_fisicas =list_create();
                list_add_all(io_input->direcciones_fisicas,io_stdin->direcciones_fisicas);
                io_input->input_length = string_length(input) + 1;
                io_input->input = input;
                
                enviar_input(io_input, socket_memoria, IO_M_STDIN);                
               
                free(io_input);
                break;
            case IO_M_STDIN_FIN:
               
                log_info(logger_entrada_salida, "Se guardo inpunt correctamente en memoria: \n");
                response = IO_K_STDIN_FIN;        

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar respuesta de IO_K_STDIN_FIN a Kernel");
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

}

 int esperar_ingreso_teclado() {
    size_t size = TAMANIO_INICIAL_BUFFER;
    size_t len = 0;
    int ch;

    input = (char *)malloc(TAMANIO_INICIAL_BUFFER * sizeof(char));
    if (input == NULL) {
        log_error(logger_entrada_salida, "Error al asignar memoria a input");
        return 1;
    }

    printf("Por favor, ingresá una cadena de texto: ");
    fflush(stdout); // Asegúrate de que la salida se vacíe

    // levanta caracter a caracter hasta que encuentra un "enter" de la variable stdin
    while ((ch = getchar()) != '\n' && ch != EOF) {
        printf("Caracter leído: %c (Código ASCII: %d)\n", ch, ch); // Depuración
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

    if (ch == '\n') {
        printf("Caracter de nueva línea leído, saliendo del bucle.\n"); // Depuración
    } else if (ch == EOF) {
        printf("EOF leído, saliendo del bucle.\n"); // Depuración
    }

    input[len] = '\0'; // Agrega el terminador nulo al final de la cadena
    log_info(logger_entrada_salida, "Se ingresó la cadena: %s\n", input);

    // free(input); // Libera la memoria asignada si es necesario en otro lugar
    return 0;
}


