#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>
#include <../include/conexion.h>

int main(int argc, char* argv[]) {
	//char* pathconf = argv[1];
	char* pathconf = "memoria.config";
	t_config* config;
	char* puerto;
	bool flag_conexion_activa;
	char* nombre = "cliente";

	// Hacer los if correspondientes en caso de fallar
    logger = log_create("memoria.log", "Memoria LOG", 1, LOG_LEVEL_DEBUG);
	config = iniciar_config(pathconf, logger);

	 if (config_save(config) == -1) {
        printf("Error al guardar el archivo de configuración.\n");
    }
	puerto = config_get_string_value(config,"PUERTO_ESCUCHA");
	int server_fd = iniciar_servidor_memoria(logger,puerto);
	log_info(logger, "Servidor listo para recibir al cliente");

	while (1) {
		int cliente_fd = esperar_cliente(logger, nombre, server_fd);
        //inicio handshake
        size_t bytes;

        int32_t handshake;
        int32_t resultOk = 0;
        int32_t resultError = -1;

        bytes = recv(cliente_fd, &handshake, sizeof(int32_t), MSG_WAITALL);
        if (handshake == HANDSHAKE) {
            bytes = send(cliente_fd, &resultOk, sizeof(int32_t), 0);
			flag_conexion_activa = true;
			log_info(logger, "Handshake salió bien, un gusto");
        } else {
            bytes = send(cliente_fd, &resultError, sizeof(int32_t), 0);
			flag_conexion_activa = false;
			log_warning(logger, "Handshake salió mal, no te conozco");
        }
        //fin handshake

        t_list* lista;

		while (flag_conexion_activa) {
			int cod_op = recibir_operacion(cliente_fd);
			switch (cod_op) {
			case PROXIMA_INSTRUCCION://CPU me pide su sig instruccion
				//abrir archivo de instrucciones (buscar como leer un archivo linea por linea)
				//ubicar instruccion a partir PC que me envia cpu
				//enviar codigo de operacion a cpu para que espere la instruccion(armar paquete con instruccion y codigo de opercion nuevo)
				break;
			case INTERFAZ_IO://interfaz de io que me tiene que pasar
				lista = recibir_paquete(cliente_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_error(logger, "Fallo en el envío de paquete");
				close(cliente_fd); // Cerrar el descriptor del cliente desconectado
				flag_conexion_activa = false;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}

	}
	log_destroy(logger);
	config_destroy(config);
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
