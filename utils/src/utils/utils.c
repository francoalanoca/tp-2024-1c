#include "utils.h"

t_log* logger;



/*
typedef struct {
    t_log *log;
    int fd;
    char *server_name;
} t_procesar_conexion_args;*/

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
    //log_info(logger, "ip:%s, puerto:%s", ip,puerto);
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
    log_info(logger, "Escuchando en IP %s, puerto: %s (%s)\n", ip, puerto, name);

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

int recibir_operacion(int socket_cliente) // modificar 
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
        log_error(logger, "Error creando el socket para IP %s", ip);
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

t_paquete* crear_paquete(op_code codigo_operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	crear_buffer(paquete);
	return paquete;
}
void agregar_a_buffer(t_buffer* un_buffer, void* valor, int tamanio)
{
	//si el buffer esta vacio
	if (un_buffer->size == 0)
	{
		//reservamos memoria para su size y el int
		un_buffer->stream = malloc(sizeof(int) + tamanio);
		//copiamos en el buffer lo que ingreso
		memcpy(un_buffer->stream, &tamanio, sizeof(int));
		//nos desplazamos y copiamos en el buffer el tamanio de lo que ingreso
		memcpy(un_buffer->stream + sizeof(int), valor, tamanio);
	}
	else{ //sino estaba vacio
		//agreandamos el espacio en memoria a lo que necesitemos
		un_buffer->stream = realloc(un_buffer->stream, un_buffer->size + tamanio + sizeof(int));
		//copiamos en el buffer lo que ingreso
		memcpy(un_buffer->stream + un_buffer->size, &tamanio, sizeof(int));
		//nos desplazamos y copiamos en el buffer el tamanio de lo que ingreso
		memcpy(un_buffer->stream + un_buffer->size + sizeof(int), valor, tamanio);
	}
	

	//actualizamos el buffer
	un_buffer->size += sizeof(int);
	un_buffer->size += tamanio;
	//un_buffer->size += tamanio + sizeof(int);
}

void eliminar_buffer(t_buffer* un_buffer)
{
	if (un_buffer != NULL)
	{
		free(un_buffer->stream);
	}
	free(un_buffer);
}

void* extraer_contenido_del_buffer(t_buffer* un_buffer)
{
	if (un_buffer->size == 0)
	{
		printf("\n[ERROR] Al intentar extrar contenido de un t_buffer vacio \n\n");
		exit(EXIT_FAILURE);

	}
	
	if (un_buffer->size < 0)
	{
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size negativo \n\n");
		exit(EXIT_FAILURE);
	}
	
	int tamanio_contenido;
	memcpy(&tamanio_contenido, un_buffer->stream, sizeof(int));
	void* contenido = malloc(tamanio_contenido);
	memcpy(contenido, un_buffer->stream + sizeof(int), tamanio_contenido);

	int nuevo_tamanio = un_buffer->size - sizeof(int) - tamanio_contenido;


	if (nuevo_tamanio == 0)
	{
		un_buffer->size = 0;
		free(un_buffer->stream);
		un_buffer->stream = NULL;
		return contenido;
	}
	
	if (nuevo_tamanio < 0)
	{
		perror("\n[ERROR] Buffer con tamanio negativo");
		exit(EXIT_FAILURE);
	}
	
	void* nuevo_stream = malloc(nuevo_tamanio);
	memcpy(nuevo_stream, un_buffer->stream + sizeof(int) + tamanio_contenido, nuevo_tamanio);
	free(un_buffer->stream);
	un_buffer->size = nuevo_tamanio;
	un_buffer->stream = nuevo_stream;

	return contenido;

}


char* extraer_string_del_buffer(t_buffer* un_buffer)
{
	char* valor_string = extraer_contenido_del_buffer(un_buffer);
	return valor_string;
}

//Agrega una variable de tipo int al buffer
void cargar_int_al_buffer(t_buffer* un_buffer, int tamanio_int)
{
	agregar_a_buffer(un_buffer, &tamanio_int, sizeof(int));
}

//Agrega una variable de tipo uint32 al buffer
void cargar_uint32_al_buffer(t_buffer* un_buffer, uint32_t tamanio_uint32)
{
	agregar_a_buffer(un_buffer, &tamanio_uint32, sizeof(uint32_t));
}

//Agrega una variable de tipo string al buffer
void cargar_string_al_buffer(t_buffer* un_buffer, char* tamanio_string)
{
	agregar_a_buffer(un_buffer, tamanio_string, strlen(tamanio_string) + 1);

}
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
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



