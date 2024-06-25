#include <../include/conexion.h>


//Por alguna razon la funcion de utils no corre
int iniciar_servidor_memoria()
{

	int memoria_server_fd, socket_cliente;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((memoria_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

   
    if (setsockopt(memoria_server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(cfg_memoria->PUERTO_ESCUCHA); 

    // Binding el socket al puerto dado 
    if (bind(memoria_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(memoria_server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), ip_str, INET_ADDRSTRLEN);
    log_info(logger_memoria, "Servidor MEMORIA está escuchando en %s:%d...\n", ip_str, ntohs(address.sin_port));


while (1) {
        // Espera al cliente
        if ((socket_cliente = accept(memoria_server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Servidor MEMORIA recibió conexión cliente en %s:%d\n", ip_str, ntohs(address.sin_port));

        pthread_t client_thread;
        int* socket_cliente_hilo = malloc(sizeof(int));
        if (socket_cliente_hilo == NULL) {
            perror("malloc failed");
            close(socket_cliente);
            continue;
        }

        *socket_cliente_hilo = socket_cliente;

        // Crear hilos para atender al cliente
        if (pthread_create(&client_thread, NULL, memoria_atender_cliente, (void*)socket_cliente_hilo) != 0) {
            perror("pthread_create");
            free(socket_cliente_hilo);
            close(socket_cliente);
            continue;
        }
        pthread_detach(client_thread);
    }

    close(memoria_server_fd);
    return 0;

}

