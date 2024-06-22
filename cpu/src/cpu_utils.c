#include"../include/cpu_utils.h"

int tamanioParams;
int tamanioInterfaces;

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
                strcpy(prox_inst->param2, "1");
        //INSTRUCCION DE PRUEBA 2 (IO_GEN_SLEEP)
              prox_inst->idLength = 4;
                prox_inst->id = IO_GEN_SLEEP;  
                prox_inst->param1Length = (strlen("Int1") + 1) * sizeof(char*);
                prox_inst->param1 = malloc(prox_inst->param1Length);
                strcpy(prox_inst->param1, "Int1");
                
                prox_inst->param2Length = (strlen("10")+1) * sizeof(char*);
                prox_inst->param2 = malloc(prox_inst->param2Length);
                strcpy(prox_inst->param2, "10");*/
       // log_info(logger_cpu, "WAIT SEMAFORO");
       // sem_wait(&sem_conexion_lista);
        
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
            sum(inst->param1, inst->param2,proceso,logger);
            break;
        }
        case SUB:
        {
            sub(inst->param1, inst->param2,proceso,logger);
            break;
        }
        case JNZ:
        {
            jnz(inst->param1, inst->param2,proceso,logger);
            break;
        }
        case IO_GEN_SLEEP:
        {
            char *endptr;
            uint32_t param2_num = (uint32_t)strtoul(inst->param2, &endptr, 10);// Convertir la cadena a uint32_t
            
            io_gen_sleep(inst->param1, param2_num,proceso);
            break;
        }

        case MOV_IN:
        {
          
            mov_in(inst->param1, inst->param2,proceso,logger);
            break;
        }
        
        case MOV_OUT:
        {
          
            mov_out(inst->param1, inst->param2,proceso,logger);
            break;
        }

        case RESIZE:
        {
             char *endptr;
            uint32_t param1_num = (uint32_t)strtoul(inst->param1, &endptr, 10);// Convertir la cadena a uint32_t
            
            resize(param1_num);
            break;
        }

        case COPY_STRING:
        {
             char *endptr;
            uint32_t param1_num = (uint32_t)strtoul(inst->param1, &endptr, 10);// Convertir la cadena a uint32_t
            
            copy_string(param1_num);
            break;
        }

        case WAIT:
        {
            wait_inst(inst->param1);
            break;
        }

        case SIGNAL:
        {
            signal_inst(inst->param1);
            break;
        }

        case IO_STDIN_READ:
        {
            io_stdin_read(inst->param1,inst->param2,inst->param3,proceso,logger);
            break;
        }

        case IO_STDOUT_WRITE:
        {
            io_stdout_write(inst->param1,inst->param2,inst->param3,proceso,logger);
            break;
        }

        case EXIT:
        {
            exit_inst();
            break;
        }

        case IO_FS_CREATE:
        {
            io_fs_create(inst->param1,inst->param2,proceso,logger);
            break;
        }

        case IO_FS_DELETE:
        {
            io_fs_delete(inst->param1,inst->param2,proceso,logger);
            break;
        }

        case IO_FS_TRUNCATE:
        {
            io_fs_truncate(inst->param1,inst->param2,inst->param3,proceso,logger);
            break;
        }

        case IO_FS_WRITE:
        {
            io_fs_write(inst->param1,inst->param2,inst->param3,inst->param4,inst->param5,proceso,logger);
            break;
        }

        case IO_FS_READ:
        {
            io_fs_read(inst->param1,inst->param2,inst->param3,inst->param4,inst->param5,proceso,logger);
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
    //printf("El valor del set es : %d ", valor);
    registros registro_elegido = identificarRegistro(registro);
    switch(registro_elegido){
        case PC:
        {
           proceso->pcb->registros_cpu->PC = valor;
            break;
        }
        case AX:
        {
           proceso->pcb->registros_cpu->AX = valor;
            break;
        }
        case BX:
        {
           proceso->pcb->registros_cpu->BX = valor;
            break;
        }
        case CX:
        {
           proceso->pcb->registros_cpu->CX = valor;
            break;
        }
        case DX:
        {
           proceso->pcb->registros_cpu->DX = valor;
            break;
        }
        case EAX:
        {
           proceso->pcb->registros_cpu->EAX = valor;
            break;
        }
        case EBX:
        {
           proceso->pcb->registros_cpu->EBX = valor;
            break;
        }
        case ECX:
        {
           proceso->pcb->registros_cpu->ECX = valor;
            break;
        }
        case EDX:
        {
           proceso->pcb->registros_cpu->EDX = valor;
            break;
        }
        case SI:
        {
           proceso->pcb->registros_cpu->SI = valor;
            break;
        }
        case DI:
        {
           proceso->pcb->registros_cpu->DI = valor;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }

    //proceso->pcb->registros_cpu->AX;
   // registro = valor;
}

void sum(char* registro_destino, char* registro_origen, t_proceso* proceso, t_log *logger){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso,logger);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso,logger);

    switch(id_registro_destino){
        case PC:
        {
           proceso->pcb->registros_cpu->PC = valor_reg_destino + valor_reg_origen;
            break;
        }
        case AX:
        {
           proceso->pcb->registros_cpu->AX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case BX:
        {
           proceso->pcb->registros_cpu->BX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case CX:
        {
           proceso->pcb->registros_cpu->CX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case DX:
        {
           proceso->pcb->registros_cpu->DX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EAX:
        {
           proceso->pcb->registros_cpu->EAX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EBX:
        {
           proceso->pcb->registros_cpu->EBX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case ECX:
        {
           proceso->pcb->registros_cpu->ECX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EDX:
        {
           proceso->pcb->registros_cpu->EDX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case SI:
        {
           proceso->pcb->registros_cpu->SI = valor_reg_destino + valor_reg_origen;
            break;
        }
        case DI:
        {
           proceso->pcb->registros_cpu->DI = valor_reg_destino + valor_reg_origen;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }


    //registro_destino = registro_destino + registro_origen;
}

void sub(char* registro_destino, char* registro_origen, t_proceso* proceso, t_log *logger){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso,logger);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso,logger);

    switch(id_registro_destino){
        case PC:
        {
           proceso->pcb->registros_cpu->PC = valor_reg_destino - valor_reg_origen;
            break;
        }
        case AX:
        {
           proceso->pcb->registros_cpu->AX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case BX:
        {
           proceso->pcb->registros_cpu->BX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case CX:
        {
           proceso->pcb->registros_cpu->CX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case DX:
        {
           proceso->pcb->registros_cpu->DX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EAX:
        {
           proceso->pcb->registros_cpu->EAX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EBX:
        {
           proceso->pcb->registros_cpu->EBX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case ECX:
        {
           proceso->pcb->registros_cpu->ECX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EDX:
        {
           proceso->pcb->registros_cpu->EDX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case SI:
        {
           proceso->pcb->registros_cpu->SI = valor_reg_destino - valor_reg_origen;
            break;
        }
        case DI:
        {
           proceso->pcb->registros_cpu->DI = valor_reg_destino - valor_reg_origen;
            break;
        }
        default:
        log_info(logger, "El registro no existe");
    }
    //registro_destino = registro_destino - registro_origen;
}

void jnz(char* registro, uint32_t inst, t_proceso* proceso, t_log* logger){
    registros id_registro = identificarRegistro(registro);
    uint32_t valor_registro = obtenerValorActualRegistro(id_registro,proceso, logger);
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
enviar_interfaz_a_kernel(interfaz_elegida, unidades_de_trabajo,conexion_kernel);//VER IMPLEMENTACION
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
    t_paquete* paquete_interrupcion_kernel;
    uint32_t tamanio_lista = malloc(sizeof(uint32_t));
    tamanio_lista = proceso_interrumpido_actual->pcb->lista_recursos_pcb->elements_count;
 
    paquete_interrupcion_kernel = crear_paquete(INTERRUPCION_CPU);
 
    agregar_a_paquete(paquete_interrupcion_kernel,  &proceso_interrumpido_actual->pcb->pid,  sizeof(uint32_t));         
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->program_counter, sizeof(uint32_t));  
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->path_length, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_interrupcion_kernel, proceso_interrumpido_actual->pcb->path, proceso_interrumpido_actual->pcb->path_length);
    //VER SI ES UNA LISTA O DEBE SER UN DICCIONARIO
    uint32_t recurso = malloc(sizeof(uint32_t));
    for (int i = 0; i < tamanio_lista; i++) {
        recurso = list_get(proceso_interrumpido_actual->pcb->lista_recursos_pcb, i);
        agregar_a_paquete(paquete_interrupcion_kernel, &recurso, sizeof(uint32_t)); //CONFIRMAR EL TIPO DE DATO DE LA LISTA/DICCIONARIO

    }
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->mutex_lista_recursos, sizeof(pthread_mutex_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->AX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->BX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->CX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->DX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->EAX, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->EBX, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->ECX, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->EDX, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->SI, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->registros_cpu->DI, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->estado, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->pcb->tiempo_ejecucion, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, &proceso_interrumpido_actual->tamanio_motivo_interrupcion, sizeof(uint32_t));
    agregar_a_paquete(paquete_interrupcion_kernel, proceso_interrumpido_actual->motivo_interrupcion, proceso_interrumpido_actual->tamanio_motivo_interrupcion);

    enviar_paquete(paquete_interrupcion_kernel, conexion);   
    free(paquete_interrupcion_kernel);
    free(tamanio_lista);
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
	//buffer_add(buffer, &pcb->pid, sizeof(int));
	buffer_add(buffer, &pcb->program_counter, sizeof(int));
	//buffer_add(buffer, &pcb->quantum, sizeof(int));
	buffer_add(buffer, &pcb->registros_cpu, sizeof(t_registros_CPU));
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

/*t_buffer *proceso_serializar(t_proceso* proceso) {
	
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
  /* 	  for(int i = 0; i < list_size(proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso->interfaces,i));
	  }
    //buffer_add_interfaz(buffer, persona->interfaces); //RECORRER LISTA DE INTERFACES CON FOR Y HACER ESTO POR CADA UNA

    return buffer;
}*/

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
    nuevo_proceso->pcb = proceso->pcb;
    strcpy(nuevo_proceso->motivo_interrupcion, motivo);
    return nuevo_proceso;
}

/*t_buffer *proceso_interrumpido_serializar(t_proceso_interrumpido* proceso_interrumpido) {
	

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
	      /* 

   	  for(int i = 0; i < list_size(proceso_interrumpido->proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso_interrumpido->proceso->interfaces,i));
	  }
      printf("agrego interfaces\n");	

    return buffer;
}*/

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

uint32_t obtenerValorActualRegistro(registros id_registro, t_proceso* proceso, t_log* logger){
    switch(id_registro){
        case PC:
        {
           return proceso->pcb->registros_cpu->PC;
            break;
        }
        case AX:
        {
           return proceso->pcb->registros_cpu->AX;
            break;
        }
        case BX:
        {
           return proceso->pcb->registros_cpu->BX;
            break;
        }
        case CX:
        {
           return proceso->pcb->registros_cpu->CX;
            break;
        }
        case DX:
        {
           return proceso->pcb->registros_cpu->DX;
            break;
        }
        case EAX:
        {
           return proceso->pcb->registros_cpu->EAX;
            break;
        }
        case EBX:
        {
           return proceso->pcb->registros_cpu->EBX;
            break;
        }
        case ECX:
        {
           return proceso->pcb->registros_cpu->ECX;
            break;
        }
        case EDX:
        {
           return proceso->pcb->registros_cpu->EDX;
            break;
        }
        case SI:
        {
           return proceso->pcb->registros_cpu->SI;
            break;
        }
        case DI:
        {
           return proceso->pcb->registros_cpu->DI;
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

void enviar_interfaz_a_kernel(t_interfaz* interfaz_elegida,uint32_t unidades_de_trabajo, int conexion){
    printf("entro a enviar_interfaz_a_kernel\n");
    t_paquete* paquete_interfaz_kernel;
   
    paquete_interfaz_kernel = crear_paquete(ENVIO_INTERFAZ); 
    
    agregar_a_paquete(paquete_interfaz_kernel, &proceso_actual->pcb->pid, sizeof(uint32_t));
    agregar_a_paquete(paquete_interfaz_kernel, &interfaz_elegida->nombre_length, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_interfaz_kernel, interfaz_elegida->nombre, interfaz_elegida->nombre_length);
    agregar_a_paquete(paquete_interfaz_kernel, &unidades_de_trabajo, sizeof(uint32_t));
    
       
    enviar_paquete(paquete_interfaz_kernel, conexion); 
    free(paquete_interfaz_kernel);
}

/*t_buffer* envio_interfaz_serializar(t_interfaz* interfaz_elegida, uint32_t unidades_de_trabajo){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
tamanioBuffer = ((strlen(interfaz_elegida->nombre) + 1)* sizeof(char*)) //TAMANIO DEL NOMBRE DE LA INTERFAZ
             + sizeof(t_tipo_interfaz_enum)
             + sizeof(uint32_t) // unidades_trabajo
             + sizeof(uint32_t); // pid
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
    buffer_add_uint32(buffer,proceso_actual->pcb->pid);
    printf("hice buffer_add_uint32\n");
    return buffer;

}*/

uint32_t mmu(uint32_t direccion_logica, uint32_t tamanio_pag, int conexion){
    uint32_t direccion_resultado = malloc(sizeof(uint32_t));
    bool encontro_en_tlb = false;
    uint32_t indice_encontrado = malloc(sizeof(uint32_t));
   // char* valor_direccion_logica = concatenar_cadenas(uint32_to_string(direccion_logica->nro_pag),uint32_to_string(direccion_logica->nro_pag));
    uint32_t nro_pagina = malloc(sizeof(uint32_t));
    uint32_t desplazamiento = malloc(sizeof(uint32_t));
 //CALCULAR NRO DE PAGINA Y DESPLAZAMIENTO
    nro_pagina =  floor(direccion_logica / tamanio_pag);
    desplazamiento = direccion_logica - nro_pagina * tamanio_pag;

    //CHEQUEAR EN TLB
    //list_iterate(tlb, verificar_existencia_en_tlb(&encontro_en_tlb));

       	  for(uint32_t i = 0; i < tlb->elements_count; i++){	
			//if(verificar_existencia_en_tlb(proceso_actual->pcb->pid, direccion_logica->nro_pag, i)){
            if(verificar_existencia_en_tlb(proceso_actual->pcb->pid, nro_pagina, i)){
                encontro_en_tlb = true;
                indice_encontrado = i;
            }
	  }

      if(encontro_en_tlb){
        t_registro_tlb* registro_tlb_encontrado = malloc(sizeof(t_registro_tlb));
        registro_tlb_encontrado = list_get(tlb,indice_encontrado);
        //direccion_resultado->nro_frame = registro_tlb_encontrado->nro_marco;
        //HACER CALCULO DIRECCION FISICA
        direccion_resultado = registro_tlb_encontrado->nro_marco * tamanio_pag;
      }
      else{//Si no esta, pedir a memoria el marco para esa pagina y proceso
        pedir_marco_a_memoria(proceso_actual->pcb->pid,nro_pagina,conexion);
        //al recibir marco de memoria guardarlo en la TLB, si no hay espacio usar algoritmo
        //WAIT SEMAFORO MARCO RECIBIDO
        sem_wait(&sem_marco_recibido);
    if(tlb->elements_count = cfg_cpu->CANTIDAD_ENTRADAS_TLB){
        usar_algoritmo_tlb(proceso_actual->pcb->pid,nro_pagina,marco_recibido); //TODO:IMPLEMENTAR FUNCION
        direccion_resultado = marco_recibido * tamanio_pag;
    }
    else{
        agregar_a_tlb(proceso_actual->pcb->pid,nro_pagina,marco_recibido);
        direccion_resultado = marco_recibido * tamanio_pag;
    }
      }

    //ver el caso en que me piden un tamaño que no entra en la pagina
    return direccion_resultado;
}

bool verificar_existencia_en_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t indice){
    //buscar por indice en la lista(tlb) si existe
    t_registro_tlb* registro_tlb_actual = malloc(sizeof(t_registro_tlb));

    registro_tlb_actual = list_get(tlb,indice);

    if(registro_tlb_actual->pid = pid && registro_tlb_actual->nro_pagina == nro_pagina){
        return true;
    }

  return false;

}

/*char* uint32_to_string(uint32_t number) {
    // Un uint32_t tiene un máximo de 10 dígitos, más el terminador nulo.
    char* str = malloc(11 * sizeof(char));
    if (str == NULL) {
        return NULL;
    }

    sprintf(str, "%u", number);

    return str;
}*/

/*char* concatenar_cadenas(const char* str1, const char* str2) {
    // Calcular la longitud de las dos cadenas
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    
    // Asignar suficiente memoria para la cadena resultante
    // +1 para el carácter nulo
    char* resultado = malloc((len1 + len2 + 1) * sizeof(char));
    if (resultado == NULL) {
        // Manejo de error en caso de fallo en la asignación de memoria
        return NULL;
    }

    // Copiar la primera cadena al resultado
    strcpy(resultado, str1);
    // Concatenar la segunda cadena al resultado
    strcat(resultado, str2);

    return resultado;
}*/

/*uint32_t string_a_uint32(const char* str) {
    // Usamos strtoul para convertir la cadena a un número sin signo
    char *endptr;
    unsigned long valor = strtoul(str, &endptr, 10);

    // Verificar si se produjo un error durante la conversión
    if (*endptr != '\0' || valor > UINT32_MAX) {
        fprintf(stderr, "Conversión inválida o fuera de rango: %s\n", str);
        exit_inst(EXIT_FAILURE);
    }

    return (uint32_t)valor;
}*/

void pedir_marco_a_memoria(uint32_t pid, uint32_t nro_pagina, int conexion){
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    printf("Voy a entrar a pedir_marco_a_memoria");
    t_busqueda_marco* busqueda_marco = malloc(sizeof(t_busqueda_marco));
    busqueda_marco->nro_pagina = nro_pagina;
    busqueda_marco->pid;

     printf("Voy a entrar a busqueda_marco_serializar");
    paquete -> codigo_operacion = PEDIDO_MARCO_A_MEMORIA;
    paquete->buffer = busqueda_marco_serializar(busqueda_marco);
     printf("codOpe: %d,  size buffer:%d, size int: %d, %d", paquete->codigo_operacion,paquete->buffer->size,sizeof(uint32_t),sizeof(op_code));

    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    

// Por último enviamos
    send(conexion, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0); //VER que socket poner(reemplazar unSocket)
 printf("Envio de mensaje\n");
// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(busqueda_marco);
}

t_buffer* busqueda_marco_serializar(t_busqueda_marco* busqueda_marco){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
tamanioBuffer = sizeof(uint32_t) //PID
             + sizeof(uint32_t); // NRO_PAGINA		 
  t_buffer *buffer = buffer_create(tamanioBuffer);
printf("empiezo con los buffer_add\n");	
    buffer_add_uint32(buffer,busqueda_marco->pid);
    printf("hice buffer_add_uint32 (PID)\n");
    buffer_add_uint32(buffer,busqueda_marco->nro_pagina);
    printf("hice buffer_add_uint32 (NRO_PAGINA)\n");
    return buffer;

}

void agregar_a_tlb(uint32_t pid, uint32_t nro_pag, uint32_t marco){
    t_registro_tlb* nuevo_registro = malloc(sizeof(t_registro_tlb));
    nuevo_registro->pid = pid;
    nuevo_registro->nro_pagina = nro_pag;
    nuevo_registro->nro_marco = marco;
    list_add(tlb,nuevo_registro);
}

void mov_in(char* registro_datos, char* registro_direccion, t_proceso* proceso, t_log* logger){
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el 
    //Registro Dirección y lo almacena en el Registro Datos
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso, logger);

    uint32_t dir_fisica_result = malloc(sizeof(uint32_t));
    dir_fisica_result = mmu(valor_registro_direccion,tamanio_pagina,socket_memoria);

    pedir_valor_a_memoria(dir_fisica_result,proceso->pcb->pid);
    wait(&sem_valor_registro_recibido);
    
    set(registro_datos,valor_registro_obtenido,proceso,logger);

}

void mov_out(char* registro_direccion, char* registro_datos, t_proceso* proceso, t_log* logger){
    // Lee el valor del Registro Datos y lo escribe en la dirección física de
    // memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
    registros id_registro_datos = identificarRegistro(registro_datos);
    uint32_t valor_registro_datos = obtenerValorActualRegistro(id_registro_datos,proceso, logger);
    
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso, logger);

    uint32_t dir_fisica_result = malloc(sizeof(uint32_t));
    dir_fisica_result = mmu(valor_registro_direccion,tamanio_pagina,socket_memoria);
    //TODO: Si el tamanio de valor_registro_datos(es un int de 32 siempre?) es mayor a tamanio_pagina hay
    //que dividir ambos y tomar el floor para obtener cant de paginas, con eso dividir datos a enviar en *cant de paginas*, y
    //por cada pedacito de intfo llamar a mmu y agregar dir fisca obtenida en lista 


    guardar_en_direccion_fisica(dir_fisica_result,valor_registro_datos,proceso->pcb->pid);//TODO

}

void resize(uint32_t tamanio){
    //Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado
    //por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá
    //devolver el contexto de ejecución al Kernel informando de esta situación.

    solicitar_resize_a_memoria(proceso_actual,tamanio);
    //WAIT SEMAFORO
    sem_wait(&sem_valor_resize_recibido);
    if(strcmp(rta_resize, "Out of memory") == 0){

        envia_error_de_memoria_a_kernel(proceso_actual);
    }
}

void copy_string(uint32_t tamanio){
    //Toma del string apuntado por el registro SI y copia la cantidad de bytes indicadas
    // en el parámetro tamaño a la posición de memoria apuntada por el registro DI

    //obtener valor de SI, obtener string recortado, mandar mensaje a memoria con direccion y string cortado(nuev struct?)
    
    //registros id_registro_SI = identificarRegistro("SI");
    //uint32_t valor_registro_SI = obtenerValorActualRegistro(id_registro_SI,proceso, logger);

    uint32_t dir_fisica_SI = malloc(sizeof(uint32_t));
    dir_fisica_SI = mmu(proceso_actual->pcb->registros_cpu->SI,tamanio_pagina,socket_memoria);

    pedir_valor_a_memoria(dir_fisica_SI,proceso_actual->pcb->pid);
    wait(&sem_valor_registro_recibido);


    char* valor_a_enviar = malloc(tamanio);
    valor_a_enviar = string_substring_until(valor_registro_obtenido,tamanio); //VER BIEN QUE HACE LA FUNCION

    uint32_t dir_fisica_DI = malloc(sizeof(uint32_t));
    dir_fisica_DI = mmu(proceso_actual->pcb->registros_cpu->DI,tamanio_pagina,socket_memoria);

    guardar_string_en_memoria(valor_a_enviar,tamanio,dir_fisica_DI,proceso_actual->pcb->pid);
}

void wait_inst(char* recurso){
    // Esta instrucción solicita al Kernel que se asigne una instancia del recurso
    //indicado por parámetro.

    solicitar_wait_kernel(proceso_actual,(strlen(recurso) + 1) * sizeof(char),recurso); 
}

void signal_inst(char* recurso){
    //Esta instrucción solicita al Kernel que se libere una instancia del recurso
    //indicado por parámetro
    solicitar_signal_kernel(proceso_actual,(strlen(recurso) + 1) * sizeof(char) ,recurso);
}

void io_stdin_read(char* interfaz, char* registro_direccion, char* registro_tamanio, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al Kernel que mediante la interfaz ingresada 
    //se lea desde el STDIN (Teclado) un valor cuyo tamaño está delimitado 
    //por el valor del Registro Tamaño y el mismo se guarde a partir de la
    //Dirección Lógica almacenada en el Registro Dirección.

    //obtener valores de los parametros, luego guardalos en una struct nueva, serializar y mandar
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso, logger);
    //TRADUCIR DIR LOGICA A FISICA?
    uint32_t dir_fisica = malloc(sizeof(uint32_t));
    dir_fisica = mmu(valor_registro_direccion,tamanio_pagina,socket_memoria);

    registros id_registro_tamanio = identificarRegistro(registro_tamanio);
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_tamanio = obtenerValorActualRegistro(id_registro_tamanio,proceso, logger);
    solicitar_io_stdin_read_a_kernel((strlen(interfaz) + 1) * sizeof(char) ,interfaz,dir_fisica,valor_registro_tamanio);
}

void io_stdout_write(char* interfaz, char* registro_direccion, char* registro_tamanio, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea 
    //desde la posición de memoria indicada por la Dirección Lógica almacenada
    // en el Registro Dirección, un tamaño indicadopor el Registro Tamaño y se imprima por pantalla.

    //obtener valores de los parametros, luego guardalos en una struct nueva, serializar y mandar
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso, logger);
    //TRADUCIR DIR LOGICA A FISICA?
    uint32_t dir_fisica = malloc(sizeof(uint32_t));
    dir_fisica = mmu(valor_registro_direccion,tamanio_pagina,socket_memoria);

    registros id_registro_tamanio = identificarRegistro(registro_tamanio);
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_tamanio = obtenerValorActualRegistro(id_registro_tamanio,proceso, logger);
    solicitar_io_stdout_write_a_kernel((strlen(interfaz) + 1) * sizeof(char),interfaz,dir_fisica,valor_registro_tamanio);
}

void exit_inst(){
    // Esta instrucción representa la syscall de finalización del proceso. Se deberá devolver el
    //Contexto de Ejecución actualizado al Kernel para su finalización.

    solicitar_exit_a_kernel(proceso_actual);

}

void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid){
        printf("entro a pedir_valor_a_memoria\n");
    
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete -> codigo_operacion = PETICION_VALOR_MEMORIA;
    paquete->buffer = direccion_fisica_serializar(dir_fisica,pid);
    printf("sali de direccion_fisica_serializar\n");
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(socket_memoria, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_buffer* direccion_fisica_serializar(uint32_t dir_fisica,uint32_t pid){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
tamanioBuffer =  sizeof(uint32_t) // dir_fisica
                + sizeof(uint32_t); //pid

  t_buffer *buffer = buffer_create(tamanioBuffer);
    buffer_add_uint32(buffer,dir_fisica);
    printf("hice buffer_add_uint32\n");
        buffer_add_uint32(buffer,pid);
    printf("hice buffer_add_uint32\n");
    return buffer;

}

void guardar_en_direccion_fisica(uint32_t dir_fisica_result, uint32_t valor_registro_datos, uint32_t pid){
        printf("entro a guardar_en_direccion_fisica\n");
    
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete -> codigo_operacion = GUARDAR_EN_DIRECCION_FISICA;
    paquete->buffer = direccion_fisica_valor_serializar(dir_fisica_result,valor_registro_datos,pid);
    printf("sali de direccion_fisica_serializar\n");
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(socket_memoria, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_buffer* direccion_fisica_valor_serializar(uint32_t dir_fisica, uint32_t valor, uint32_t pid){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
tamanioBuffer =  sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);  // dir_fisica	 + valor + pid
  t_buffer *buffer = buffer_create(tamanioBuffer);
    buffer_add_uint32(buffer,dir_fisica);
    printf("hice buffer_add_uint32\n");
    buffer_add_uint32(buffer,valor);
    printf("hice buffer_add_uint32\n");
    buffer_add_uint32(buffer,pid);
    printf("hice buffer_add_uint32\n");
    return buffer;

}

void solicitar_resize_a_memoria(t_proceso* proceso, uint32_t tamanio){
       printf("entro a solicitar_resize_a_memoria\n");
    
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete -> codigo_operacion = SOLICITUD_RESIZE;
    paquete->buffer = proceso_resize_serializar(proceso,tamanio);
    printf("sali de proceso_resize_serializar\n");
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(socket_memoria, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_buffer* proceso_resize_serializar(t_proceso* proceso, uint32_t tamanio){
    int tamanioParams = malloc(sizeof(int));
printf("malloc tamanioParams\n");
int tamanioInterfaces = malloc(sizeof(int));
printf("malloc tamanioInterfaces\n");
printf("tamListaInterfaces: %d\n", list_size(proceso->interfaces) );

   	  for(int i = 0; i < list_size(proceso->interfaces); i++){
        printf("entro for tamanioInterfaces\n");	
        printf("list_get name: %s\n",(t_interfaz*)list_get(proceso->interfaces,i) );
            calcularTamanioInterfaz((t_interfaz*)list_get(proceso->interfaces,i));
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
             + sizeof(uint32_t); //tamanio
	printf("calcula tamanioBuffer\n");		 
  t_buffer *buffer = buffer_create(tamanioBuffer);

    buffer_add_pcb(buffer, proceso->pcb);
    //buffer_add_uint8(buffer, proceso_interrumpido->proceso->cantidad_instrucciones);
printf("agrego pcb\n");	
	  /*for(int i = 0; i < proceso_interrumpido->proceso->cantidad_instrucciones; i++){	
			buffer_add_instruccion(buffer, list_get(proceso_interrumpido->proceso->instrucciones,i));
	  }*/
	       

   	  for(int i = 0; i < list_size(proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso->interfaces,i));
	  }
      printf("agrego interfaces\n");	

      buffer_add_uint32(buffer,tamanio);
      printf("agrego tamanio\n");
    return buffer;
}

void envia_error_de_memoria_a_kernel(t_proceso* proceso){
        printf("entro a envia_error_de_memoria_a_kernel\n");
        t_paquete* paquete_error_memoria;
        uint32_t tamanio_lista = malloc(sizeof(uint32_t));
        tamanio_lista = proceso->pcb->lista_recursos_pcb->elements_count;
   
    paquete_error_memoria = crear_paquete(ENVIAR_ERROR_MEMORIA_A_KERNEL); 
    
    agregar_a_paquete(paquete_error_memoria,  &proceso->pcb->pid,  sizeof(uint32_t));         
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->program_counter, sizeof(uint32_t));  
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->path_length, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_error_memoria, proceso->pcb->path, proceso->pcb->path_length);
    //VER SI ES UNA LISTA O DEBE SER UN DICCIONARIO
    uint32_t recurso =  malloc(sizeof(uint32_t));
    for (int i = 0; i < tamanio_lista; i++) {
        recurso = list_get(proceso_interrumpido_actual->pcb->lista_recursos_pcb, i);
        agregar_a_paquete(paquete_error_memoria, &recurso, sizeof(uint32_t)); //CONFIRMAR EL TIPO DE DATO DE LA LISTA/DICCIONARIO

    }
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->mutex_lista_recursos, sizeof(pthread_mutex_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->AX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->BX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->CX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->DX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->EAX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->EBX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->ECX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->EDX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->SI, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu->DI, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->estado, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->tiempo_ejecucion, sizeof(uint32_t));
    
       
    enviar_paquete(paquete_error_memoria, conexion_kernel); 
    free(paquete_error_memoria);
   
}

t_buffer* proceso_serializar(t_proceso* proceso){
	

    uint32_t tamanioBuffer =
    calcular_tamanio_proceso(proceso);	 
  t_buffer *buffer = buffer_create(tamanioBuffer);

    buffer_add_pcb(buffer, proceso->pcb);
    //buffer_add_uint8(buffer, proceso_interrumpido->proceso->cantidad_instrucciones);
printf("agrego pcb\n");	
	  /*for(int i = 0; i < proceso_interrumpido->proceso->cantidad_instrucciones; i++){	
			buffer_add_instruccion(buffer, list_get(proceso_interrumpido->proceso->instrucciones,i));
	  }*/
	       

   	  for(int i = 0; i < list_size(proceso->interfaces); i++){	
			buffer_add_interfaz(buffer, list_get(proceso->interfaces,i));
	  }
      printf("agrego interfaces\n");	

    return buffer;
}

void guardar_string_en_memoria(char* valor_a_enviar,uint32_t tamanio_valor,uint32_t direccion, uint32_t pid){
     printf("entro a guardar_string_en_memoria\n");
    
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete -> codigo_operacion = ENVIO_COPY_STRING_A_MEMORIA; 
    paquete->buffer = copy_string_serializar(valor_a_enviar,tamanio_valor,direccion,pid);
    printf("sali de direccion_fisica_serializar\n");
    void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t)); //VER el uint_32

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

// Por último enviamos
    send(socket_memoria, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

t_buffer* copy_string_serializar(char* valor_a_enviar,uint32_t tamanio_valor,uint32_t direccion, uint32_t pid){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
    tamanioBuffer =  tamanio_valor + sizeof(uint32_t); 
  t_buffer *buffer = buffer_create(tamanioBuffer);
    buffer_add_string(buffer,tamanio_valor,valor_a_enviar);
    printf("hice buffer_add_string\n");
    buffer_add_uint32(buffer,direccion);
    printf("hice buffer_add_uint32\n");
    return buffer;
}

void solicitar_wait_kernel(t_proceso* proceso,uint32_t recurso_tamanio ,char* recurso){
        printf("entro a solicitar_wait_kernel\n");
        
        t_paquete* paquete_wait_kernel;
        uint32_t tamanio_lista = malloc(sizeof(uint32_t));
        tamanio_lista = proceso->pcb->lista_recursos_pcb->elements_count;
   
        paquete_wait_kernel = crear_paquete(ENVIO_WAIT_A_KERNEL); 
        
        agregar_a_paquete(paquete_wait_kernel,  &proceso->pcb->pid,  sizeof(uint32_t));         
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->program_counter, sizeof(uint32_t));  
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->path_length, sizeof(uint32_t)); 
        agregar_a_paquete(paquete_wait_kernel, proceso->pcb->path, proceso->pcb->path_length);
        //VER SI ES UNA LISTA O DEBE SER UN DICCIONARIO
        uint32_t recurso_actual =  malloc(sizeof(uint32_t));
    for (int i = 0; i < tamanio_lista; i++) {
        recurso_actual = list_get(proceso_interrumpido_actual->pcb->lista_recursos_pcb, i);
        agregar_a_paquete(paquete_wait_kernel, &recurso, sizeof(uint32_t)); //CONFIRMAR EL TIPO DE DATO DE LA LISTA/DICCIONARIO

        }
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->mutex_lista_recursos, sizeof(pthread_mutex_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->PC, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->AX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->BX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->CX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->DX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->EAX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->EBX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->ECX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->EDX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->SI, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->registros_cpu->DI, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->estado, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &proceso->pcb->tiempo_ejecucion, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &recurso_tamanio, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, recurso, recurso_tamanio);
        
        enviar_paquete(paquete_wait_kernel, conexion_kernel); 
        free(paquete_wait_kernel);
        free(tamanio_lista);

}

t_buffer* proceso_recurso_serializar(t_proceso* proceso, char* registro){
    uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
    
    tamanioBuffer =  calcular_tamanio_proceso(proceso) + (sizeof(char*) * strlen(registro)); 
  t_buffer *buffer = buffer_create(tamanioBuffer);
    buffer = proceso_serializar(proceso);
    buffer_add_string(buffer,sizeof(char*) * strlen(registro),registro);
    printf("hice buffer_add_uint32\n");
    return buffer;
}

uint32_t calcular_tamanio_proceso(t_proceso* proceso){
        uint32_t tamanioParams = malloc(sizeof(uint32_t));
printf("malloc tamanioParams\n");
uint32_t tamanioInterfaces = malloc(sizeof(uint32_t));
printf("malloc tamanioInterfaces\n");
printf("tamListaInterfaces: %d\n", list_size(proceso->interfaces) );

   	  for(int i = 0; i < list_size(proceso->interfaces); i++){
        printf("entro for tamanioInterfaces\n");	
        printf("list_get name: %s\n",(t_interfaz*)list_get(proceso->interfaces,i) );
            calcularTamanioInterfaz((t_interfaz*)list_get(proceso->interfaces,i));
	  }

printf("calcula tamanioInterfaces\n");
uint32_t tamanio_pcb = malloc(sizeof(uint32_t));
tamanio_pcb = sizeof(uint32_t) * 3 + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 4;
printf("calcula tamanio_pcb\n");
//int tamanioInstrucciones = malloc(sizeof(int));
//tamanioInstrucciones = ((sizeof(uint8_t) * 6 + sizeof(tipo_instruccion)) * proceso_interrumpido->proceso->cantidad_instrucciones )+ tamanioParams;

uint32_t tamanioBuffer = malloc(sizeof(uint32_t));
tamanioBuffer = tamanio_pcb
  //           + sizeof(uint8_t) // cantidad_instrucciones
  //           + tamanioInstrucciones
			 + tamanioInterfaces;
	printf("calcula tamanioBuffer\n");
    return tamanioBuffer;
}

void solicitar_signal_kernel(t_proceso* proceso,uint32_t recurso_tamanio,char* recurso){
        printf("entro a solicitar_wait_kernel\n");
        t_paquete* paquete_signal_kernel;
        uint32_t tamanio_lista = malloc(sizeof(uint32_t));
        tamanio_lista = proceso->pcb->lista_recursos_pcb->elements_count;
   
        paquete_signal_kernel = crear_paquete(ENVIO_SIGNAL_A_KERNEL); 
        
        agregar_a_paquete(paquete_signal_kernel,  &proceso->pcb->pid,  sizeof(uint32_t));         
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->program_counter, sizeof(uint32_t));  
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->path_length, sizeof(uint32_t)); 
        agregar_a_paquete(paquete_signal_kernel, proceso->pcb->path, proceso->pcb->path_length);
        //VER SI ES UNA LISTA O DEBE SER UN DICCIONARIO
        uint32_t recurso_actual =  malloc(sizeof(uint32_t));
    for (int i = 0; i < tamanio_lista; i++) {
        recurso_actual = list_get(proceso_interrumpido_actual->pcb->lista_recursos_pcb, i);
        agregar_a_paquete(paquete_signal_kernel, &recurso, sizeof(uint32_t)); //CONFIRMAR EL TIPO DE DATO DE LA LISTA/DICCIONARIO

        }
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->mutex_lista_recursos, sizeof(pthread_mutex_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->PC, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->AX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->BX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->CX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->DX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->EAX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->EBX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->ECX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->EDX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->SI, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->registros_cpu->DI, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->estado, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &proceso->pcb->tiempo_ejecucion, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &recurso_tamanio, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, recurso, recurso_tamanio);
        
        enviar_paquete(paquete_signal_kernel, conexion_kernel); 
        free(paquete_signal_kernel);
        free(tamanio_lista);
}

void solicitar_io_stdin_read_a_kernel(uint32_t tamanio_nombre_interfaz,char* nombre_interfaz, uint32_t direccion, uint32_t tamanio){
      printf("entro a solicitar_io_stdin_read_a_kernel\n");
    
    t_paquete* paquete_io_stdin_read;
        
   
        paquete_io_stdin_read = crear_paquete(SOLICITUD_IO_STDIN_READ); 
        
        agregar_a_paquete(paquete_io_stdin_read,  &proceso_actual->pcb->pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_stdin_read,  &tamanio_nombre_interfaz,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_stdin_read,  nombre_interfaz,  tamanio_nombre_interfaz);       
        agregar_a_paquete(paquete_io_stdin_read, &direccion, sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_stdin_read, &tamanio, sizeof(uint32_t)); 
        
        enviar_paquete(paquete_io_stdin_read, conexion_kernel); 
        free(paquete_io_stdin_read);
        
}

t_buffer* direccion_tamanio_interfaz_serializar(char* interfaz, uint32_t direccion, uint32_t tamanio){
        uint32_t tamanioBuffer; //= malloc(sizeof(uint32_t));
    
    tamanioBuffer =  (sizeof(char*) * strlen(interfaz)) + sizeof(uint32_t) + sizeof(uint32_t); 

    t_buffer *buffer = buffer_create(tamanioBuffer);
    buffer_add_string(buffer,sizeof(char*) * strlen(interfaz),interfaz);
    printf("hice buffer_add_string\n");

    buffer_add_uint32(buffer,direccion);
    printf("hice buffer_add_uint32\n");

    buffer_add_uint32(buffer,tamanio);
    printf("hice buffer_add_uint32\n");
    
    return buffer;
}

void solicitar_io_stdout_write_a_kernel(uint32_t tamanio_nombre_interfaz, char* nombre_interfaz, uint32_t direccion, uint32_t tamanio){
          printf("entro a solicitar_io_stdout_write_a_kernel\n");
    
        t_paquete* paquete_io_stdout_write;
        
   
        paquete_io_stdout_write = crear_paquete(SOLICITUD_IO_STDOUT_WRITE); 
        
        agregar_a_paquete(paquete_io_stdout_write,  &proceso_actual->pcb->pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_stdout_write,  &tamanio_nombre_interfaz,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_stdout_write,  nombre_interfaz,  tamanio_nombre_interfaz);       
        agregar_a_paquete(paquete_io_stdout_write, &direccion, sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_stdout_write, &tamanio, sizeof(uint32_t)); 
        
        enviar_paquete(paquete_io_stdout_write, conexion_kernel); 
        free(paquete_io_stdout_write);
}

void solicitar_exit_a_kernel(t_proceso* proceso){
        printf("entro a solicitar_io_stdout_write_a_kernel\n");
        t_paquete* paquete_exit_kernel;
        uint32_t tamanio_lista = malloc(sizeof(uint32_t));
        tamanio_lista = proceso->pcb->lista_recursos_pcb->elements_count;
        paquete_exit_kernel = crear_paquete(SOLICITUD_EXIT_KERNEL); 
        
        agregar_a_paquete(paquete_exit_kernel,  &proceso->pcb->pid,  sizeof(uint32_t));         
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->program_counter, sizeof(uint32_t));  
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->path_length, sizeof(uint32_t)); 
        agregar_a_paquete(paquete_exit_kernel, proceso->pcb->path, proceso->pcb->path_length);
        //VER SI ES UNA LISTA O DEBE SER UN DICCIONARIO
        uint32_t recurso =  malloc(sizeof(uint32_t));
    for (int i = 0; i < tamanio_lista; i++) {
        recurso = list_get(proceso_interrumpido_actual->pcb->lista_recursos_pcb, i);
        agregar_a_paquete(paquete_exit_kernel, &recurso, sizeof(uint32_t)); //CONFIRMAR EL TIPO DE DATO DE LA LISTA/DICCIONARIO

        }
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->mutex_lista_recursos, sizeof(pthread_mutex_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->PC, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->AX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->BX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->CX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->DX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->EAX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->EBX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->ECX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->EDX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->SI, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu->DI, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->estado, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->tiempo_ejecucion, sizeof(uint32_t));
        
        enviar_paquete(paquete_exit_kernel, conexion_kernel); 
        free(paquete_exit_kernel);
    
}

void usar_algoritmo_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t nro_marco){
    t_registro_tlb* nuevo_registro = malloc(sizeof(t_registro_tlb));
    nuevo_registro->pid = pid;
    nuevo_registro->nro_pagina = nro_pagina;
    nuevo_registro->nro_marco = nro_marco;

    if(strcmp(cfg_cpu->ALGORITMO_TLB, "FIFO") == 0){
        list_add_in_index(tlb,0,nuevo_registro);
    } 
    else if(strcmp(cfg_cpu->ALGORITMO_TLB, "LRU") == 0){
        if(valor_repetido_tlb(nuevo_registro->pid,nuevo_registro->nro_pagina) != -1){
            //Si pid y pagina se repiten, lo pongo al final de la TLB
            list_remove_and_destroy_element(tlb,valor_repetido_tlb(nuevo_registro->pid,nuevo_registro->nro_pagina),free);
            list_add(tlb,nuevo_registro);
        }
        else{
            //Si no esta repetido, se reemplaza el primero de la lista
            list_add_in_index(tlb,0,nuevo_registro);
        }
        
        
    }
    else{
        printf("ALGORITMO DESOCNOCIDO");
    }
}

uint32_t valor_repetido_tlb(uint32_t pid, uint32_t nro_pag){
    uint32_t indice_encontrado = malloc(sizeof(uint32_t));
    indice_encontrado = -1;
    t_registro_tlb* registro_actual = malloc(sizeof(t_registro_tlb));
    for (uint32_t i = 0; i < tlb->elements_count; i++)
    {
        registro_actual = list_get(tlb,i);
        if(registro_actual->pid == pid && registro_actual->nro_pagina == nro_pag){
            indice_encontrado = i;
        }
    }
    
    return indice_encontrado;

}

/*void liberar_memoria_tlb(t_registro_tlb* valor){
    free(valor);
}*/

void obtenerTamanioPagina(int conexion){
    printf("entro a obtenerTamanioPagina\n");
    
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete -> codigo_operacion = SOLICITUD_TAMANIO_PAGINA; 
    void* a_enviar = malloc(sizeof(op_code)); 

    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

    offset += sizeof(op_code);

// Por último enviamos
    send(conexion, a_enviar, sizeof(op_code), 0);

// No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete);
}

void io_fs_create(char* interfaz, char* nombre_archivo, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al Kernel que mediante la
    //interfaz seleccionada, se cree un archivo en el FS montado en dicha interfaz
    
    t_interfaz* interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    if(interfaz_elegida != NULL){
    enviar_io_fs_create_a_kernel(interfaz_elegida->nombre_length,interfaz_elegida->nombre,(strlen(nombre_archivo) + 1) * sizeof(char) ,nombre_archivo,proceso->pcb->pid);//VER IMPLEMENTACION, op:SOLICITUD_IO_FS_CREATE_A_KERNEL
    }
    else{
        printf("La interfaz no se encuentra en el proceso");
    }
    
    printf("Sale de io_fs_create");
}

void io_fs_delete(char* interfaz, char* nombre_archivo, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al Kernel que mediante la
    //interfaz seleccionada, se elimine un archivo en el FS montado en dicha interfaz

    t_interfaz* interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    if(interfaz_elegida != NULL){
    enviar_io_fs_delete_a_kernel(interfaz_elegida->nombre_length,interfaz_elegida->nombre,(strlen(nombre_archivo) + 1) * sizeof(char) ,nombre_archivo,proceso->pcb->pid);//VER IMPLEMENTACION, op:SOLICITUD_IO_FS_DELETE_A_KERNEL
    }
    else{
        printf("La interfaz no se encuentra en el proceso");
    }
    
    printf("Sale de io_fs_delete");
}

void io_fs_truncate(char* interfaz, char* nombre_archivo, char* registro_tamanio, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al
    //Kernel que mediante la interfaz seleccionada, se modifique el tamaño del archivo en el FS
    //montado en dicha interfaz, actualizando al valor que se encuentra en el registro indicado por
    //Registro Tamaño.

    t_interfaz* interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    if(interfaz_elegida != NULL){
        registros id_registro_tamanio = identificarRegistro(registro_tamanio);
        uint32_t valor_registro_tamanio = obtenerValorActualRegistro(id_registro_tamanio,proceso, logger);
        enviar_io_fs_truncate_a_kernel(interfaz_elegida->nombre_length,interfaz_elegida->nombre,(strlen(nombre_archivo) + 1) * sizeof(char) ,nombre_archivo,valor_registro_tamanio,proceso->pcb->pid);//VER IMPLEMENTACION, op:SOLICITUD_IO_FS_TRUNCATE_A_KERNEL
    }
    else{
        printf("La interfaz no se encuentra en el proceso");
    }
}

void io_fs_write(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se
    //lea desde Memoria la cantidad de bytes indicadas por el Registro Tamaño a partir de la
    //dirección lógica que se encuentra en el Registro Dirección y se escriban en el archivo a partir
    //del valor del Registro Puntero Archivo.

    t_interfaz* interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    if(interfaz_elegida != NULL){
        registros id_registro_direccion = identificarRegistro(registro_direccion);
        uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso, logger);
        uint32_t dir_fisica = malloc(sizeof(uint32_t));
        dir_fisica = mmu(valor_registro_direccion,tamanio_pagina,socket_memoria);
        //OBTENER VALOR DE LA DIR FISICA OBTENIDA? O PASO DIRECTAMENTE LA DIR FISICA?
        registros id_registro_tamanio = identificarRegistro(registro_tamanio);
        uint32_t valor_registro_tamanio = obtenerValorActualRegistro(id_registro_tamanio,proceso, logger);
        registros id_registro_puntero_archivo = identificarRegistro(registro_puntero_archivo);
        uint32_t valor_registro_puntero_archivo = obtenerValorActualRegistro(id_registro_puntero_archivo,proceso, logger);

        enviar_io_fs_write_a_kernel(interfaz_elegida->nombre_length,interfaz_elegida->nombre,(strlen(nombre_archivo) + 1) * sizeof(char) ,nombre_archivo,valor_registro_tamanio,dir_fisica,valor_registro_puntero_archivo, proceso->pcb->pid);//VER IMPLEMENTACION, op:SOLICITUD_IO_FS_WRITE_A_KERNEL
    }
    else{
        printf("La interfaz no se encuentra en el proceso");
    }
}

void io_fs_read(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo, t_proceso* proceso, t_log* logger){
    //Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se
    //lea desde el archivo a partir del valor del Registro Puntero Archivo la cantidad de bytes
    //indicada por Registro Tamaño y se escriban en la Memoria a partir de la dirección lógica
    //indicada en el Registro Dirección

    t_interfaz* interfaz_elegida;
    interfaz_elegida = elegir_interfaz(interfaz, proceso); //Esta funcion recorre la lista de interfaces del proceso y se fija cual coincide con la que pasa por parametro(compara nombres y si encuentra devuelve la interfaz)
    if(interfaz_elegida != NULL){
        registros id_registro_direccion = identificarRegistro(registro_direccion);
        uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso, logger);
        uint32_t dir_fisica = malloc(sizeof(uint32_t));
        dir_fisica = mmu(valor_registro_direccion,tamanio_pagina,socket_memoria);
        //OBTENER VALOR DE LA DIR FISICA OBTENIDA? O PASO DIRECTAMENTE LA DIR FISICA?
        registros id_registro_tamanio = identificarRegistro(registro_tamanio);
        uint32_t valor_registro_tamanio = obtenerValorActualRegistro(id_registro_tamanio,proceso, logger);
        registros id_registro_puntero_archivo = identificarRegistro(registro_puntero_archivo);
        uint32_t valor_registro_puntero_archivo = obtenerValorActualRegistro(id_registro_puntero_archivo,proceso, logger);

        enviar_io_fs_read_a_kernel(interfaz_elegida->nombre_length,interfaz_elegida->nombre,(strlen(nombre_archivo) + 1) * sizeof(char) ,nombre_archivo,valor_registro_tamanio,dir_fisica,valor_registro_puntero_archivo, proceso->pcb->pid);//VER IMPLEMENTACION, op:SOLICITUD_IO_FS_READ_A_KERNEL
    }
    else{
        printf("La interfaz no se encuentra en el proceso");
    }
}

void enviar_io_fs_create_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t pid){
    printf("entro a enviar_io_fs_create_a_kernel\n");
    t_paquete* paquete_io_fs_create;
        
   
        paquete_io_fs_create = crear_paquete(SOLICITUD_IO_FS_CREATE_A_KERNEL); 
        
        agregar_a_paquete(paquete_io_fs_create,  &pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_fs_create,  &tamanio_nombre_archivo,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_create,  nombre_archivo,  tamanio_nombre_archivo);       
        agregar_a_paquete(paquete_io_fs_create,  &tamanio_interfaz_elegida,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_create,  interfaz_elegida,  tamanio_interfaz_elegida); 
        
        enviar_paquete(paquete_io_fs_create, conexion_kernel); 
        free(paquete_io_fs_create);
}

void enviar_io_fs_delete_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t pid){
    printf("entro a enviar_io_fs_delete_a_kernel\n");
    t_paquete* paquete_io_fs_delete;
        
   
        paquete_io_fs_delete = crear_paquete(SOLICITUD_IO_FS_DELETE_A_KERNEL); 
        
        agregar_a_paquete(paquete_io_fs_delete,  &pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_fs_delete,  &tamanio_nombre_archivo,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_delete,  nombre_archivo,  tamanio_nombre_archivo);       
        agregar_a_paquete(paquete_io_fs_delete,  &tamanio_interfaz_elegida,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_delete,  interfaz_elegida,  tamanio_interfaz_elegida); 
        
        enviar_paquete(paquete_io_fs_delete, conexion_kernel); 
        free(paquete_io_fs_delete);
}

void enviar_io_fs_truncate_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t tamanio,uint32_t pid){
    printf("entro a enviar_io_fs_truncate_a_kernel\n");
    t_paquete* paquete_io_fs_truncate;
        
   
        paquete_io_fs_truncate = crear_paquete(SOLICITUD_IO_FS_TRUNCATE_A_KERNEL); 
        
        agregar_a_paquete(paquete_io_fs_truncate,  &pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_fs_truncate,  &tamanio_nombre_archivo,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_truncate,  nombre_archivo,  tamanio_nombre_archivo);       
        agregar_a_paquete(paquete_io_fs_truncate,  &tamanio_interfaz_elegida,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_truncate,  interfaz_elegida,  tamanio_interfaz_elegida); 
        agregar_a_paquete(paquete_io_fs_truncate,  &tamanio,  sizeof(uint32_t));
        
        enviar_paquete(paquete_io_fs_truncate, conexion_kernel); 
        free(paquete_io_fs_truncate);
}

