#include"utils.h"

t_log* logger;

typedef struct {
    t_log *log;
    int fd;
    char *server_name;
} t_procesar_conexion_args;

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto) {
    //printf("ip: %s, puerto: %s", ip, puerto);
    log_info(logger, "comienza iniciar_servidor");
    
    int socket_servidor;
    struct addrinfo hints, *servinfo;

    // Inicializando hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    log_info(logger, "hizo memset");
    log_info(logger, "ip:%s, puerto:%d", ip,puerto);
    // Recibe los addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);
    log_info(logger, "hizo getaddrinfo");

    bool conecto = false;
    log_info(logger, "antes del for");
    // Itera por cada addrinfo devuelto
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_servidor == -1) // fallo de crear socket
            continue;
        int yes=1;
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            // Si entra aca fallo el bind
            handle_error("bind");
            close(socket_servidor);
            continue;
        }
        // Ni bien conecta uno nos vamos del for
        conecto = true;
        break;
    }
     log_info(logger, "despues del for");

    if(!conecto) {
        free(servinfo);
        return 0;
    }

    listen(socket_servidor, SOMAXCONN); // Escuchando (hasta SOMAXCONN conexiones simultaneas)

    // Aviso al logger
    log_info(logger, "Escuchando en puerto: %s (%s)\n", ip, puerto, name);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(t_log* logger, const char* name, int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

    log_info(logger, "Cliente conectado (a %s)\n", name);

    return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
//

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto) {
    struct addrinfo hints, *servinfo;

    // Init de hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    errno = 0;
    // Crea un socket con la informacion recibida (del primero, suficiente)
    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    int yes=1;
    setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    // Fallo en crear el socket
    if(socket_cliente == -1) {
        printf("socket() devolvio: %s \n",strerror(errno));
        log_error(logger, "Error creando el socket para %s", ip, puerto);
        log_error(logger, "y puerto %s", puerto);
        //freeaddrinfo(servinfo);
        return 0;
    }

    // Error conectando
    if(connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(logger, "Error al conectar (a %s)\n", server_name);
        freeaddrinfo(servinfo);
        return 0;
    } else
        log_info(logger, "Cliente conectado en %s:%s (a %s)\n", ip, puerto, server_name);

    freeaddrinfo(servinfo);

    return socket_cliente;
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

bool config_has_all_properties(t_config *cfg, char **properties)
{
    for (uint8_t i = 0; properties[i] != NULL; i++)
    {
        if (!config_has_property(cfg, properties[i]))
            return false;
    }
    return true;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


t_config* iniciar_config(char* path_config) {
    t_config* nuevo_config;
    if((nuevo_config = config_create(path_config)) == NULL){ //config_create: Devuelve un puntero hacia la estructura creada o NULL en caso de no encontrar el archivo en el path especificado
        printf("No pude leer la config");
        exit(2);
    }
    return nuevo_config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	  log_destroy(logger);
	  config_destroy(config);
	  liberar_conexion(conexion);

}

int recibir_informacion(int conexion, t_log* logger){
//inicio handshake
        size_t bytes;

        int32_t handshake;
        int32_t resultOk = 0;
        int32_t resultError = -1;

        bytes = recv(conexion, &handshake, sizeof(int32_t), MSG_WAITALL);
        if (handshake == 1) {
            bytes = send(conexion, &resultOk, sizeof(int32_t), 0);
        } else {
            bytes = send(conexion, &resultError, sizeof(int32_t), 0);
        }
        //fin handshake

        t_list* lista;

		while (1) {
			
			int cod_op = recibir_operacion(conexion);
			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(conexion);
				break;
			case PAQUETE:
				lista = recibir_paquete(conexion);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				//log_error(logger, "El cliente se desconecto.");
				close(conexion); // Cerrar el descriptor del cliente desconectado
				return -1;//goto outside_loop; // Salir del bucle interno
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
        //outside_loop:
        // Salir del bucle interno y esperar por un nuevo cliente
        //continue;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

void handshake_cliente(t_config* config, t_log* logger, int conexion){
	 //inicio handshake
   // enviar_handshake(conexion);
    size_t bytes;

int32_t handshake = 1;
int32_t result;

bytes = send(conexion, &handshake, sizeof(int32_t), 0);
bytes = recv(conexion, &result, sizeof(int32_t), MSG_WAITALL);

if (result == 0) {
    // Handshake OK
    log_info(logger,"Handshake OK");
} else {
    // Handshake ERROR
    log_info(logger,"Handshake ERROR");
}
    //fin handshake
}
/*
void* crearServidor(t_log* logger){
int fd_mod2 = -1;
  //  puerto_kernel = cfg_kernel->PUERTO_ESCUCHA;
    fd_mod2 = iniciar_servidor2(logger, "SERVER KERNEL", "127.0.0.1", "4444");
    if (fd_mod2 == 0) {
        log_error(logger, "Fallo al crear el servidor, cerrando KERNEL");
        return EXIT_FAILURE;
    }

    while (server_escuchar(logger, "SERVER KERNEL", (uint32_t)fd_mod2));
}
int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    int cliente_socket = esperar_cliente2(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

void procesar_conexion(void *void_args) {
    t_procesar_conexion_args *args = (t_procesar_conexion_args *) void_args;
    t_log *logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    while (cliente_socket != -1) {

        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

        switch (cop) {
            
            case -1:
                log_error(logger, "Cliente desconectado de %s...", server_name);
                return;
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
        }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}

int esperar_cliente2(t_log* logger, const char* name, int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

    log_info(logger, "Cliente conectado (a %s)\n", name);

    return socket_cliente;
}
*/