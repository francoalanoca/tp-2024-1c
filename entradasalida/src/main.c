#include "../include/main.h"

char *path_config;
char *nombre_interfaz_custom;
int main(char argc, char *argv[]) {
    nombre_interfaz_custom = argv[1];
    path_config = argv[2];
    int socket_memoria;
    int socket_kernel;

    printf("iniciando\n");
      printf("nombre de interfaz custom: %s \n",nombre_interfaz_custom);
    if (!init(path_config) || !cargar_configuracion(nombre_interfaz_custom, path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;
    }


    //CONEXION//
    socket_kernel = crear_conexion(logger_entrada_salida, "KERNEL", cfg_entrada_salida->IP_KERNEL, cfg_entrada_salida->PUERTO_KERNEL);
    
    log_info(logger_entrada_salida, "Socket de KERNEL : %d\n",socket_kernel);   
    
    socket_memoria = crear_conexion(logger_entrada_salida, "MEMORIA", cfg_entrada_salida->IP_MEMORIA, cfg_entrada_salida->PUERTO_MEMORIA);
    
    log_info(logger_entrada_salida, "Socket de MEMORIA : %d\n",socket_memoria);   
     

    
    //INICIAR INTERFACE//
    iniciar_interface(cfg_entrada_salida->TIPO_INTERFAZ,cfg_entrada_salida->NOMBRE_INTERFAZ, socket_kernel, socket_memoria);

    
    



    cerrar_programa();
    return 0;
}