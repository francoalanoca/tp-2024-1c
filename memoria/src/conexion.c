#include <../include/conexion.h>


//Por alguna razon la funcion de utils no corre
int iniciar_servidor_memoria()
{

	int memoria_server_fd, socket_cliente;
    int err;
    char ip_str[INET_ADDRSTRLEN];
    struct addrinfo hints, *server_info;
    char puerto[6];
    struct addrinfo *p ;
    sprintf(puerto, "%d", cfg_memoria->PUERTO_ESCUCHA);
    
    memset(&hints, 0, sizeof(hints)); 
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    err = getaddrinfo(NULL, puerto, &hints, &server_info);
   
      for (p= server_info; p != NULL; p = p->ai_next) {
        if ((memoria_server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Falló el socket");
            continue;
        }
        if (bind(memoria_server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(memoria_server_fd);
            perror("Falló el bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Falló el bind con todas las direcciones\n");
        exit(EXIT_FAILURE);
    }

    inet_ntop(server_info->ai_family, server_info->ai_addr, ip_str, sizeof(ip_str));
    freeaddrinfo(server_info);
       
    if (listen(memoria_server_fd, SOMAXCONN) < 0) {
        perror("Falló el listen");
        exit(EXIT_FAILURE);
    }    
     
    log_info(logger_memoria, "Servidor MEMORIA está escuchando en %s:%d...\n", ip_str, cfg_memoria->PUERTO_ESCUCHA);


while (1) {
        // Espera al cliente
        if ((socket_cliente = accept(memoria_server_fd, NULL,NULL)) < 0) {
            perror("Falló aceptar");
            exit(EXIT_FAILURE);
        }

        printf("Servidor MEMORIA recibió conexión cliente en %s:%d...\n", ip_str, cfg_memoria->PUERTO_ESCUCHA);

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