bool config_has_all_properties(t_config *cfg, char **properties)
{
    for (uint8_t i = 0; properties[i] != NULL; i++)
    {
        if (!config_has_property(cfg, properties[i]))
            return false;
    }
    return true;
}

t_config* iniciar_config(char* path_config, t_log* logger) {
    t_config* nuevo_config;
    if((nuevo_config = config_create(path_config)) == NULL){ //config_create: Devuelve un puntero hacia la estructura creada o NULL en caso de no encontrar el archivo en el path especificado
        printf("No pude leer la config"); // Usar loggs
        log_error(logger, "No se pudo leer la config :(");
		//exit(2); // Por qué 2???
		exit(EXIT_FAILURE);
    }
    return nuevo_config;

}

void imprimir_stream(void* stream, int size) {
    unsigned char* bytes = (unsigned char*)stream;
    for (int i = 0; i < size; i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

t_tipo_interfaz_enum obtener_tipo_interfaz_enum (const char* tipo_interfaz_str) {
    if (strcmp(tipo_interfaz_str, "GENERICA") == 0) {
        return GENERICA ;
    } else if (strcmp(tipo_interfaz_str, "STDIN") == 0) {
        return STDIN ;
    } else if (strcmp(tipo_interfaz_str, "STDOUT") == 0) {
        return STDOUT ;
    } else if (strcmp(tipo_interfaz_str, "DIALFS") == 0) {
        return DIALFS ;    
    } else {
        // Manejo de error para tipos de interfaz desconocidos
        return -1; 
    }
}
// Kernel recibe una interfaz con op_cod = INTERFAZ_ENVIAR
 t_interfaz* deserializar_interfaz(t_list*  lista_paquete ){

    t_interfaz* interfaz = malloc(sizeof(t_interfaz));

    interfaz->nombre_length = *(uint32_t*)list_get(lista_paquete, 0);
    interfaz->nombre = list_get(lista_paquete, 1);
    interfaz->tipo = *(uint32_t*)list_get(lista_paquete, 2);
    
	return interfaz;
}

// kernel envia el tiempo a esperar a IO
void enviar_espera(t_io_espera* io_espera, int socket){

    t_paquete* paquete_espera = malloc(sizeof(t_paquete)); 

    paquete_espera = crear_paquete(IO_K_GEN_SLEEP); 
    agregar_a_paquete(paquete_espera, &io_espera->pid, sizeof(io_espera->pid));  
    agregar_a_paquete(paquete_espera, &io_espera->tiempo_espera, sizeof(io_espera->tiempo_espera));      
       
    enviar_paquete(paquete_espera, socket);  

}
// usar en memoria cuando recibe IO_M_STDIN
 t_io_input* deserializar_input(t_list*  lista_paquete ){

    t_io_input* io_unput = malloc(sizeof(t_io_input));

    io_unput->pid = *(uint32_t*)list_get(lista_paquete, 0);
    io_unput->direccion_fisica = *(uint32_t*)list_get(lista_paquete, 1);
    io_unput->input_length = *(uint32_t*)list_get(lista_paquete, 2);
    io_unput->input = list_get(lista_paquete, 3);    

    return io_unput;

}

// Kernel envía a io un stdin usando op_cod= IO_K_STDIN
void enviar_io_df(t_io_direcciones_fisicas* io_df, int socket, op_code codigo_operacion){

    t_paquete* paquete_espera = malloc(sizeof(t_paquete));    
   
    paquete_espera = crear_paquete(codigo_operacion); 
    
    agregar_a_paquete(paquete_espera, &io_df->pid, sizeof(io_df->pid)); 
    
    uint32_t list_tamanio = list_size(io_df->direcciones_fisicas);
    
    agregar_a_paquete(paquete_espera, &list_tamanio, sizeof(uint32_t));

    for (int i = 0; i < list_tamanio; i++) {
        uint32_t* direccion_fisica = (uint32_t*) list_get(io_df->direcciones_fisicas, i);
        agregar_a_paquete(paquete_espera, direccion_fisica, sizeof(uint32_t));
    }   
       
    enviar_paquete(paquete_espera, socket);  
  printf("Kernel server sent IO_K_STDIN to client\n");

}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	  log_destroy(logger);
	  config_destroy(config);
	  liberar_conexion(conexion);

}


