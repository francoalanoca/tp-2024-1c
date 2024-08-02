#include "../include/planificador.h"
//t_planificador* planificador;
sem_t sem_planificar;
t_temporal* cronometro;
//sem_t sem_contexto_ejecucion_recibido;
//sem_t sem_confirmacion_memoria;


// Devuelve un t_algoritmo a partir de la config cargada
t_algoritmo_planificacion obtener_algoritmo_planificador(char* algoritmo_planificacion) {
    if (strcmp(algoritmo_planificacion, "FIFO") == 0) {
        return FIFO;
    } else if (strcmp(algoritmo_planificacion, "ROUND ROBIN") == 0) {
        return ROUND_ROBIN;
    } else if (strcmp(algoritmo_planificacion, "VIRTUAL ROUND ROBIN") == 0) {
        return VIRTUAL_ROUND_ROBIN;
    } else {
        // Manejo de error para tipos de algoritmos desconocidos
        return -1;
    }
}

// detiene tanto el planificador de corto plazo como el de largo plazo
void detener_planificacion(t_planificador* planificador) {
    sem_wait(&sem_planificar);
    planificador->planificacion_detenida = true;
}

//Inicializa un nuevo planificador

t_planificador* inicializar_planificador(t_algoritmo_planificacion algoritmo, int quantum,int grado_multiprogramacion) {

    planificador = malloc(sizeof(t_planificador));
    printf("creo planificador\n");
    planificador->cola_new = list_create();
    planificador->cola_ready = list_create();
    planificador->cola_ready_prioridad = list_create(); // la creo igual 
    planificador->cola_exec = list_create();
     printf("creo cola_exec\n");
    planificador->cola_blocked = dictionary_create();
    printf("creo cola_blocked\n");
    planificador->cola_exit = list_create();
    planificador->algoritmo = algoritmo;
    planificador->quantum = quantum;
    planificador->grado_multiprogramacion = grado_multiprogramacion;
    planificador->grado_multiprogramacion_actual = 0;
    planificador->planificacion_detenida = false; // Inicializar planificación como no detenida
    printf("creo planificacion_detenida\n");
    crear_listas_recursos();
    printf("creo crear_listas_recursos\n");
    return planificador;
}

// Destruye el planificador y libera la memoria
void destruir_planificador(t_planificador* planificador) {
    list_destroy_and_destroy_elements(planificador->cola_new, free);
    list_destroy_and_destroy_elements(planificador->cola_ready, free);
    list_destroy_and_destroy_elements(planificador->cola_exec, free);
    list_destroy_and_destroy_elements(planificador->cola_blocked, free);
    list_destroy_and_destroy_elements(planificador->cola_exit, free);
    free(planificador);
}

// Agrega un nuevo proceso al planificador
bool agregar_proceso(t_planificador* planificador, t_pcb* proceso) {

    list_add(planificador->cola_new, proceso);
    log_info(logger_kernel,"Se crea el proceso %d en NEW",proceso->pid);
    if (planificador->grado_multiprogramacion_actual <= planificador->grado_multiprogramacion) {
        printf("ENTRARE A sem_prioridad_io\n");
       // sem_wait(&sem_prioridad_io); DESCOMENTAR
        //t_pcb* proceso_nuevo = list_remove(planificador->cola_new, 0);
       // list_add(planificador->cola_ready, proceso_nuevo); // REVISAR
       // planificador->grado_multiprogramacion_actual++;
    }
    return true;
}

// Obtiene el próximo proceso a ejecutar
t_pcb* obtener_proximo_proceso(t_planificador* planificador) {
    t_pcb* proceso;
    if (planificador->algoritmo != VIRTUAL_ROUND_ROBIN) {
         pthread_mutex_lock(&mutex_cola_ready);
        proceso = list_remove(planificador->cola_ready, 0);  
         pthread_mutex_unlock(&mutex_cola_ready);      
    } else { // Virtual Round Robin
       if (list_size(planificador->cola_ready_prioridad) > 0) {
            pthread_mutex_lock(&mutex_cola_ready_prioridad);
            proceso = list_remove(planificador->cola_ready_prioridad, 0);
             pthread_mutex_unlock(&mutex_cola_ready_prioridad); 
        }else {
             pthread_mutex_lock(&mutex_cola_ready);
            proceso = list_remove(planificador->cola_ready, 0); 
             pthread_mutex_unlock(&mutex_cola_ready);
        } 
    }   

    return proceso;
}


