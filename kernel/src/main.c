
#include "../include/main.h"


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

char *path_config;




int main(char argc, char *argv[]) {
    path_config = argv[1];

    int conexion_consola;
    int conexion_entrada_salida;


    int contador_pid;

//CONFIGURACION

   if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar KERNEL");
        return EXIT_FAILURE;
    }

    
    sem_init(&sem_planificar, 1, 1);
    sem_init(&sem_contexto_ejecucion_recibido,0,0);

    sem_init(&sem_confirmacion_memoria,0,0);
    sem_init(&sem_interrupcion_atendida,0,0);
    sem_init(&sem_io_fs_libre,0,0);

    pthread_mutex_init(&mutex_cola_ready_prioridad, NULL);
    pthread_mutex_init(&mutex_cola_ready, NULL);


//INICIAR SERVIDOR Y CONSOLA INTERACTIVA 

    pthread_t hilo1, hilo2, hilo3;

    pthread_create(&hilo1, NULL, (void*)iniciar_consola_interactiva, NULL);
    pthread_create(&hilo2, NULL, (void*)crearServidor, NULL);
    pthread_create(&hilo3, NULL, (void*)Escuchar_Msj_De_Conexiones, NULL);

    // ESPERAR A QUE LOS HILOS TERINEN (OPCIONAL, EN CASO DE QUE QUIERAS ESPERAR)

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);
    pthread_join(hilo3, NULL);
    cerrar_programa();
       
    return 0;


   

}