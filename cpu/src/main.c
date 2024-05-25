#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"


char *path_config;
char *ip_cpu;

t_proceso* proceso_actual;
t_proceso_interrumpido* proceso_interrumpido_actual;
bool interrupcion_kernel;
instr_t *prox_inst;

int tamanioParams;
int tamanioInterfaces;
int conexion_kernel;


int main(int argc, char* argv[]) {
    decir_hola("CPU");

    path_config = argv[1];
    ip_cpu = argv[2];

	int socket_memoria;
	//char* ip;



     printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;

    }
    
    
	log_info(logger_cpu, "empieza el programa");
    //
    pthread_t servidor_dispatch;
    pthread_create(&servidor_dispatch, NULL,crear_servidor_dispatch,ip_cpu);
    pthread_detach(servidor_dispatch);

    pthread_t servidor_interrupt;
    pthread_create(&servidor_interrupt, NULL,crear_servidor_interrupt,ip_cpu);
    pthread_detach(servidor_interrupt);
    log_info(logger_cpu, "cree los hilos servidor");
    //crear_servidor_dispatch(ip_cpu);
    //crear_servidor_interrupt(ip_cpu);
    socket_memoria = crear_conexion(logger_cpu, "PRUEBA", cfg_cpu->IP_MEMORIA, "8003");
    log_info(logger_cpu, "creo socket");
    if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con memoria");
    }
    else {
        log_info(logger_cpu, "Error en handshake con memoria");
        return EXIT_FAILURE;
    }
    proceso_actual = malloc(sizeof(t_proceso));
    proceso_actual->interfaces = malloc(sizeof(t_interfaz));
    proceso_actual->pcb = malloc(sizeof(t_pcb));
    proceso_actual->pcb->registrosCPU = malloc(sizeof(t_registros_CPU));
    t_registros_CPU* registros_prueba = malloc((sizeof(uint32_t) * 7) + (sizeof(uint8_t) * 4));
    registros_prueba->PC = 1;
    registros_prueba->AX = 2;
    registros_prueba->BX = 3;
    registros_prueba->CX = 4;
    registros_prueba->DX = 5;
    registros_prueba->EAX = 6;
    registros_prueba->EBX = 7;
    registros_prueba->ECX = 8;
    registros_prueba->EDX = 9;
    registros_prueba->SI = 10;
    registros_prueba->DI = 11;

    proceso_actual->pcb->pid=1;
    proceso_actual->pcb->program_counter=2;
    proceso_actual->pcb->quantum=3;
    proceso_actual->pcb->registrosCPU = registros_prueba;
    proceso_actual->interfaces = list_create();
    t_interfaz* int_prueba = malloc(sizeof(t_interfaz));
    int_prueba->nombre="Int1";
    int_prueba->tipo = GENERICA;
    list_add(proceso_actual->interfaces, int_prueba);
    //
   proceso_interrumpido_actual = malloc(sizeof(t_proceso_interrumpido));
    proceso_interrumpido_actual->proceso = malloc(sizeof(t_proceso));
    proceso_interrumpido_actual->proceso->interfaces = malloc(sizeof(t_interfaz));
    proceso_interrumpido_actual->proceso->pcb = malloc(sizeof(t_pcb));
    proceso_interrumpido_actual->proceso->pcb->registrosCPU = malloc(sizeof(t_registros_CPU));
    proceso_interrumpido_actual->proceso->interfaces = list_create();
    proceso_interrumpido_actual->tamanio_motivo_interrupcion = 6;
     proceso_interrumpido_actual->motivo_interrupcion = malloc(proceso_interrumpido_actual->tamanio_motivo_interrupcion );
    strcpy(proceso_interrumpido_actual->motivo_interrupcion, "Motivo");
    list_add(proceso_interrumpido_actual->proceso->interfaces, int_prueba);
     
    //
   // interrupcion_kernel = malloc(sizeof(bool));*/
    interrupcion_kernel = true;
    
    prox_inst = malloc(sizeof(instr_t));
    //prox_inst->
    conexion_kernel =  socket_memoria;
    //
    ////////////////////////////////////////////////////////////
    /*log_info(logger_cpu, "se creo el servidor");
   
     socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);
      
     if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con memoria");
    }
    else {
        log_info(logger_cpu, "Error en handshake con memoria");
        return EXIT_FAILURE;
    }*/
