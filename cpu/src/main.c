#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <pthread.h>
#include "../include/main.h"

char *path_config;
char *ip_cpu;

t_pcb* proceso_actual;
t_proceso_interrumpido* proceso_interrumpido_actual;
bool interrupcion_kernel;
instr_t *prox_inst;
t_list* tlb;

int conexion_kernel;

sem_t sem_valor_instruccion;
uint32_t marco_recibido;
sem_t sem_marco_recibido;
sem_t sem_valor_registro_recibido;
sem_t sem_valor_resize_recibido;
sem_t sem_valor_tamanio_pagina;
uint32_t tamanio_pagina;
pthread_mutex_t mutex_proceso_actual;
pthread_mutex_t mutex_proceso_interrumpido_actual;
pthread_mutex_t mutex_interrupcion_kernel;

int socket_memoria;

char* valor_registro_obtenido;
char* rta_resize;



int main(int argc, char* argv[]) {
    decir_hola("CPU");

    path_config = argv[1];
    ip_cpu = argv[2];

    int socket_memoria;

    sem_init(&sem_valor_instruccion, 0, 0);
    sem_init(&sem_marco_recibido, 0, 0);
    sem_init(&sem_valor_registro_recibido, 0, 0);
    sem_init(&sem_valor_resize_recibido, 0, 0);
    sem_init(&sem_valor_tamanio_pagina, 0, 0);
    pthread_mutex_init(&mutex_proceso_actual, NULL);
    pthread_mutex_init(&mutex_proceso_interrumpido_actual, NULL);
    pthread_mutex_init(&mutex_interrupcion_kernel, NULL);

    tlb = list_create();

    printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config)) {
        //cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        liberar_memoria();
        return EXIT_FAILURE;
    }

    log_info(logger_cpu, "empieza el programa");

    pthread_t servidor_dispatch;
    pthread_create(&servidor_dispatch, NULL, crear_servidor_dispatch, ip_cpu);
    pthread_detach(servidor_dispatch);

    pthread_t servidor_interrupt;
    pthread_create(&servidor_interrupt, NULL, crear_servidor_interrupt, ip_cpu);
    pthread_detach(servidor_interrupt);
    log_info(logger_cpu, "cree los hilos servidor");

    // Obtener tamaño de página
    obtenerTamanioPagina(socket_memoria);
    sem_wait(&sem_valor_tamanio_pagina);

    prox_inst = malloc(sizeof(instr_t));

    log_info(logger_cpu, "se creo el servidor");

    socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);

    if (hacer_handshake(socket_memoria) == HANDSHAKE) {
        log_info(logger_cpu, "Correcto en handshake con memoria");
    } else {
        log_info(logger_cpu, "Error en handshake con memoria");
        liberar_memoria();
        return EXIT_FAILURE;
    }

    while(1){
        if(proceso_actual != NULL){
            ciclo_de_instrucciones(socket_memoria, logger_cpu, cfg_cpu, proceso_actual);
        }
    }

    liberar_memoria();

    return 0;
}

void ciclo_de_instrucciones(int conexion, t_log* logger, t_config* config, t_pcb* proceso) {
    log_info(logger, "Entro al ciclo");
    instr_t *inst = malloc(sizeof(instr_t));
    log_info(logger, "Voy a entrar a fetch");
    inst = fetch(conexion, logger, config, proceso);
    tipo_instruccion tipo_inst;
    log_info(logger, "Voy a entrar a decode");
    tipo_inst = decode(inst);
    log_info(logger, "Voy a entrar a execute");
    execute(logger, config, inst, tipo_inst, proceso);
    if(proceso_actual != NULL){// Si la instruccion que acaba de ejecutar no es EXIT
        pthread_mutex_lock(&mutex_proceso_actual);
        proceso_actual->program_counter += 1;
        pthread_mutex_unlock(&mutex_proceso_actual);
        log_info(logger, "Voy a entrar a check_interrupt");
        check_interrupt();
        log_info(logger, "Sale de check_interrupt");
    }
    
    
    log_info(logger, "Termino ciclo de instrucciones");
    pthread_mutex_lock(&mutex_interrupcion_kernel);
    interrupcion_kernel = false;
    pthread_mutex_unlock(&mutex_interrupcion_kernel);
    free(inst->param1);
    free(inst->param2);
    free(inst->param3);
    free(inst->param4);
    free(inst->param5);
    free(inst);
}

void liberar_memoria() {
    // Liberar la lista tlb
    if (tlb) {
        list_destroy_and_destroy_elements(tlb, free);
    }

    // Liberar semáforos
    sem_destroy(&sem_valor_instruccion);
    sem_destroy(&sem_marco_recibido);
    sem_destroy(&sem_valor_registro_recibido);
    sem_destroy(&sem_valor_resize_recibido);
    sem_destroy(&sem_valor_tamanio_pagina);

    free(prox_inst);

    
    if (proceso_actual) {
        // libero todas las estructuras dentro de proceso_actual
        if (proceso_actual->path) {
            free(proceso_actual->path);
        }
        free(proceso_actual);
    }

    if (proceso_interrumpido_actual) {
        
        free(proceso_interrumpido_actual);
    }

    // Liberar strings
    free(path_config);
    free(ip_cpu);
    free(valor_registro_obtenido);
    free(rta_resize);

    //Liberar config y logger(declarados en init_cpu)
    cerrar_programa();
}
