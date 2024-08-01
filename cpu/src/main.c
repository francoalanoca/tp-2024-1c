#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <pthread.h>
#include "../include/main.h"

char *path_config;
char *ip_cpu;

t_pcb *proceso_actual;
t_proceso_interrumpido *proceso_interrumpido_actual;
bool interrupcion_kernel;
instr_t *prox_inst;
t_list *tlb;

int conexion_kernel_dispatch = -1;
int conexion_kernel_interrupt = -1;

sem_t sem_valor_instruccion;
uint32_t marco_recibido;
sem_t sem_marco_recibido;
sem_t sem_valor_registro_recibido;
sem_t sem_valor_resize_recibido;
sem_t sem_valor_tamanio_pagina;
sem_t sem_servidor_creado;
sem_t sem_interrupcion_kernel;
sem_t sem_check_interrupcion_kernel;
sem_t sem_conexion_interrupt_iniciado;
sem_t sem_conexion_dispatch_iniciado;
uint32_t tamanio_pagina;
pthread_mutex_t mutex_proceso_actual;
pthread_mutex_t mutex_proceso_interrumpido_actual;
pthread_mutex_t mutex_interrupcion_kernel;
pthread_t hilo_atender_memoria;

int socket_memoria;

char *valor_registro_obtenido;
int rta_resize;

int main(int argc, char *argv[])
{
    decir_hola("CPU");

    path_config = argv[1];
    ip_cpu = argv[2];
    proceso_actual= NULL;


    sem_init(&sem_valor_instruccion, 0, 0);
    sem_init(&sem_marco_recibido, 0, 0);
    sem_init(&sem_valor_registro_recibido, 0, 0);
    sem_init(&sem_valor_resize_recibido, 0, 0);
    sem_init(&sem_valor_tamanio_pagina, 0, 0);
    sem_init(&sem_servidor_creado, 0, 0);
    sem_init(&sem_interrupcion_kernel, 0, 0);
    sem_init(&sem_check_interrupcion_kernel, 0, 0);
    sem_init(&sem_conexion_interrupt_iniciado, 0, 0);
    sem_init(&sem_conexion_dispatch_iniciado, 0, 0);
    pthread_mutex_init(&mutex_proceso_actual, NULL);
    pthread_mutex_init(&mutex_proceso_interrumpido_actual, NULL);
    pthread_mutex_init(&mutex_interrupcion_kernel, NULL);

    prox_inst = malloc(sizeof(instr_t));
    printf("Creo prox_inst\n");
    tlb = list_create();

    printf("TLB size: %d\n", list_size(tlb));

    printf("iniciando ");
    if (!init(path_config) || !cargar_configuracion(path_config))
    {
        // cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        liberar_memoria();
        return EXIT_FAILURE;
    }

    log_info(logger_cpu, "empieza el programa");
      socket_memoria = crear_conexion(logger_cpu, "MEMORIA", cfg_cpu->IP_MEMORIA, cfg_cpu->PUERTO_MEMORIA);
    log_info(logger_cpu, "cree la conexion con memoria");
    if (hacer_handshake(socket_memoria) == HANDSHAKE_OK)
    {
        log_info(logger_cpu, "Correcto en handshake con memoria");
        sem_post(&sem_servidor_creado);
    }
    else
    {
        log_info(logger_cpu, "Error en handshake con memoria");
        liberar_memoria();
        return EXIT_FAILURE;
    }
    

    
  
    //sem_wait(&sem_conexion_dispatch_iniciado);
    //sem_wait(&sem_conexion_interrupt_iniciado);


   log_info(logger_cpu,"conexion memoria %d",socket_memoria);
    pthread_create(&hilo_atender_memoria, NULL, (void *)atender_memoria, &socket_memoria);
    
       log_info(logger_cpu,"conexion memoria %d",socket_memoria);
    printf("Paso  sem_servidor_creado\n");
    // Obtener tamaño de página
    obtenerTamanioPagina(socket_memoria);
    sem_wait(&sem_valor_tamanio_pagina);


    pthread_t servidor_dispatch;
    pthread_create(&servidor_dispatch, NULL, (void *)crear_servidor_dispatch, ip_cpu);
   // pthread_detach(servidor_dispatch);

    pthread_t servidor_interrupt;
    pthread_create(&servidor_interrupt, NULL, (void *)crear_servidor_interrupt, ip_cpu);
   // pthread_detach(servidor_interrupt);
    log_info(logger_cpu, "cree los hilos servidor");

    /*ANTERIOR
        proceso_interrumpido_actual = malloc(sizeof(t_proceso_interrumpido));
        proceso_interrumpido_actual->pcb = malloc(sizeof(t_pcb));
        proceso_interrumpido_actual->pcb->pid = NULL;*/
    sem_wait(&sem_servidor_creado);
    sem_wait(&sem_conexion_dispatch_iniciado);
    sem_wait(&sem_conexion_interrupt_iniciado);
   // pthread_create(&hilo_ejecutar_ciclo, NULL,(void *) ejecutar_ciclo, (void*)params);
    ciclo_params_t *params = malloc(sizeof(ciclo_params_t));
    params->socket_memoria = socket_memoria;
    params->proceso_actual = proceso_actual;
    params->tlb = tlb;
    params->conexion_kernel_dispatch = conexion_kernel_dispatch;
    params->conexion_kernel_interrupt = conexion_kernel_interrupt;

   pthread_t hilo_ejecutar_ciclo;
   log_info(logger_cpu,"conexion memoria %d",socket_memoria);
      log_info(logger_cpu,"voy a crear hilo");
    int result = pthread_create(&hilo_ejecutar_ciclo, NULL, ejecutar_ciclo,(void*)params);
    //pthread_join(hilo_ejecutar_ciclo,NULL);
    pthread_join(servidor_dispatch, NULL);
    pthread_join(servidor_interrupt, NULL);
    pthread_join(hilo_ejecutar_ciclo, NULL);
    pthread_detach(hilo_atender_memoria);
}

void ejecutar_ciclo(void* arg) {
    ciclo_params_t* params = (ciclo_params_t*)arg;
   

    while (1) {
      
        pthread_mutex_lock(&mutex_proceso_actual);
       
        if (proceso_actual != NULL) {
            pthread_mutex_unlock(&mutex_proceso_actual);
            ciclo_de_instrucciones( &socket_memoria, proceso_actual, tlb, &conexion_kernel_dispatch, &conexion_kernel_interrupt);
        } else {
            pthread_mutex_unlock(&mutex_proceso_actual);
           
        }
    }

    
}

void liberar_memoria()
{
    // Liberar la lista tlb
    if (tlb)
    {
        list_destroy_and_destroy_elements(tlb, free);
    }

    // Liberar semáforos
    sem_destroy(&sem_valor_instruccion);
    sem_destroy(&sem_marco_recibido);
    sem_destroy(&sem_valor_registro_recibido);
    sem_destroy(&sem_valor_resize_recibido);
    sem_destroy(&sem_valor_tamanio_pagina);

    free(prox_inst);

    if (proceso_actual)
    {
        // libero todas las estructuras dentro de proceso_actual
        if (proceso_actual->path)
        {
            free(proceso_actual->path);
        }
        free(proceso_actual);
    }

    if (proceso_interrumpido_actual)
    {

        free(proceso_interrumpido_actual);
    }

    // Liberar strings
    free(path_config);
    free(ip_cpu);
    free(valor_registro_obtenido);
    free(rta_resize);

    // Liberar config y logger(declarados en init_cpu)
    cerrar_programa();
}