////////////////////////////////////////////////////////////
    //TODO: HACER HANDSHAKE CON KERNEL Y ENVIAR SOCKET A CICLO_DE INSTRUCCIONES PARA USAR EN EL CHECK_INETRRUPT

   //  printf("AX actual: %d", proceso_actual->pcb->registrosCPU->AX);
   ciclo_de_instrucciones(socket_memoria,logger_cpu,cfg_cpu,proceso_actual); //TODO: Crear esta funcion, para el fetch mandar mensaje a memoria usando PROXIMA_INSTRUCCION


	


	//terminar_programa(socket_memoria, logger_cpu, cfg_cpu);
    return 0;
}

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_proceso* proceso){
    log_info(logger, "Entro al ciclo");
    instr_t *inst = malloc(sizeof(instr_t));
    log_info(logger, "Voy a entrar a fetch");
    inst = fetch(conexion,logger,config,proceso);
    //log_info(logger, "Instrucc nueva: %d, %s, %s",inst->id, inst->param1, inst->param2);
    tipo_instruccion tipo_inst; //TODO: ver como hacer malloc
    log_info(logger, "Voy a entrar a decode");
    tipo_inst = decode(inst);//TODO: ver como funciona
    log_info(logger, "Voy a entrar a execute");//, tipo_inst: %d", tipo_inst);
    execute(logger, config,inst,tipo_inst, proceso);
    log_info(logger, "Voy a entrar a check_interrupt");//, AX: %d", proceso->pcb->registrosCPU->AX);
    //log_info(logger, "interrupcion_kernel: %s", interrupcion_kernel ? "true" : "false");
    check_interrupt();
    log_info(logger, "Sale de check_interrupt");
    proceso_actual->pcb->program_counter += 1;
    log_info(logger, "Termino ciclo de instrucciones");

}

instr_t* fetch(int conexion, t_log* logger, t_config* config, t_proceso* proceso){
       // instr_t *prox_inst = malloc(sizeof(instr_t));
       log_info(logger, "Voy a entrar a pedir_instruccion");
       pedir_instruccion(proceso, conexion,logger); //TODO:VER COMO RECIBIR LA INSTRUCCION
       //SEMAFORO QUE ESPERE A RECIBIR LA PROX INSTRUCCION
       //PARA PROBAR:
       //INSTRUCCION DE PRUEBA 1 (SET)
       /*prox_inst->idLength = 4;
                prox_inst->id = SET;
                
                prox_inst->param1Length = (strlen("AX") + 1) * sizeof(char*);
                prox_inst->param1 = malloc(prox_inst->param1Length);
                strcpy(prox_inst->param1, "AX");
                
                prox_inst->param2Length = (strlen("1")+1) * sizeof(char*);
                prox_inst->param2 = malloc(prox_inst->param2Length);
                strcpy(prox_inst->param2, "1");*/
                //INSTRUCCION DE PRUEBA 2 (IO_GEN_SLEEP)
              prox_inst->idLength = 4;
                prox_inst->id = IO_GEN_SLEEP;  
                prox_inst->param1Length = (strlen("Int1") + 1) * sizeof(char*);
                prox_inst->param1 = malloc(prox_inst->param1Length);
                strcpy(prox_inst->param1, "Int1");
                
                prox_inst->param2Length = (strlen("10")+1) * sizeof(char*);
                prox_inst->param2 = malloc(prox_inst->param2Length);
                strcpy(prox_inst->param2, "10");
        
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
            char *endptr;
            uint32_t param2_num = (uint32_t)strtoul(inst->param2, &endptr, 10);// Convertir la cadena a uint32_t
            set(inst->param1, param2_num, proceso, logger);
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
            char *endptr;
            uint32_t param2_num = (uint32_t)strtoul(inst->param2, &endptr, 10);// Convertir la cadena a uint32_t
            
            io_gen_sleep(inst->param1, param2_num,proceso);
            break;
        }
    }

}

void check_interrupt(){
    printf("Entro checkinterrupt\n");
    if(interrupcion_kernel){//en esta funcion no se usara dispatch sino interrupt
         printf("Entro if checkinterrupt\n");
        generar_interrupcion_a_kernel(conexion_kernel); //TODO:VER COMO MANDAR CONEXION A KERNEL
    }
}

