#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include<commons/config.h>
#include <pthread.h>
#include "../include/main.h"


char *path_config;
char *ip_cpu;

t_pcb* proceso_actual;
t_proceso_interrumpido* proceso_interrumpido_actual;
bool interrupcion_kernel;
instr_t *prox_inst;
t_list* tlb;

//int tamanioParams;
//int tamanioInterfaces;
int conexion_kernel;

sem_t sem_conexion_lista;

uint32_t marco_recibido;

sem_t sem_marco_recibido;

sem_t sem_valor_registro_recibido;

sem_t sem_valor_resize_recibido;

sem_t sem_valor_tamanio_pagina;

uint32_t tamanio_pagina;

int socket_memoria;

char* valor_registro_obtenido;

char* rta_resize;


int main(int argc, char* argv[]) {
    decir_hola("CPU");

    path_config = argv[1];
    ip_cpu = argv[2];

	int socket_memoria;
	//char* ip;

    sem_init(&sem_conexion_lista,0,0);

    sem_init(&sem_marco_recibido,0,0);
    
    sem_init(&sem_valor_registro_recibido,0,0);
    
    sem_init(&sem_valor_resize_recibido,0,0);
    
    sem_init(&sem_valor_tamanio_pagina,0,0);

    

    tlb = malloc(sizeof(t_list)); //REVISAR

    tlb = list_create();

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

    /*COMIENZO CONEXION PRUEBA
    socket_memoria = crear_conexion(logger_cpu, "PRUEBA", cfg_cpu->IP_MEMORIA, "8003");
    log_info(logger_cpu, "creo socket");
    if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con memoria");
    }
    else {
        log_info(logger_cpu, "Error en handshake con memoria");
        return EXIT_FAILURE;
    }*///FIN CONEXION PRUEBA

    //COMIENZO OBTENER TAMANIO_PAGINA
    obtenerTamanioPagina(socket_memoria);
    sem_wait(&sem_valor_tamanio_pagina);
    //FIN OBTENER TAMANIO PAGINA
    
    /*COMIENZO DATOS PRUEBA
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
    *///FIN DATOS PRUEBA
     
    //
   // interrupcion_kernel = malloc(sizeof(bool));
   // interrupcion_kernel = true;
    
    prox_inst = malloc(sizeof(instr_t));
    //prox_inst->
    //conexion_kernel =  socket_memoria;
    //
    ////////////////////////////////////////////////////////////
    log_info(logger_cpu, "se creo el servidor");
   
     socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);
      
     if ( (hacer_handshake (socket_memoria) == HANDSHAKE)){
        log_info(logger_cpu, "Correcto en handshake con memoria");
    }
    else {
        log_info(logger_cpu, "Error en handshake con memoria");
        return EXIT_FAILURE;
    }
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
    proceso_actual->program_counter += 1;
    log_info(logger, "Termino ciclo de instrucciones");

}


