#include <../include/conexion.h>


//Por alguna razon la funcion de utils no corre
int iniciar_servidor_memoria(t_log* logger, char* puerto)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo -> ai_family,
							 servinfo -> ai_socktype,
							 servinfo -> ai_protocol);
							 
	bind(socket_servidor,servinfo -> ai_addr, servinfo -> ai_addrlen);
	log_info(logger, "Bind terminado");
	listen(socket_servidor, SOMAXCONN);
	log_info(logger, "Listen terminado");

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}



//Funcion que inicia y crea el socket del servido memoria y liego espera a sus clientes dandoles sus socket
void iniciar_conexiones(){
	//Iniciar server de Memoria
    fd_memoria = iniciar_servidor_memoria(logger_memoria, cfg_memoria->PUERTO_ESCUCHA);
    log_info(logger_memoria,"Inicio de server Memoria exitosamente");

    //Esperar al cliente Kernel
    log_info(logger_memoria, "Esperando a Kernel");
    fd_kernel = esperar_cliente(logger_memoria, "Kernel", fd_memoria);
    //Esperar al cliente Cpu
    log_info(logger_memoria, "Esperando Cpu");
    fd_cpu = esperar_cliente(logger_memoria, "Cpu", fd_memoria);
    //Esperar al cliente EntradaSalida
    log_info(logger_memoria, "Esperando a Entrada Salida");
    fd_entradasalida = esperar_cliente(logger_memoria, "Entrada Salida", fd_memoria);
}




// int hacer_handshake (int socket_cliente){
//     uint32_t handshake  = HANDSHAKE;

//     send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
//     return recibir_operacion(socket_cliente);
// }