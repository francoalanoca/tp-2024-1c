#include <../include/conexion.h>


//Por alguna razon la funcion de utils no corre
int iniciar_servidor_memoria(t_log* logger, char* puerto)
{

	int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8003
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); //cfg->puerto_memoria

    // Binding the socket to the port 8003
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Memoria server is listening on %s:%d...\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    while (1) {

		//ESpera el cliente
        if ((new_socket = accept(server_fd, (struct sockaddr )&address, (socklen_t)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Memoria server received connection from client at %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        pthread_t client_thread;
        int* new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }

        *new_sock = new_socket;

		//crear hilos aca?

        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock) != 0) { //funcion q atiende a todos quientes

		//crear hilos aca?
            perror("pthread_create");
            free(new_sock);
            close(new_socket);
            continue;
        }
        pthread_detach(client_thread);  // Detach the thread so that it cleans up after itself
    }

    close(server_fd);
    return 0;

}


//
//Funcion que inicia y crea el socket del servido memoria y liego espera a sus clientes dandoles sus socket
// void iniciar_conexiones(){
// 	//Iniciar server de Memoria
//     fd_memoria = iniciar_servidor_memoria(logger_memoria, cfg_memoria->PUERTO_ESCUCHA);
//     log_info(logger_memoria,"Inicio de server Memoria exitosamente");

//     //Esperar al cliente Kernel
//     log_info(logger_memoria, "Esperando a Kernel");
//     fd_kernel = esperar_cliente(logger_memoria, "Kernel", fd_memoria);
//     //Esperar al cliente Cpu
//     log_info(logger_memoria, "Esperando Cpu");
//     fd_cpu = esperar_cliente(logger_memoria, "Cpu", fd_memoria);
//     //Esperar al cliente EntradaSalida
//     log_info(logger_memoria, "Esperando a Entrada Salida");
//     fd_entradasalida = esperar_cliente(logger_memoria, "Entrada Salida", fd_memoria);
// }




// int hacer_handshake (int socket_cliente){
//     uint32_t handshake  = HANDSHAKE;

//     send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
//     return recibir_operacion(socket_cliente);
// }