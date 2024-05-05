#include"../include/servidorCpu.h"
char* puerto_dispatch;
int fd_mod2 = -1;
//pcb *pcb_actual;

void* crear_servidor_dispatch(){
    log_info(logger_cpu, "empieza crear_servidor_dispatch");

    log_info(logger_cpu, "valor de PUERTO_ESCUCHA_DISPATCH: %s", cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
    

    puerto_dispatch = malloc((strlen(cfg_cpu->PUERTO_ESCUCHA_DISPATCH) + 1) * sizeof(char));
if (puerto_dispatch != NULL) {
    strcpy(puerto_dispatch, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
}
else{
    log_info(logger_cpu,"error al asignar memoria a variable del puerto");
}
    //strcpy(puerto_dispatch, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
    log_info(logger_cpu, "crea puerto_dispatch");
    printf("El puerto_dispatch es: %s", puerto_dispatch);
    fd_mod2 = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", "127.0.0.1",  puerto_dispatch);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod2 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
log_info(logger_cpu, "va a escuchar");
    while (server_escuchar(logger_cpu, "SERVER CPU", (uint32_t)fd_mod2));
}

int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    log_info(logger_cpu, "entra a server escuchar");
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    log_info(logger_cpu, "sale de esperar_cliente");

    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
         t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        //pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,cliente_socket);//TODO:Redefinir procesar_conexion para que reciba un PCB
        pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

/*
void procesar_conexion(int cliente_socket){
    printf("El socket del cliente es: %d", cliente_socket);
}*/

void procesar_conexion(void *v_args){
     t_procesar_conexion_args *args = (t_procesar_conexion_args *) v_args;
    t_log *logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

     op_code cop;

     ///
   
    t_paquete* paquete = malloc(sizeof(t_paquete));
    cop = recv(cliente_socket, &(paquete->codigo_operacion), sizeof(op_code), 0);
    paquete->buffer = malloc(sizeof(t_buffer));
    recv(cliente_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(cliente_socket, paquete->buffer->stream, paquete->buffer->size, 0); //agregar &?
     ///

    while (cliente_socket != -1) {

       // if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
        if (cop != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

    //switch (cop){
        switch (cop){
            case NUEVO_PROCESO:
            {
                t_proceso* proceso = malloc(sizeof(t_proceso)); //REVISAR
                log_info(logger_cpu, "PROCESO RECIBIDO");
                proceso = proceso_deserializar(paquete->buffer); //DEFINIR FUNCION PARA DESERIALIZAR
                proceso_actual = proceso; //Agregar a lista de procesos?
                free(proceso);
                break;
            }
           
    }   
free(paquete->buffer->stream);
free(paquete->buffer);
free(paquete);
}
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;

    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

void buffer_read(t_buffer *buffer, void *data, uint32_t size){
	//VER ESTA CAUNDO HAY HECHO LA PARE DE DESEREALIZACION
	void* stream = buffer->stream;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&data, stream, size);
    stream += size;
}

uint8_t buffer_read_uint8(t_buffer *buffer){
	uint8_t valor = malloc(sizeof(uint8_t));
	buffer_read(buffer,&valor,sizeof(uint8_t));
	return valor;
}

t_pcb *buffer_read_pcb(t_buffer *buffer, uint32_t *length){
	t_pcb * valor = malloc(length);
	buffer_read(buffer,&valor,length);//REVISAR
	return valor;
}

instr_t *buffer_read_instruccion(t_buffer *buffer, uint32_t *length){
	instr_t * valor = malloc(length);
	buffer_read(buffer,&valor,length);//REVISAR
	return valor;
}

t_interfaz *buffer_read_interfaz(t_buffer *buffer, uint32_t *length){
	t_interfaz * valor = malloc(length);
	buffer_read(buffer,&valor,length);//REVISAR
	return valor;
}

t_proceso *proceso_deserializar(t_buffer *buffer) {
    t_proceso *proceso = malloc(sizeof(t_proceso));
	
	int tamanio_pcb = malloc(sizeof(int));
tamanio_pcb = sizeof(uint32_t) * 3 + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 4;
//REVISAR ACA (DESERIALIZACION)
    proceso->pcb = buffer_read_pcb(buffer, tamanio_pcb);
    proceso->cantidad_instrucciones = buffer_read_uint8(buffer);
   // proceso->instrucciones = buffer_read_instruccion(buffer);
   	  for(int i = 0; i < proceso->cantidad_instrucciones; i++){	
			buffer_read_instruccion(buffer,sizeof(list_get(proceso->instrucciones,i)));
	  }

   	  for(int i = 0; i < list_size(proceso->interfaces); i++){	
			buffer_read_interfaz(buffer,sizeof(list_get(proceso->interfaces,i)));
	  }
	  
	  free(tamanio_pcb);

    return proceso;
}