void enviar_io_fs_write_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t valor_tamanio,uint32_t direccion_fisica,uint32_t puntero_archivo,uint32_t pid){
    printf("entro a enviar_io_fs_write_a_kernel\n");
    t_paquete* paquete_io_fs_write;
        
   
        paquete_io_fs_write = crear_paquete(SOLICITUD_IO_FS_WRITE_A_KERNEL); 
        
        agregar_a_paquete(paquete_io_fs_write,  &pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_fs_write,  &tamanio_nombre_archivo,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_write,  nombre_archivo,  tamanio_nombre_archivo);       
        agregar_a_paquete(paquete_io_fs_write,  &tamanio_interfaz_elegida,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_write,  interfaz_elegida,  tamanio_interfaz_elegida); 
        agregar_a_paquete(paquete_io_fs_write,  &direccion_fisica,  sizeof(uint32_t));
        agregar_a_paquete(paquete_io_fs_write,  &valor_tamanio,  sizeof(uint32_t));
        agregar_a_paquete(paquete_io_fs_write,  &puntero_archivo,  sizeof(uint32_t));
        
        enviar_paquete(paquete_io_fs_write, conexion_kernel); 
        free(paquete_io_fs_write);
   
}

void enviar_io_fs_read_a_kernel(uint32_t tamanio_interfaz_elegida,char* interfaz_elegida,uint32_t tamanio_nombre_archivo,char* nombre_archivo,uint32_t valor_tamanio,uint32_t direccion_fisica,uint32_t puntero_archivo,uint32_t pid){
    printf("entro a enviar_io_fs_read_a_kernel\n");
    
   t_paquete* paquete_io_fs_read;
        
   
        paquete_io_fs_read = crear_paquete(SOLICITUD_IO_FS_READ_A_KERNEL); 
        
        agregar_a_paquete(paquete_io_fs_read,  &pid,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_io_fs_read,  &tamanio_nombre_archivo,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_read,  nombre_archivo,  tamanio_nombre_archivo);       
        agregar_a_paquete(paquete_io_fs_read,  &tamanio_interfaz_elegida,  sizeof(uint32_t));  
        agregar_a_paquete(paquete_io_fs_read,  interfaz_elegida,  tamanio_interfaz_elegida); 
        agregar_a_paquete(paquete_io_fs_read,  &direccion_fisica,  sizeof(uint32_t));
        agregar_a_paquete(paquete_io_fs_read,  &valor_tamanio,  sizeof(uint32_t));
        agregar_a_paquete(paquete_io_fs_read,  &puntero_archivo,  sizeof(uint32_t));
        
        enviar_paquete(paquete_io_fs_read, conexion_kernel); 
        free(paquete_io_fs_read);
}