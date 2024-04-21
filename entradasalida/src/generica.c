#include "../include/generica.h"


void iniciar_interfaz_generica (int socket_kernel) {
        log_info(logger_entrada_salida, "Interfaz %s de tipo GENERICA iniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  


}

void esperar(int unidades){
    log_info(logger_entrada_salida, "Se solicitaron  %d unidades de trabajo",unidades);
    sleep (unidades * cfg_entrada_salida->TIEMPO_UNIDAD_TRABAJO);
    log_info(logger_entrada_salida, "Se completaron  %d unidades de trabajo",unidades);
}
