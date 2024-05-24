#include <../include/conexion.h>

// int iniciar_servidor_memoria(t_log* logger, char* puerto)
// {

// 	int socket_servidor;

// 	struct addrinfo hints, *servinfo, *p;

// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_UNSPEC;
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE;

// 	getaddrinfo(NULL, puerto, &hints, &servinfo);

// 	socket_servidor = socket(servinfo -> ai_family,
// 							 servinfo -> ai_socktype,
// 							 servinfo -> ai_protocol);
							 
// 	bind(socket_servidor,servinfo -> ai_addr, servinfo -> ai_addrlen);
// 	log_info(logger, "Bind terminado");
// 	listen(socket_servidor, SOMAXCONN);
// 	log_info(logger, "Listen terminado");

// 	freeaddrinfo(servinfo);
// 	log_trace(logger, "Listo para escuchar a mi cliente");

// 	return socket_servidor;
// }




void iniciar_conexiones(){
	//Iniciar server de Memoria
    fd_memoria = iniciar_servidor(logger_memoria, "Memoria", "8002", cfg_memoria->PUERTO_ESCUCHA);
    log_info(logger_memoria,"Inicio de server Memoria exitosamente");

    //Esperar al cliente Kernel
    log_info(logger_memoria, "Conexion con Kernel exitosa");
    fd_kernel = esperar_cliente(logger_memoria, "Kernel", fd_memoria);
    //Esperar al cliente Cpu
    log_info(logger_memoria, "Conexion con Cpu exitosa");
    fd_cpu = esperar_cliente(logger_memoria, "Cpu", fd_memoria);
    //Esperar al cliente EntradaSalida
    log_info(logger_memoria, "Conexion con Entrada Salida exitosa");
    fd_entradasalida = esperar_cliente(logger_memoria, "Entrada Salida", fd_memoria);
}




// int hacer_handshake (int socket_cliente){
//     uint32_t handshake  = HANDSHAKE;

//     send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
//     return recibir_operacion(socket_cliente);
// }