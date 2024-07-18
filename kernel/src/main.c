
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

//INICIAR SERVIDOR y CONSOLA

    crearServidor();
 
    iniciar_consola_interactiva();
    
// ESCUCHAR A LOS MODULOS
 
    Escuchar_Msj_De_Conexiones();
 
//EMPEZAR PLANIFICACIONES


   

//CERRAR PROGRAMA

    cerrar_programa();

}