// Desaloja un proceso de la cola de ejecución y lo pone en la cola de listos
void desalojar_proceso(t_planificador* planificador, t_pcb* proceso) {
    
    list_remove(planificador->cola_exec, buscar_indice_pcb_por_pid(planificador->cola_exec,proceso->pid));
    list_add(planificador->cola_ready, proceso);
}

//Bloquea un proceso y lo mueve a la cola de bloqueados
void bloquear_proceso(t_planificador* planificador, t_proceso_data* proceso_data, char* nombre_lista) {
    pthread_mutex_lock(&mutex_cola_exec); 
    list_remove(planificador->cola_exec,buscar_indice_pcb_por_pid(planificador->cola_exec,proceso_data->pcb->pid) );
    pthread_mutex_unlock(&mutex_cola_exec);
    t_list* bloqueados = dictionary_get(planificador->cola_blocked,nombre_lista);
    list_add(bloqueados,proceso_data);
    dictionary_put(planificador->cola_blocked,nombre_lista,bloqueados); // bnloquea el que lo envuelve por ahora
    sem_post(&sem_cpu_libre);
}

//  Desbloquea un proceso y lo mueve a la cola de listos
void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso, char* nombre_lista) {
    t_list* lista_a_desbloquear = malloc(sizeof(t_list));
    lista_a_desbloquear = dictionary_get(planificador->cola_blocked,nombre_lista);
    uint32_t indice_a_desbloquear = encontrar_indice_proceso_data_pid(lista_a_desbloquear,proceso);
    list_remove_and_destroy_element(lista_a_desbloquear,indice_a_desbloquear,list_destroy);
    dictionary_put(planificador->cola_blocked,nombre_lista,lista_a_desbloquear);
    if (!planificador->planificacion_detenida) {
        list_add(planificador->cola_ready, proceso);
    }
}

// Finaliza un proceso y libera su memoria
void finalizar_proceso(t_planificador* planificador, t_pcb* proceso) {
    //Liberar los recursos del proceso
    char* pid_string = malloc(sizeof(proceso->pid));
     sprintf(pid_string, "%d", proceso->pid); //itoa(proceso->pid, pid_string, 10);
     log_info(logger_kernel, "PID: %s -proceso a liberar recursos",pid_string);
    t_proceso_recurso_diccionario* proceso_recurso  = dictionary_get(procesos_recursos,pid_string);
    if (proceso_recurso != NULL && !list_is_empty(proceso_recurso->nombres_recursos)) {
            for (int i = 0; i < list_size(proceso_recurso->nombres_recursos); i++)
            {
                //buscar en que indice de la tabla general de recursos esta
                uint32_t indice_recurso_buscado = buscar_indice_recurso(cfg_kernel->RECURSOS,list_get(proceso_recurso->nombres_recursos,i)); 
                
                //sumo cant instancias correspondientes a lista de instancias global del recurso correspondiente
                uint32_t instancias_generales_actuales ;
                uint32_t instancias_proceso ;
                instancias_generales_actuales = list_get(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_buscado);
                instancias_proceso = list_get(proceso_recurso->instancias_recursos,i);
                list_replace(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_buscado,instancias_generales_actuales + instancias_proceso);
            }
            
            //borrar valor del diccionario para pid correspondiente
            dictionary_remove_and_destroy(procesos_recursos,pid_string,free);//TODO: reemplazar free por funcion que borre la esttructura y las listas que lo componen
     }       
    log_info(logger_kernel, "PID: %s -PROCESO LIBERADO",pid_string);
    liberar_memoria_pcb(proceso);
    planificador->grado_multiprogramacion_actual--;
    sem_post(&sem_cpu_libre);

}