void pedir_instruccion(t_proceso* proceso,int conexion, t_log* logger){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    //paquete->buffer= malloc(sizeof(t_buffer));
    log_info(logger, "Voy a entrar a crear_proceso_memoria");
    //t_proceso_memoria* proceso_memoria = malloc(sizeof(t_proceso_memoria));
    t_proceso_memoria* proceso_memoria = crear_proceso_memoria(proceso);
     log_info(logger, "Voy a entrar a proceso_memoria_serializar");
    paquete -> codigo_operacion = PROXIMA_INSTRUCCION;
    paquete->buffer = proceso_memoria_serializar(proceso_memoria,logger);
     log_info(logger, "codOpe: %d,  size buffer:%d, size int: %d, %d", paquete->codigo_operacion,paquete->buffer->size,sizeof(uint32_t),sizeof(op_code));

    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    

// Por último enviamos
    send(conexion, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0); //VER que socket poner(reemplazar unSocket)
 log_info(logger, "Envio de mensaje\n");
// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(proceso_memoria);

    
    //return pedir_inst_a_memoria(proceso->pcb->program_counter, PROXIMA_INSTRUCCION);
}

void set(char* registro, uint32_t valor, t_proceso* proceso, t_log *logger){
    printf("El valor del set es : %d ", valor);
    registros registro_elegido = identificarRegistro(registro);
    switch(registro_elegido){
        case PC:
        {
           proceso->pcb->registrosCPU->PC = valor;
            break;
        }
        case AX:
        {
           proceso->pcb->registrosCPU->AX = valor;
            break;
        }
        case BX:
        {
           proceso->pcb->registrosCPU->BX = valor;
            break;
        }
        case CX:
        {
           proceso->pcb->registrosCPU->CX = valor;
            break;
        }
        case DX:
        {
           proceso->pcb->registrosCPU->DX = valor;
            break;
        }
        case EAX:
        {
           proceso->pcb->registrosCPU->EAX = valor;
            break;
        }
        case EBX:
        {
           proceso->pcb->registrosCPU->EBX = valor;
            break;
        }
        case ECX:
        {
           proceso->pcb->registrosCPU->ECX = valor;
            break;
        }
        case EDX:
        {
           proceso->pcb->registrosCPU->EDX = valor;
            break;
        }
        case SI:
        {
           proceso->pcb->registrosCPU->SI = valor;
            break;
        }
        case DI:
        {
           proceso->pcb->registrosCPU->DI = valor;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }

    //proceso->pcb->registrosCPU.AX;
   // registro = valor;
}

void sum(char* registro_destino, char* registro_origen, t_proceso* proceso){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso);

    switch(id_registro_destino){
        case PC:
        {
           proceso->pcb->registrosCPU->PC = valor_reg_destino + valor_reg_origen;
            break;
        }
        case AX:
        {
           proceso->pcb->registrosCPU->AX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case BX:
        {
           proceso->pcb->registrosCPU->BX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case CX:
        {
           proceso->pcb->registrosCPU->CX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case DX:
        {
           proceso->pcb->registrosCPU->DX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EAX:
        {
           proceso->pcb->registrosCPU->EAX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EBX:
        {
           proceso->pcb->registrosCPU->EBX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case ECX:
        {
           proceso->pcb->registrosCPU->ECX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EDX:
        {
           proceso->pcb->registrosCPU->EDX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case SI:
        {
           proceso->pcb->registrosCPU->SI = valor_reg_destino + valor_reg_origen;
            break;
        }
        case DI:
        {
           proceso->pcb->registrosCPU->DI = valor_reg_destino + valor_reg_origen;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }


    //registro_destino = registro_destino + registro_origen;
}

void sub(char* registro_destino, char* registro_origen, t_proceso* proceso){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso);

    switch(id_registro_destino){
        case PC:
        {
           proceso->pcb->registrosCPU->PC = valor_reg_destino - valor_reg_origen;
            break;
        }
        case AX:
        {
           proceso->pcb->registrosCPU->AX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case BX:
        {
           proceso->pcb->registrosCPU->BX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case CX:
        {
           proceso->pcb->registrosCPU->CX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case DX:
        {
           proceso->pcb->registrosCPU->DX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EAX:
        {
           proceso->pcb->registrosCPU->EAX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EBX:
        {
           proceso->pcb->registrosCPU->EBX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case ECX:
        {
           proceso->pcb->registrosCPU->ECX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EDX:
        {
           proceso->pcb->registrosCPU->EDX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case SI:
        {
           proceso->pcb->registrosCPU->SI = valor_reg_destino - valor_reg_origen;
            break;
        }
        case DI:
        {
           proceso->pcb->registrosCPU->DI = valor_reg_destino - valor_reg_origen;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }
    //registro_destino = registro_destino - registro_origen;
}

void jnz(char* registro, uint32_t inst, t_proceso* proceso){
    registros id_registro = identificarRegistro(registro);
    uint32_t valor_registro = obtenerValorActualRegistro(id_registro,proceso);
    if(valor_registro != 0){
        proceso->pcb->program_counter = inst;
    }
}
//void io_gen_sleep(Interfaz interfaz, int unidades_de_trabajo){ //TODO: VER PARAMETROS
void io_gen_sleep(char* interfaz, uint32_t unidades_de_trabajo, t_proceso* proceso){
   // t_interfaz interfaz_elegida = malloc(sizeof(t_interfaz));//REVISAR
   t_interfaz* interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    if(interfaz_elegida != NULL){
enviar_interfaz_a_kernel(interfaz_elegida, unidades_de_trabajo);//VER IMPLEMENTACION
    }
    
    printf("Entra a io_gen_sleep");
}

instr_t* pedir_inst_a_memoria(int pc, int valor){//TODO:DEFINIR
    instr_t *prox_inst = malloc(sizeof(instr_t));
    return prox_inst;
}


/*bool verificar_interrupcion_kernel(){
    return false;
}*/

void generar_interrupcion_a_kernel(int conexion){
    printf("entro a generar_interrupcion_a_kernel\n");
    t_paquete* paquete = malloc(sizeof(t_paquete));
    //t_proceso_memoria* proceso_memoria = malloc(sizeof(t_proceso_memoria));
    //t_proceso_interrumpido* proceso_interrumpido = crear_proceso_interrumpido(proceso_actual, "Motivo de interrupcion");//ESTO ES SI EL KENERL ME MANDA PROCESO
    
    paquete -> codigo_operacion = INTERRUPCION_CPU;
    printf("Voy a proceso_interrumpido_serializar\n");
    paquete->buffer = proceso_interrumpido_serializar(proceso_interrumpido_actual);
printf("Salgo de proceso_interrumpido_serializar\n");
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(conexion, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0); 

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    //free(proceso_interrumpido);
}


// Crea un buffer vacío de tamaño size y offset 0
t_buffer *buffer_create(uint32_t size){
   t_buffer *buffer = (t_buffer *)malloc(sizeof(t_buffer));
    if (buffer == NULL) {
        printf("Error al asignar memoria para el buffer.\n");
        return NULL;
    }
    buffer->stream = (char *)malloc(size);
    if (buffer->stream == NULL) {
        printf("Error al asignar memoria para el buffer stream.\n");
        free(buffer);
        return NULL;
    }
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
printf("Entre buffer_add\n");

    // Verificar que haya suficiente espacio en el buffer
    if (buffer->offset + size > buffer->size) {
        printf("Error: no hay suficiente espacio en el buffer.\n");
        printf("buffer->offset + size: %d , buffer->size: %d.\n",buffer->offset + size,buffer->size) ;
        return;
    }

    // Copiar los datos en el buffer
    memcpy(buffer->stream + buffer->offset, data, size);

    printf("Sale memcpy\n");
    buffer->offset += size;
}

void buffer_add_uint32(t_buffer *buffer, uint32_t data){
    printf("Voy a  entrar buffer_add\n");
	buffer_add(buffer,&data,sizeof(uint32_t));
     printf("sali de  buffer_add\n");
}


// Agrega un uint8_t al buffer
void buffer_add_uint8(t_buffer *buffer, uint8_t data){
	buffer_add(buffer,&data,sizeof(uint8_t));
}


// Agrega string al buffer con un uint32_t adelante indicando su longitud
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string){
	//buffer_add(buffer,&length,sizeof(uint32_t));
    printf("hago buffer_add de l length\n");	
	buffer_add(buffer,string,length); //VER SI MULTIPLICAR LENGTH POR TAMANIO DE CHAR*
     printf("hago buffer_add de l string\n");
}

void buffer_add_t_tipo_interfaz_enum(t_buffer *buffer, t_tipo_interfaz_enum *data){
    printf("entro a buffer_add_t_tipo_interfaz_enum, data:%d, size:%d\n", data,sizeof(t_tipo_interfaz_enum) );
    buffer_add(buffer,data,sizeof(t_tipo_interfaz_enum));
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
    printf("Emplieza ccalcularTamanioInterfaz\n");
	uint32_t tamanio;// = malloc(sizeof(uint32_t)); //VER DONDE HACER FREE
	tamanio = ((strlen(interfaz->nombre)+1) * sizeof(char*)) + sizeof(t_tipo_interfaz_enum); //REVISAR
    printf("Va a sumar tamanio\n");
	tamanioInterfaces += tamanio;
    printf("suma tamanio\n");
	//free(tamanio);
    printf("Termina ccalcularTamanioInterfaz\n");
}

void buffer_add_interfaz(t_buffer* buffer, t_interfaz* interfaz){
	buffer_add(buffer, &interfaz->nombre,((strlen(interfaz->nombre) + 1) * sizeof(char*))); 
	buffer_add(buffer, &interfaz->tipo,((sizeof(interfaz->tipo)) * sizeof(char*))); //VER SI ESTA BIEN EL STRLEN
}

t_buffer *proceso_serializar(t_proceso* proceso) {
	
//t_buffer* buffer = malloc(sizeof(t_buffer)); //VER SI HACE FALTA
//t_proceso* proceso = malloc(sizeof(proceso);

int tamanioParams = malloc(sizeof(int));//hay que recorrer con for la lista de instrucciones y por cada una ir sumando en esta variable el tameanio de los parametros

int tamanioInterfaces = malloc(sizeof(int)); //hay que recorrer con for la lista de interfaces y por cada una ir sumando en esta variable el tameanio del nombre y tipo

//list_iterate(proceso->instrucciones, calcularTamanioInstruccion);

list_iterate(proceso->interfaces, calcularTamanioInterfaz);

int tamanio_pcb = malloc(sizeof(int));
tamanio_pcb = sizeof(uint32_t) * 3 + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 4;

//int tamanioInstrucciones = malloc(sizeof(int));
//tamanioInstrucciones = ((sizeof(uint8_t) * 6 + sizeof(tipo_instruccion)) * proceso->cantidad_instrucciones )+ tamanioParams;

int tamanioBuffer = malloc(sizeof(int));
tamanioBuffer = tamanio_pcb
          //   + sizeof(uint8_t) // cantidad_instrucciones
           //  + tamanioInstrucciones
			 + tamanioInterfaces;
			 
  t_buffer *buffer = buffer_create(tamanioBuffer);
//REVISAR ACA (SERIALIZACION)
    buffer_add_pcb(buffer, proceso->pcb);
    //buffer_add_uint8(buffer, proceso->cantidad_instrucciones);
	//list_iterate(proceso->instrucciones, buffer_add_instruccion(buffer));//NO SE PUEDE HACER YA QUE LA FUNCION RECIBE UN PARAM BUFFER
	//list_iterate(proceso->interfaces, buffer_add_interfaz(buffer));//NO SE PUEDE HACER YA QUE LA FUNCION RECIBE UN PARAM BUFFER
	//REVISAR ACAAAAA//VER DE HACER FOR
	
	/////////
	/*  for(int i = 0; i < proceso->cantidad_instrucciones; i++){	
			buffer_add_instruccion(buffer, list_get(proceso->instrucciones,i));
	  }*/
	       
	
	///////
   //  
   	  for(int i = 0; i < list_size(proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso->interfaces,i));
	  }
    //buffer_add_interfaz(buffer, persona->interfaces); //RECORRER LISTA DE INTERFACES CON FOR Y HACER ESTO POR CADA UNA

    return buffer;
}

t_proceso_memoria* crear_proceso_memoria(t_proceso* proceso){
    t_proceso_memoria* nuevo_proceso = malloc(sizeof(t_proceso_memoria));
    nuevo_proceso->pid = proceso->pcb->pid;
    nuevo_proceso->program_counter = proceso->pcb->program_counter;
    printf("Nuevo proceso memora: pid: %d,program counter: %d", nuevo_proceso->pid, nuevo_proceso->program_counter);
    return nuevo_proceso;
}

t_buffer *proceso_memoria_serializar(t_proceso_memoria* proceso_memoria, t_log* logger) {
log_info(logger, "Entro proceso_memoria_serializar");
uint32_t tamanioBuffer = malloc(sizeof(uint32_t));
tamanioBuffer = sizeof(uint32_t) //pid
             + sizeof(uint32_t); // program_counter
     printf("aaaaaaaaaa");
     log_info(logger, "Va a entrar a buffer_create"); 

			 
  t_buffer *buffer = buffer_create(tamanioBuffer);
log_info(logger, "Va a agregar pid %d", proceso_memoria->pid);
    buffer_add_uint32(buffer, proceso_memoria->pid);
    log_info(logger, "Va a agregar programcounter");
    buffer_add_uint32(buffer, proceso_memoria->program_counter);
    log_info(logger, "sale proceso_memoria_serializar");

    return buffer;
}

t_proceso_interrumpido* crear_proceso_interrumpido(t_proceso* proceso, char* motivo){
    t_proceso_interrumpido* nuevo_proceso = malloc(sizeof(t_proceso_interrumpido));
    nuevo_proceso->proceso = proceso;
    strcpy(nuevo_proceso->motivo_interrupcion, motivo);
    return nuevo_proceso;
}

t_buffer *proceso_interrumpido_serializar(t_proceso_interrumpido* proceso_interrumpido) {
	

int tamanioParams = malloc(sizeof(int));//hay que recorrer con for la lista de instrucciones y por cada una ir sumando en esta variable el tameanio de los parametros
printf("malloc tamanioParams\n");
int tamanioInterfaces = malloc(sizeof(int)); //hay que recorrer con for la lista de interfaces y por cada una ir sumando en esta variable el tameanio del nombre y tipo
printf("malloc tamanioInterfaces\n");
//list_iterate(proceso_interrumpido->proceso->instrucciones, calcularTamanioInstruccion);
printf("tamListaInterfaces: %d\n", list_size(proceso_interrumpido->proceso->interfaces) );
//list_iterate(proceso_interrumpido->proceso->interfaces, calcularTamanioInterfaz);
   	  for(int i = 0; i < list_size(proceso_interrumpido->proceso->interfaces); i++){
        printf("entro for tamanioInterfaces\n");	
        printf("list_get name: %s\n",(t_interfaz*)list_get(proceso_interrumpido->proceso->interfaces,i) );
            calcularTamanioInterfaz((t_interfaz*)list_get(proceso_interrumpido->proceso->interfaces,i));
			//buffer_add_interfaz(buffer, list_get(proceso_interrumpido->proceso->interfaces,i));
	  }
printf("calcula tamanioInterfaces\n");
uint32_t tamanio_pcb = malloc(sizeof(uint32_t));
tamanio_pcb = sizeof(uint32_t) * 3 + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 4;
printf("calcula tamanio_pcb\n");
//int tamanioInstrucciones = malloc(sizeof(int));
//tamanioInstrucciones = ((sizeof(uint8_t) * 6 + sizeof(tipo_instruccion)) * proceso_interrumpido->proceso->cantidad_instrucciones )+ tamanioParams;

int tamanioBuffer = malloc(sizeof(int));
tamanioBuffer = tamanio_pcb
  //           + sizeof(uint8_t) // cantidad_instrucciones
  //           + tamanioInstrucciones
			 + tamanioInterfaces
             + (strlen(proceso_interrumpido->motivo_interrupcion) + 1);
	printf("calcula tamanioBuffer\n");		 
  t_buffer *buffer = buffer_create(tamanioBuffer);

    buffer_add_pcb(buffer, proceso_interrumpido->proceso->pcb);
    //buffer_add_uint8(buffer, proceso_interrumpido->proceso->cantidad_instrucciones);
printf("agrego pcb\n");	
	  /*for(int i = 0; i < proceso_interrumpido->proceso->cantidad_instrucciones; i++){	
			buffer_add_instruccion(buffer, list_get(proceso_interrumpido->proceso->instrucciones,i));
	  }*/
	       

   	  for(int i = 0; i < list_size(proceso_interrumpido->proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso_interrumpido->proceso->interfaces,i));
	  }
      printf("agrego interfaces\n");	

    return buffer;
}

registros identificarRegistro(char* registro){
    if(strcmp(registro,"PC") == 0){
        return PC;
    }
    else if(strcmp(registro,"AX") == 0){
        return AX;
    }
    else if(strcmp(registro,"BX") == 0){
        return BX;
    }
    else if(strcmp(registro,"CX") == 0){
        return CX;
    }
    else if(strcmp(registro,"DX") == 0){
        return DX;
    }
    else if(strcmp(registro,"EAX") == 0){
        return EAX;
    }
    else if(strcmp(registro,"EBX") == 0){
        return EBX;
    }
    else if(strcmp(registro,"ECX") == 0){
        return ECX;
    }
    else if(strcmp(registro,"EDX") == 0){
        return EDX;
    }
    else if(strcmp(registro,"SI") == 0){
        return SI;
    }
    else if(strcmp(registro,"DI") == 0){
        return DI;
    }
    else{
        return REG_NO_ENC;
    }
}

uint32_t obtenerValorActualRegistro(registros id_registro, t_proceso* proceso){
    switch(id_registro){
        case PC:
        {
           return proceso->pcb->registrosCPU->PC;
            break;
        }
        case AX:
        {
           return proceso->pcb->registrosCPU->AX;
            break;
        }
        case BX:
        {
           return proceso->pcb->registrosCPU->BX;
            break;
        }
        case CX:
        {
           return proceso->pcb->registrosCPU->CX;
            break;
        }
        case DX:
        {
           return proceso->pcb->registrosCPU->DX;
            break;
        }
        case EAX:
        {
           return proceso->pcb->registrosCPU->EAX;
            break;
        }
        case EBX:
        {
           return proceso->pcb->registrosCPU->EBX;
            break;
        }
        case ECX:
        {
           return proceso->pcb->registrosCPU->ECX;
            break;
        }
        case EDX:
        {
           return proceso->pcb->registrosCPU->EDX;
            break;
        }
        case SI:
        {
           return proceso->pcb->registrosCPU->SI;
            break;
        }
        case DI:
        {
           return proceso->pcb->registrosCPU->DI;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }
}

t_interfaz* elegir_interfaz(char* interfaz, t_proceso* proceso){
    t_interfaz* interfaz_actual; //malloc?
    for(int i = 0; i < proceso->interfaces->elements_count; i++){	 //REVISAR ELEMENTS_COUNT
			interfaz_actual = list_get(proceso->interfaces,i);
            if(strcmp(interfaz_actual->nombre, interfaz) == 0){
                return interfaz_actual;
            }
	  }
      return NULL;
}

void enviar_interfaz_a_kernel(t_interfaz* interfaz_elegida,uint32_t unidades_de_trabajo){
    printf("entro a enviar_interfaz_a_kernel\n");
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    //t_proceso_memoria* proceso_memoria = malloc(sizeof(t_proceso_memoria));
    //t_proceso_interrumpido* proceso_interrumpido = crear_proceso_interrumpido(proceso_actual, "Motivo de interrupcion");//ESTO ES SI EL KENERL ME MANDA PROCESO
    
    paquete -> codigo_operacion = ENVIO_INTERFAZ;
    paquete->buffer = envio_interfaz_serializar(interfaz_elegida, unidades_de_trabajo);
    printf("sali de envio_interfaz_serializar\n");
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(conexion_kernel, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0); //VER que socket poner(reemplazar unSocket)

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    //free(proceso_interrumpido);
}

t_buffer* envio_interfaz_serializar(t_interfaz* interfaz_elegida, uint32_t unidades_de_trabajo){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
tamanioBuffer = ((strlen(interfaz_elegida->nombre) + 1)* sizeof(char*)) //TAMANIO DEL NOMBRE DE LA INTERFAZ
             + sizeof(t_tipo_interfaz_enum)
             + sizeof(uint32_t); // unidades_trabajo
printf("tamanioBuffer calculado: %d + %d + %d \n",((strlen(interfaz_elegida->nombre) + 1)* sizeof(char*)) , sizeof(t_tipo_interfaz_enum),sizeof(uint8_t));			 
  t_buffer *buffer = buffer_create(tamanioBuffer);
//REVISAR ACA (SERIALIZACION)
printf("empiezo con los buffer_add\n");	
    buffer_add_string(buffer,((strlen(interfaz_elegida->nombre) + 1)* sizeof(char*)) ,interfaz_elegida->nombre);
   printf("hice buffer_add_string\n");	
    buffer_add_uint32(buffer,interfaz_elegida->tipo);
    printf("hice buffer_add_t_tipo_interfaz_enum\n");
    buffer_add_uint32(buffer,unidades_de_trabajo);
    printf("hice buffer_add_uint32\n");
    return buffer;

}