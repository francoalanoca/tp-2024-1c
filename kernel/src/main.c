#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <kernel.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

// Definiciones globales
t_log *logger;
t_config_kernel *config_kernel;

void iniciar_config_kernel(){
    config_kernel = malloc(sizeof(t_config_kernel));
     config = config_create("/home/utnso/tp-2024-1c-Pasaron-cosas/kernel/config/kernel.config");
    if (config == NULL) {
        perror("No se pudo encontrar el path del config");
        exit(EXIT_FAILURE);
    }
      config_kernel->PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    log_info(logger,"PUERTO_ESCUCHA:%d",config_kernel->PUERTO_ESCUCHA);
    config_kernel->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    log_info(logger,"IP_MEMORIA:%s", config_kernel->IP_MEMORIA);
    config_kernel->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
    log_info(logger,"PUERTO_MEMORIA:%d",config_kernel->PUERTO_MEMORIA);
    config_kernel->IP_CPU = config_get_string_value(config, "IP_CPU");
    log_info(logger,"IP_CPU:%s", config_kernel->IP_CPU);
    config_kernel->PUERTO_CPU_DISPATCH = config_get_int_value(config, "PUERTO_CPU_DISPATCH");
    log_info(logger,"PUERTO_CPU_DISPATCH:%d",config_kernel->PUERTO_CPU_DISPATCH);
    config_kernel->PUERTO_CPU_INTERRUPT = config_get_int_value(config, "PUERTO_CPU_INTERRUPT");
    log_info(logger,"PUERTO_CPU_ITERRUPT:%d",config_kernel->PUERTO_CPU_INTERRUPT);
}

planificador= inicializar_planificador (obtener_algoritmo_planificador(), config_kernel-> QUANTUM);


int conectar_a(char *ip, int puerto) {
    int sockfd;
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_error(logger, "Error al crear el socket");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(puerto);
   
    if (connect(sockfd, (void*)&serv_addr, sizeof(serv_addr)) !=0) {
        log_error(logger, "Error al conectar al servidor");
      
        return -1;
    }
    printf("socket:%d",sockfd);
    return sockfd;
}

int main() {
    logger = log_create("kernel.log","kernel-log", 1, LOG_LEVEL_DEBUG);
    iniciar_config_kernel();
    
    
    // Conexión con Memoria (cliente)
    int conexion_memoria = conectar_a(config_kernel->IP_MEMORIA, config_kernel->PUERTO_MEMORIA);
    if (conexion_memoria == -1) {
        log_error(logger, "Error al conectar con la memoria");
        return EXIT_FAILURE;
    }

    // Conexión con CPU (cliente)
    int conexion_cpu = conectar_a(config_kernel->IP_CPU, config_kernel->PUERTO_CPU_DISPATCH);
    if (conexion_cpu == -1) {
        log_error(logger, "Error al conectar con la CPU");
        return EXIT_FAILURE;
    }

    
    // Configuración del socket servidor
    struct sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = INADDR_ANY;
    direccionServidor.sin_port = htons(config_kernel->PUERTO_ESCUCHA);
    int servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(servidor, (void*)&direccionServidor, sizeof(direccionServidor)) != 0) {
        perror("Fallo el bind");
        return EXIT_FAILURE;
    }
    printf("Estoy escuchando\n");
    listen(servidor, SOMAXCONN);

     //Aceptar conexiones entrantes
    struct sockaddr_in direccionCliente;
    unsigned int tamañoDireccion;
 
 while (1) {
    pthread_t thread;
        int *fd_conexion_ptr = malloc(sizeof(int));
       *fd_conexion_ptr = accept(servidor, (void*)&direccionCliente, &tamañoDireccion);
        pthread_create(&thread, NULL, (void*)atender_cliente, fd_conexion_ptr);
        pthread_detach(thread);
    }
    

    return EXIT_SUCCESS;
}


void liberar_config() {
    free(config_kernel->IP_MEMORIA);
    free(config_kernel);
    config_destroy(config);
}

void terminar_programa_kernel(int conexion,t_log* logger,t_config* config){
    liberar_config();
    log_destroy(logger);
    close(conexion_cpu);
    close(conexion_memoria);
}
void atender_cliente(void *arg) {// nos tira este error:"message": "cast from pointer to integer of different size [-Wpointer-to-int-cast]",
    int servidor= ((int)arg);
  
    char *mensaje_bienvenida = "¡Bienvenido al servidor!";
    send(servidor, mensaje_bienvenida, strlen(mensaje_bienvenida), 0);
    
    char buffer[1024];
    recv(servidor, buffer, sizeof(buffer), 0);
    printf("Mensaje recibido del cliente: %s\n", buffer);
    
  
    close(servidor);
    free(arg);
    
    pthread_exit(NULL);
}
