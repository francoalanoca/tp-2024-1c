#include<../include/conexion.h>

int iniciar_servidor_memoria(t_log* logger, char* puerto)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

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