#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>

int main(int argc, char* argv[]) {
    decir_hola("Memoria");
    
    logger = log_create("memoria.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");

	while (1) {
		int cliente_fd = esperar_cliente(server_fd);
        //inicio handshake
        size_t bytes;

        int32_t handshake;
        int32_t resultOk = 0;
        int32_t resultError = -1;

        bytes = recv(cliente_fd, &handshake, sizeof(int32_t), MSG_WAITALL);
        if (handshake == 1) {
            bytes = send(cliente_fd, &resultOk, sizeof(int32_t), 0);
        } else {
            bytes = send(cliente_fd, &resultError, sizeof(int32_t), 0);
        }
        //fin handshake

        t_list* lista;

		while (1) {
			int cod_op = recibir_operacion(cliente_fd);
			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cliente_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(cliente_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				//log_error(logger, "El cliente se desconecto.");
				close(cliente_fd); // Cerrar el descriptor del cliente desconectado
				goto outside_loop; // Salir del bucle interno
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
//comentario
        outside_loop:
        // Salir del bucle interno y esperar por un nuevo cliente
        continue;
	}

	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
