#include <protocolo_kernel.h>



//Funcion que crea hilos para cada modulo y los va atendiendo

void Escuchar_Msj_De_Conexiones(){

//Atender los msj de memoria
   pthread_t hilo_kernel_memoria;
   pthread_create(&hilo_kernel_memoria, NULL, (void*)Kernel_atender_memoria, NULL);
   pthread_detach(hilo_kernel_memoria);

//Atender los msj de cpu - dispatch
   pthread_t hilo_cpu_dispatch;
   pthread_create(&hilo_cpu_dispatch, NULL, (void*)Kernel_atender_cpu_dispatch, NULL);
   pthread_detach(hilo_cpu_dispatch);

//Atender los msj de cpu - interrupt
   pthread_t hilo_cpu_interrupt;
   pthread_create(&hilo_cpu_interrupt, NULL, (void*)Kernel_atender_cpu_interrupt, NULL);
   pthread_join(hilo_cpu_interrupt, NULL);

}


void Kernel_atender_cpu_dispatch(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_cpu_dispatch);
   switch (cod_op)
   {
   case MENSAJE:
      //
      break;
   case PAQUETE:
      //
      break;
   case -1:
      log_error(logger_kernel, "Desconexion de cpu - Dispatch");
      control_key = 0;
      break;
   default:
      log_warning(logger_kernel, "Operacion desconocida de cpu - Dispatch");
      break;
   }
}

}

void Kernel_atender_cpu_interrupt(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_cpu_interrupt);
   switch (cod_op)
   {
   case MENSAJE:
      //
      break;
   case PAQUETE:
      //
      break;
   case -1:
      log_error(logger_kernel, "Desconexion de cpu - interrupt");
      control_key = 0;
      break;
   default:
      log_warning(logger_kernel, "Operacion desconocida de cpu - interrupt");
      break;
   }
}

}

void Kernel_atender_memoria(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_memoria);
   switch (cod_op)
   {
   case MENSAJE:
      //
      break;
   case PAQUETE:
      //
      break;
   case -1:
      log_error(logger_kernel, "Desconexion de memoria");
      control_key = 0;
      break;
   default:
      log_warning(logger_kernel, "Operacion desconocida de memoria");
      break;
   }
}

}

////////////////TENGO QUE ACOMODAR ESTA FUNCION////////////////////////

//Kernel le envia a memoria lo que pide para crear un proceso
void enviar_creacion_de_proceso_a_memoria(t_m_crear_proceso, int socket_memoria) {
    t_paquete* paquete_enviar_creacion_de_proceso = crear_paquete(CREAR_PROCESO_KERNEL);

    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &pcb->pid, sizeof(uint32_t));
    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &pcb->tamanio, sizeof(uint32_t));
    int nombre_len = strlen(pcb->archivo_pseudocodigo) + 1;
    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &nombre_len, sizeof(nombre_len));
    agregar_a_paquete(paquete_enviar_creacion_de_proceso, pcb->archivo_pseudocodigo, nombre_len);

    enviar_paquete(paquete_enviar_creacion_de_proceso, socket_memoria);

    printf("Se envió PCB\n");
    free(paquete_enviar_creacion_de_proceso);
}