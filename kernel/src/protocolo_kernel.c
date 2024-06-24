#include <protocolo_kernel.h>



//Funcion que crea hilos para cada modulo y los va atendiendo

void Escuchar_Msj_De_Conexiones(){

//Escuchar los msj de memoria
   pthread_t hilo_kernel_memoria;
   pthread_create(&hilo_kernel_memoria, NULL, (void*)Kernel_escuchar_memoria, NULL);
   pthread_detach(hilo_kernel_memoria);

//Escuchar los msj de cpu - dispatch
   pthread_t hilo_cpu_dispatch;
   pthread_create(&hilo_cpu_dispatch, NULL, (void*)Kernel_escuchar_cpu_dispatch, NULL);
   pthread_detach(hilo_cpu_dispatch);

//Escuchar los msj de cpu - interrupt
   pthread_t hilo_cpu_interrupt;
   pthread_create(&hilo_cpu_interrupt, NULL, (void*)Kernel_escuchar_cpu_interrupt, NULL);
   pthread_join(hilo_cpu_interrupt, NULL);

}


void Kernel_escuchar_cpu_dispatch(){

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

void Kernel_escuchar_cpu_interrupt(){

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

void Kernel_escuchar_memoria(){

bool control_key = 1;
while (control_key)
{
   int cod_op = recibir_operacion(conexion_memoria);
   switch (cod_op)
   {
   case CREAR_PROCESO_KERNEL:
      enviar_creacion_de_proceso_a_memoria(pcb,conexion_memoria);
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




//Kernel le envia a memoria lo que pide para crear un proceso
void enviar_creacion_de_proceso_a_memoria(t_pcb* pcb, int conexion_memoria) {
    t_paquete* paquete_enviar_creacion_de_proceso = crear_paquete(CREAR_PROCESO_KERNEL);

    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &pcb->pid, sizeof(uint32_t));
    int nombre_len = strlen(pcb->nombre_proceso) + 1;
    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &nombre_len, sizeof(nombre_len));
    agregar_a_paquete(paquete_enviar_creacion_de_proceso, pcb->nombre_proceso, nombre_len);

    enviar_paquete(paquete_enviar_creacion_de_proceso, conexion_memoria);

    printf("Se envió PCB\n");
    free(paquete_enviar_creacion_de_proceso);
}