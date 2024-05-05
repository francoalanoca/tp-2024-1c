#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"


char *path_config;

t_proceso* proceso_actual;

int tamanioParams;
int tamanioInterfaces;


int main(int argc, char* argv[]) {
    decir_hola("CPU");

    path_config = argv[1];

	int socket_memoria;
	char* ip;



     printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;

    }
    
	log_info(logger_cpu, "empieza el programa");
    crear_servidor_dispatch();
    log_info(logger_cpu, "se creo el servidor");
   
     socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);
       
     if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con kernel");
    }
    else {
        log_info(logger_cpu, "Error en handshake con kernel");
        return EXIT_FAILURE;
    }

     
   ciclo_de_instrucciones(socket_memoria,logger_cpu,cfg_cpu,proceso_actual); //TODO: Crear esta funcion, para el fetch mandar mensaje a memoria usando PROXIMA_INSTRUCCION


	


	terminar_programa(socket_memoria, logger_cpu, cfg_cpu);
    return 0;
}

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_proceso* proceso){
    instr_t *inst = malloc(sizeof(instr_t));
    inst = fetch(conexion,logger,config,proceso);
    tipo_instruccion tipo_inst; //TODO: ver como hacer malloc
    tipo_inst = decode(inst);//TODO: ver como funciona
    execute(logger, config,inst,tipo_inst, proceso);
    check_interrupt();
    proceso_actual->pcb->program_counter += 1;

}

instr_t* fetch(int conexion, t_log* logger, t_config* config, t_proceso* proceso){
        instr_t *prox_inst = malloc(sizeof(instr_t));
       prox_inst = pedir_instruccion(proceso, conexion);
       return prox_inst;
}

tipo_instruccion decode(instr_t* instr){
    return instr->id;//TODO: VER IMPLEMENTACION
   // return SET;
}


void execute(t_log* logger, t_config* config, instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso){

    switch(tipo_inst){
        case SET:
        {
            set(inst->param1, inst->param2, proceso);
            break;
        }
        case SUM:
        {
            sum(inst->param1, inst->param2,proceso);
            break;
        }
        case SUB:
        {
            sub(inst->param1, inst->param2,proceso);
            break;
        }
        case JNZ:
        {
            jnz(inst->param1, inst->param2,proceso);
            break;
        }
        case IO_GEN_SLEEP:
        {
            io_gen_sleep(inst->param1, inst->param2,proceso);
            break;
        }
    }

}

void check_interrupt(){
    if(verificar_interrupcion_kernel()){
        generar_interrupcion_a_kernel(proceso_actual);//TODO: en esta funcion no se usara dispatch sino interrupt
    }
}

instr_t* pedir_instruccion(t_proceso* proceso,int conexion){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete -> codigo_operacion = PROXIMA_INSTRUCCION;
    paquete->buffer = proceso_serializar(proceso);

    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));

    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(conexion, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0); //VER que socket poner(reemplazar unSocket)

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    
    //return pedir_inst_a_memoria(proceso->pcb->program_counter, PROXIMA_INSTRUCCION);
}

void set(uint32_t registro, uint32_t valor, t_proceso* proceso){
    registro = valor;
}

void sum(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso){
    registro_destino = registro_destino + registro_origen;
}

void sub(uint32_t registro_destino, uint32_t registro_origen, t_proceso* proceso){
    registro_destino = registro_destino - registro_origen;
}

void jnz(uint32_t registro, uint32_t inst, t_proceso* proceso){
    if(registro != 0){
        proceso_actual-> pcb-> program_counter = inst;
    }
}
//void io_gen_sleep(Interfaz interfaz, int unidades_de_trabajo){ //TODO: VER PARAMETROS
void io_gen_sleep(char* interfaz, int unidades_de_trabajo, t_proceso* proceso){
   // t_interfaz interfaz_elegida = malloc(sizeof(t_interfaz));//REVISAR
   t_interfaz interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    //enviar_interfaz_a_kernel(interfaz_elegida, unidades_de_trabajo);//VER IMPLEMENTACION
    printf("Entra a io_gen_sleep");
}

instr_t* pedir_inst_a_memoria(int pc, int valor){//TODO:DEFINIR
    instr_t *prox_inst = malloc(sizeof(instr_t));
    return prox_inst;
}


bool verificar_interrupcion_kernel(){
    return false;
}

void generar_interrupcion_a_kernel(t_proceso* proceso_actual){
    printf("entro a generar_interrupcion_a_kernel");
}

t_interfaz elegir_interfaz(char* interfaz,t_proceso* proceso){
    t_interfaz interf;
    return interf;

}

// Crea un buffer vacío de tamaño size y offset 0
t_buffer *buffer_create(uint32_t size){
t_buffer* buffer = malloc(sizeof(t_buffer));
buffer->size = size;
buffer->offset = 0;
return buffer;
}

// Libera la memoria asociada al buffer
void buffer_destroy(t_buffer *buffer){
	free(buffer->stream);
	free(buffer);
}

// Agrega un stream al buffer en la posición actual y avanza el offset
void buffer_add(t_buffer *buffer, void *data, uint32_t size){
	//VER ESTA CON CODIGO ANTERIOR
	memcpy(buffer->stream + buffer->offset, &data, size);
	buffer->offset += size;
}

void buffer_add_uint32(t_buffer *buffer, uint32_t data){
	buffer_add(buffer,data,sizeof(uint32_t));
}


// Agrega un uint8_t al buffer
void buffer_add_uint8(t_buffer *buffer, uint8_t data){
	buffer_add(buffer,data,sizeof(uint8_t));
}


