
#include "../include/main.h"


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

char *path_config;




int main(char argc, char *argv[]) {
    path_config = argv[1];

    int conexion_consola;
    int conexion_entrada_salida;

//CONFIGURACION

   if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar KERNEL");
        return EXIT_FAILURE;
    }

    contador_pid = 1;

    sem_init(&sem_planificar, 1, 1);
    sem_init(&sem_contexto_ejecucion_recibido,0,0);

    sem_init(&sem_confirmacion_memoria,0,0);
    sem_init(&sem_interrupcion_atendida,0,0);
    sem_init(&sem_io_fs_libre,1,1);
    sem_init(&sem_cpu_libre,1,1);
    sem_init(&sem_prioridad_io,1,1);
    sem_init(&sem_rta_crear_proceso,0,0);
    

    pthread_mutex_init(&mutex_cola_ready_prioridad, NULL);
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_envio_io, NULL);

    Empezar_conexiones();

//INICIAR SERVIDOR Y CONSOLA INTERACTIVA 

    pthread_t hilo1, hilo2, hilo3;
    pthread_t planificacion_largo_plazo,planificacion_corto_plazo;

    pthread_create(&hilo1, NULL, (void*)iniciar_consola_interactiva, conexion_memoria);
    pthread_create(&hilo2, NULL, (void*)crearServidor, NULL);
    pthread_create(&hilo3, NULL, (void*)Escuchar_Msj_De_Conexiones, NULL);
    
    
    int value;
    sem_getvalue(&sem_cpu_libre, &value);
    log_info(logger_kernel,"Semáforo valor sem_cpu_libre : %d\n", value);
    sem_wait(&sem_cpu_libre);

    if (pthread_create(&planificacion_largo_plazo, NULL,(void*) largo_plazo_nuevo_ready,NULL) != 0) {
            perror("pthread_create");            
    }


    if (pthread_create(&planificacion_corto_plazo, NULL,(void*) planificar_y_ejecutar,NULL) != 0) {
            perror("pthread_create");      
            
    }


 
    pthread_detach(&planificacion_largo_plazo);
    pthread_detach(&planificacion_corto_plazo);
    // ESPERAR A QUE LOS HILOS TERINEN (OPCIONAL, EN CASO DE QUE QUIERAS ESPERAR)

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);
    pthread_join(hilo3, NULL);
    cerrar_programa();
       
    return 0;


   

}