// Crea un nuevo proceso
void crear_proceso(t_planificador* planificador, char* path_pseudocodigo) {
    t_pcb* nuevo_proceso = crear_pcb(path_pseudocodigo);
    agregar_proceso(planificador, nuevo_proceso);
    // Notificar a la memoria para crear el proceso
}

void eliminar_proceso(t_planificador* planificador, t_pcb* proceso) {
  
    liberar_proceso_memoria(proceso->pid);
    // Esperar confirmación de la memoria
    sem_wait(&sem_confirmacion_memoria);
    // Finalizar el proceso en el planificador
    finalizar_proceso(planificador, proceso);
    
}

uint32_t encontrar_indice_proceso_pid(t_list * lista_procesos , t_pcb* pcb) {
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_pcb* proceso = list_get(lista_procesos, i);
        if (proceso->pid == pcb->pid) {
            return i;
        }
    }
    return NULL;
}

uint32_t encontrar_indice_proceso_data_pid(t_list * lista_procesos_data , t_pcb* pcb) {
    for (int i = 0; i < list_size(lista_procesos_data); i++) {
        t_proceso_data* proceso = list_get(lista_procesos_data, i);
        if (proceso->pcb->pid == pcb->pid) {
            return i;
        }
    }
    return NULL;
}

void enviar_interrupcion_a_cpu(int pid, motivo_interrupcion motivo_interrupcion,char* nombre_interface, int conexion){
    // Enviar señal de interrupción a la CPU
   
        t_paquete* paquete = crear_paquete(INTERRUPCION_KERNEL);
        uint32_t size_nombre_interfaz = strlen(nombre_interface)+1;
        agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
        agregar_a_paquete(paquete, &motivo_interrupcion, sizeof(uint32_t));
        agregar_a_paquete(paquete, &size_nombre_interfaz, sizeof(uint32_t));
        agregar_a_paquete(paquete, nombre_interface, size_nombre_interfaz);
        enviar_paquete(paquete, conexion); 
        eliminar_paquete(paquete); 
        log_info(logger_kernel,"Envío interrupcion a CPU %s", nombre_interface);

}


void liberar_proceso_memoria(uint32_t pid){
     // Notificar a la memoria para liberar las estructuras del proceso
    t_paquete* paquete_memoria = crear_paquete(FINALIZAR_PROCESO);
    // Serializar el PID del proceso a liberar
    agregar_a_paquete(paquete_memoria, &pid, sizeof(uint32_t));

    // Enviar el paquete a la memoria
    enviar_paquete(paquete_memoria, conexion_memoria);
    eliminar_paquete(paquete_memoria);
}

bool list_contains(t_list* lista_de_procesos, uint32_t pid){

    t_pcb* pcb_a_recorrer = malloc(sizeof(t_pcb));

    for (size_t i = 0; i < lista_de_procesos->elements_count; i++)
    {
        pcb_a_recorrer = list_get(lista_de_procesos,i);
        if(pcb_a_recorrer->pid == pid){
            return true;
        }
    }
    
    return false;
}

void poner_en_cola_exit(t_pcb* proceso){
    uint32_t indice_proceso_a_finalizar = encontrar_indice_proceso_pid(planificador->cola_exec,proceso);
    log_info(logger_kernel, "entro en poner A COLA EXIT"); //despues borra
    pthread_mutex_lock(&mutex_cola_exec);
    list_remove(planificador->cola_exec, indice_proceso_a_finalizar);
    pthread_mutex_unlock(&mutex_cola_exec);
    
    pthread_mutex_lock(&mutex_cola_exit);
    list_add(planificador->cola_exit, proceso);
    pthread_mutex_unlock(&mutex_cola_exit);
     log_info(logger_kernel, "PROCESO AGREGADO A COLA EXIT"); //despues borra
}