// Agrega string al buffer con un uint32_t adelante indicando su longitud
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string){
	buffer_add(buffer,length,sizeof(uint32_t));
	buffer_add(buffer,string,length); //VER SI MULTIPLICAR LENGTH POR TAMANIO DE CHAR*
}


void buffer_add_pcb(t_buffer* buffer,  t_pcb* pcb){
	buffer_add(buffer, &pcb->pid, sizeof(int));
	buffer_add(buffer, &pcb->program_counter, sizeof(int));
	buffer_add(buffer, &pcb->quantum, sizeof(int));
	buffer_add(buffer, &pcb->registrosCPU, sizeof(t_registros_CPU));
}

void buffer_add_instruccion(t_buffer* buffer, instr_t* instruccion){
	buffer_add(buffer, &instruccion->idLength, sizeof(uint8_t));
	buffer_add(buffer, &instruccion->id ,instruccion->idLength);
	buffer_add(buffer, &instruccion->param1Length ,sizeof(uint8_t));
	buffer_add(buffer, &instruccion->param1 ,instruccion->param1Length);
	buffer_add(buffer, &instruccion->param2Length ,sizeof(uint8_t));
	buffer_add(buffer, &instruccion->param2 ,instruccion->param2Length);
	buffer_add(buffer, &instruccion->param3Length ,sizeof(uint8_t));
	buffer_add(buffer, &instruccion->param3 ,instruccion->param3Length);
	buffer_add(buffer, &instruccion->param4Length ,sizeof(uint8_t));
	buffer_add(buffer, &instruccion->param4 ,instruccion->param4Length);
	buffer_add(buffer, &instruccion->param5Length ,sizeof(uint8_t));
	buffer_add(buffer, &instruccion->param5 ,instruccion->param5Length);
}

void calcularTamanioInstruccion(instr_t* instruccion){
	int tamanio = malloc(sizeof(int)); //VER DONDE HACER FREE
	tamanio = instruccion->idLength + instruccion->param1Length + instruccion->param2Length + instruccion->param3Length + instruccion->param4Length + instruccion->param5Length;
	tamanioParams += tamanio;
	free(tamanio);
}

void calcularTamanioInterfaz(t_interfaz* interfaz){
	int tamanio = malloc(sizeof(int)); //VER DONDE HACER FREE
	tamanio = sizeof(interfaz->nombre) + sizeof(interfaz->tipo); //REVISAR
	tamanioInterfaces += tamanio;
	free(tamanio);
}

void buffer_add_interfaz(t_buffer* buffer, t_interfaz* interfaz){
	buffer_add(buffer, &interfaz->nombre,((strlen(interfaz->nombre) + 1) * sizeof(char*))); 
	buffer_add(buffer, &interfaz->tipo,((strlen(interfaz->tipo) + 1) * sizeof(char*))); //VER SI ESTA BIEN EL STRLEN
}

t_buffer *proceso_serializar(t_proceso* proceso) {
	
//t_buffer* buffer = malloc(sizeof(t_buffer)); //VER SI HACE FALTA
//t_proceso* proceso = malloc(sizeof(proceso);

int tamanioParams = malloc(sizeof(int));//hay que recorrer con for la lista de instrucciones y por cada una ir sumando en esta variable el tameanio de los parametros

int tamanioInterfaces = malloc(sizeof(int)); //hay que recorrer con for la lista de interfaces y por cada una ir sumando en esta variable el tameanio del nombre y tipo

list_iterate(proceso->instrucciones, calcularTamanioInstruccion);

list_iterate(proceso->interfaces, calcularTamanioInterfaz);

int tamanio_pcb = malloc(sizeof(int));
tamanio_pcb = sizeof(uint32_t) * 3 + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 4;

int tamanioInstrucciones = malloc(sizeof(int));
tamanioInstrucciones = ((sizeof(uint8_t) * 6 + sizeof(tipo_instruccion)) * proceso->cantidad_instrucciones )+ tamanioParams;

int tamanioBuffer = malloc(sizeof(int));
tamanioBuffer = tamanio_pcb
             + sizeof(uint8_t) // cantidad_instrucciones
             + tamanioInstrucciones
			 + tamanioInterfaces;
			 
  t_buffer *buffer = buffer_create(tamanioBuffer);
//REVISAR ACA (SERIALIZACION)
    buffer_add_pcb(buffer, proceso->pcb);
    buffer_add_uint8(buffer, proceso->cantidad_instrucciones);
	//list_iterate(proceso->instrucciones, buffer_add_instruccion(buffer));//NO SE PUEDE HACER YA QUE LA FUNCION RECIBE UN PARAM BUFFER
	//list_iterate(proceso->interfaces, buffer_add_interfaz(buffer));//NO SE PUEDE HACER YA QUE LA FUNCION RECIBE UN PARAM BUFFER
	//REVISAR ACAAAAA//VER DE HACER FOR
	
	/////////
	  for(int i = 0; i < proceso->cantidad_instrucciones; i++){	
			buffer_add_instruccion(buffer, list_get(proceso->instrucciones,i));
	  }
	       
	
	///////
   //  //RECORRER LISTA DE INSTRUCCIONES CON FOR Y HACER ESTO POR CADA UNA
   	  for(int i = 0; i < list_size(proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso->interfaces,i));
	  }
    //buffer_add_interfaz(buffer, persona->interfaces); //RECORRER LISTA DE INTERFACES CON FOR Y HACER ESTO POR CADA UNA

    return buffer;
}