void enviar_proceso_a_cpu(t_pcb* pcb, int conexion){

   t_paquete* paquete_archivo_nuevo = crear_paquete(NUEVO_PROCESO);
    
    agregar_a_paquete(paquete_archivo_nuevo, &(pcb->pid), sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &(pcb->program_counter), sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &(pcb->path_length), sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, (pcb->path), pcb->path_length);
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.AX, sizeof(uint8_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.BX, sizeof(uint8_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.CX, sizeof(uint8_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.DX, sizeof(uint8_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.EAX, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.EBX, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.ECX, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.EDX, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.SI, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.DI, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->estado, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->tiempo_ejecucion, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->quantum, sizeof(uint32_t));
   
    enviar_paquete(paquete_archivo_nuevo, conexion); 

    eliminar_paquete(paquete_archivo_nuevo);


}

void planificar_y_ejecutar(){
   log_info(logger_kernel, "PLANIFICAR Y EJECUTAR"); //despues borrar
    while (1){
 
        int procesos_ready;
        bool cola_exec_vacia = list_size(planificador->cola_exec) == 0;
        if (planificador->algoritmo != VIRTUAL_ROUND_ROBIN) {
            procesos_ready = list_size(planificador->cola_ready); 
        }else {
         procesos_ready = list_size(planificador->cola_ready_prioridad) +list_size(planificador->cola_ready);
        }
       
        if (procesos_ready > 0  && !planificador->planificacion_detenida && cola_exec_vacia) {  
           
            t_pcb* siguiente_proceso;    
            if (planificador->algoritmo == FIFO) { 
                siguiente_proceso = obtener_proximo_proceso(planificador);               
                enviar_proceso_a_cpu(siguiente_proceso,conexion_cpu_dispatch);                          
                list_add(planificador->cola_exec,siguiente_proceso);                             
                log_info(logger_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EJECUTANDO",siguiente_proceso->pid); // LOG OBLIGATORIO
            }else {
                siguiente_proceso = obtener_proximo_proceso(planificador);
                ejecutar_modo_round_robin(siguiente_proceso); 
                //free(siguiente_proceso); 

            }
        }    
    }
}

void  ejecutar_modo_round_robin( t_pcb* proceso){
    int quantum_restante;
    pthread_t hilo_cronometro;
    t_args_fin_q* args = malloc(sizeof(t_args_fin_q));
     log_info(logger_kernel, "EJECUTAR MODO ROUND ROBIN\n");
    if (planificador->algoritmo == ROUND_ROBIN) {
		 quantum_restante = cfg_kernel->QUANTUM;
	}else {
		quantum_restante = proceso->quantum; // si ejecuta sin interrupciones entonces  proceso->quantum = cfg_kernel->QUANTUM
	}
	enviar_proceso_a_cpu(proceso,conexion_cpu_dispatch);
   
    args->quantum = quantum_restante;
    args->pid = proceso->pid;


	cronometro = temporal_create();	
    //creamos hilo para no tener espera activa del cronometro      
    if (pthread_create(&hilo_cronometro, NULL, lanzar_interrupcion_fin_quantum, (void*)args) != 0) {
        perror("pthread_create_hilo_cronometro");
        free(hilo_cronometro);       
        
    }
     pthread_detach(hilo_cronometro);
	
  }	


void lanzar_interrupcion_fin_quantum (void* args) {
    t_args_fin_q* args_fin_q = (t_args_fin_q*) args;
    int quantum = args_fin_q->quantum;
    int pid = args_fin_q->pid;
    t_paquete* paquete;
    uint32_t motivo = FIN_QUANTUM;
   
    sleep(quantum / 1000);   
    paquete = crear_paquete(INTERRUPCION_KERNEL); 
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
    agregar_a_paquete(paquete, &motivo, sizeof(uint32_t));   
    enviar_paquete(paquete, conexion_cpu_interrupt);  
    log_info(logger_kernel, "Enviando interrupcion FIN de QUANTUM\n");
     eliminar_paquete(paquete);
     free(args);
}

void actualizar_quantum(t_pcb* proceso){ // recibo contexto actualizado desde cpu TODO: Cambiar nombre
    
    temporal_stop(cronometro); // mover a la interrupcion de IO y tambien de recurso     
      
    proceso->tiempo_ejecucion = temporal_gettime(cronometro);
	
	proceso->quantum = cfg_kernel->QUANTUM - proceso->tiempo_ejecucion; // actualizo el nuevo quantum restante
}

void largo_plazo() {
   log_info(logger_kernel, "PLANIFICADOR LARGO PLAZO INICIADO"); //Despues borrar
   
    while (1) {

         if (list_size(planificador->cola_new) > 0  && !planificador->planificacion_detenida) {
            
          t_pcb* proceso_nuevo ;
            if (planificador->grado_multiprogramacion_actual < planificador->grado_multiprogramacion) {
                proceso_nuevo = list_remove(planificador->cola_new, 0);
                list_add(planificador->cola_ready, proceso_nuevo);
                planificador->grado_multiprogramacion_actual++;
                log_info(logger_kernel, "PID: %d - Estado Anterior: NEW - Estado Actual: READY",proceso_nuevo->pid); // LOG OBLIGATORIO
            }
        }

        if (list_size(planificador->cola_exit) > 0  && !planificador->planificacion_detenida){
            
            sem_wait(&sem_contexto_ejecucion_recibido);
            pthread_mutex_lock(&mutex_cola_exit);
            t_pcb* proceso_exit = list_remove(planificador->cola_exit,0);
             pthread_mutex_unlock(&mutex_cola_exit);
             log_info(logger_kernel, "PID: %d - proceso a finalizar",proceso_exit->pid); // LOG OBLIGATORIO
            mandar_proceso_a_finalizar(proceso_exit);
            
        }


    }
}

void crear_listas_recursos(){

recursos = malloc(sizeof(t_list));
recursos = char_array_to_list(cfg_kernel->RECURSOS);

printf("Creo lista recursos, tiene %d elementos, data:%s\n", recursos->elements_count,list_get(recursos,0));

for (size_t i = 0; i < recursos->elements_count; i++)
{
    dictionary_put(planificador->cola_blocked ,list_get(recursos,i),list_create());
} 

list_destroy_and_destroy_elements(recursos,free);

}

t_pcb* encontrar_proceso_pid(t_list * lista_procesos , uint32_t pid) {
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_pcb* proceso = list_get(lista_procesos, i);
        if (proceso->pid == pid) {
            return proceso;
        }
    }
    return NULL;
}

void mandar_proceso_a_finalizar(t_pcb* proceso_finalizar){
   printf("ME METI AL mandar_proceso_a_finalizar\n");
  

   eliminar_proceso(planificador,proceso_finalizar);
   
}

void mandar_proceso_a_finalizar_comando(int pid){
    printf("ME METI AL mandar_proceso_a_finalizar comando\n");
     pthread_mutex_lock(&mutex_cola_exec);
     t_pcb* proceso_finalizar = encontrar_proceso_pid(planificador->cola_exec, pid);
     pthread_mutex_unlock(&mutex_cola_exec);  
    if (proceso_finalizar == NULL) {
        printf("No encontre el proceso\n");
    } 
    else {
        eliminar_proceso(planificador,proceso_finalizar);

    } 
   
}

uint32_t buscar_indice_recurso(t_list* lista_recursos,char* nombre_recurso){
   uint32_t indice_encontrado = malloc(sizeof(uint32_t));
   indice_encontrado = NULL;

   for (size_t i = 0; i < lista_recursos->elements_count; i++)
   {
      if(strcmp(list_get(lista_recursos,i), nombre_recurso) == 0){
         indice_encontrado = i;
      }
   }
   return indice_encontrado;
}

int encontrar_indice_proceso_data_por_pid(t_list * lista_procesos_data , int pid ) {
    for (int i = 0; i < list_size(lista_procesos_data); i++) {
        t_proceso_data* proceso = list_get(lista_procesos_data, i);
        if (proceso->pcb->pid == pid) {
            return i;
        }
    }
    return NULL;
}
