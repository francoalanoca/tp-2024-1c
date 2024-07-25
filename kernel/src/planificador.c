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
printf("ENTRO agregar_proceso\n");
    list_add(planificador->cola_new, proceso);
    printf("Agrego a cola new\n");
    if (planificador->grado_multiprogramacion_actual <= planificador->grado_multiprogramacion) {
        printf("ENTRARE A sem_prioridad_io\n");
       // sem_wait(&sem_prioridad_io); DESCOMENTAR
        t_pcb* proceso_nuevo = list_remove(planificador->cola_new, 0);
        list_add(planificador->cola_ready, proceso_nuevo); // REVISAR
        planificador->grado_multiprogramacion_actual++;
    }
    return true;
}

// Obtiene el próximo proceso a ejecutar
t_pcb* obtener_proximo_proceso(t_planificador* planificador) {
    t_pcb* proceso;
    if (planificador->algoritmo != VIRTUAL_ROUND_ROBIN) {
        proceso = list_remove(planificador->cola_ready, 0);        
    } else { // Virtual Round Robin
       if (list_size(planificador->cola_ready_prioridad) > 0) {
            proceso = list_remove(planificador->cola_ready_prioridad, 0); 
        }else {
            proceso = list_remove(planificador->cola_ready, 0); 
        } 
    }   

    return proceso;
}


// Desaloja un proceso de la cola de ejecución y lo pone en la cola de listos
void desalojar_proceso(t_planificador* planificador, t_pcb* proceso) {
    list_remove(planificador->cola_exec, proceso);
    list_add(planificador->cola_ready, proceso);
}

//Bloquea un proceso y lo mueve a la cola de bloqueados
void bloquear_proceso(t_planificador* planificador, t_proceso_data* proceso_data, char* nombre_lista) {
    list_remove(planificador->cola_exec, proceso_data->pcb);
    dictionary_put(planificador->cola_blocked,nombre_lista,proceso_data);
    sem_post(&sem_cpu_libre);
}

//  Desbloquea un proceso y lo mueve a la cola de listos
void desbloquear_proceso(t_planificador* planificador, t_pcb* proceso, char* nombre_lista) {
    t_list* lista_a_desbloquear = malloc(sizeof(t_list));
    lista_a_desbloquear = dictionary_remove(planificador->cola_blocked,nombre_lista);
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
    pid_string = sprintf(pid_string, "%u", proceso->pid);
    t_proceso_recurso_diccionario* proceso_recurso = malloc(sizeof(t_proceso_recurso_diccionario));
    proceso_recurso = dictionary_get(procesos_recursos,pid_string);
    for (size_t i = 0; i < proceso_recurso->nombres_recursos->elements_count; i++)
    {
        //buscar en que indice de la tabla general de recursos esta
        uint32_t indice_recurso_buscado = buscar_indice_recurso(cfg_kernel->RECURSOS,list_get(proceso_recurso->nombres_recursos,i)); 
        
        //sumo cant instancias correspondientes a lista de instancias global del recurso correspondiente
        uint32_t instancias_generales_actuales = malloc(sizeof(uint32_t));
        uint32_t instancias_proceso = malloc(sizeof(uint32_t));
        instancias_generales_actuales = list_get(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_buscado);
        instancias_proceso = list_get(proceso_recurso->instancias_recursos,i);
        list_replace(cfg_kernel->INSTANCIAS_RECURSOS,indice_recurso_buscado,instancias_generales_actuales + instancias_proceso);
    }
    
    //borrar valor del diccionario para pid correspondiente
    dictionary_remove_and_destroy(procesos_recursos,pid_string,free);//TODO: reemplazar free por funcion que borre la esttructura y las listas que lo componen
    free(proceso);
     planificador->grado_multiprogramacion_actual--;
    if (!list_is_empty(planificador->cola_new) && !planificador->planificacion_detenida) {
        t_pcb* proceso_nuevo = list_remove(planificador->cola_new, 0);
        list_add(planificador->cola_ready, proceso_nuevo);
        planificador->grado_multiprogramacion_actual++;
    }
}

// Crea un nuevo proceso
void crear_proceso(t_planificador* planificador, char* path_pseudocodigo) {
    t_pcb* nuevo_proceso = crear_pcb(path_pseudocodigo);
    agregar_proceso(planificador, nuevo_proceso);
    // Notificar a la memoria para crear el proceso
}

void eliminar_proceso(t_planificador* planificador, t_pcb* proceso) {
    if (list_contains(planificador->cola_exec, proceso->pid)) {
        enviar_interrupcion_a_cpu(proceso,ELIMINAR_PROCESO,conexion_cpu_interrupt);
        
        // Esperar a que la CPU retorne el Contexto de Ejecución
        sem_wait(&sem_contexto_ejecucion_recibido);

        proceso = pcb_actualizado_interrupcion;
    }

   liberar_proceso_memoria(proceso->pid);

    // Esperar confirmación de la memoria
    sem_wait(&sem_confirmacion_memoria);

    // Finalizar el proceso en el planificador
    finalizar_proceso(planificador, proceso);
    log_info(logger_kernel, "PID: %u - Estado Anterior: EJECUTANDO - Estado Actual: EXIT", proceso->pid);
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

void enviar_interrupcion_a_cpu(t_pcb* proceso,motivo_interrupcion motivo_interrupcion, int conexion){
    // Enviar señal de interrupción a la CPU
        t_paquete* paquete = crear_paquete(INTERRUPCION_KERNEL);

        agregar_a_paquete(paquete, &(proceso->pid), sizeof(uint32_t));
        agregar_a_paquete(paquete, &motivo_interrupcion, sizeof(uint32_t));

        enviar_paquete(paquete, conexion); 

        free(paquete); 

}

void liberar_proceso_memoria(uint32_t pid){
     // Notificar a la memoria para liberar las estructuras del proceso
    t_paquete* paquete_memoria = crear_paquete(FINALIZAR_PROCESO);

    // Serializar el PID del proceso a liberar
    agregar_a_paquete(paquete_memoria, &pid, sizeof(uint32_t));

    // Enviar el paquete a la memoria
    enviar_paquete(paquete_memoria, conexion_memoria);
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
    uint32_t indice_proceso_a_finalizar = malloc(sizeof(uint32_t));
    indice_proceso_a_finalizar = encontrar_indice_proceso_pid(planificador->cola_exec,proceso);
    list_remove(planificador->cola_exec, indice_proceso_a_finalizar);
    list_add(planificador->cola_exit, proceso);
}

void enviar_proceso_a_cpu(t_pcb* pcb, int conexion){

   t_paquete* paquete_archivo_nuevo = malloc(sizeof(t_paquete));
    
    paquete_archivo_nuevo = crear_paquete(NUEVO_PROCESO);
    
    agregar_a_paquete(paquete_archivo_nuevo, &(pcb->pid), sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &(pcb->program_counter), sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &(pcb->path_length), sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, (pcb->path), pcb->path_length);
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.AX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.BX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.CX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_archivo_nuevo, &pcb->registros_cpu.DX, sizeof(uint32_t)); 
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
    log_info(logger_kernel, "llamo enviar_proceso_a_cpu"); //despues borrar

}

void planificar_y_ejecutar(){
   log_info(logger_kernel, "PLANIFICAR Y EJECUTAR"); //despues borrar
    while (1){
 
        int procesos_ready;
        if (planificador->algoritmo != VIRTUAL_ROUND_ROBIN) {
            procesos_ready = list_size(planificador->cola_ready); 
        }else {
         procesos_ready = list_size(planificador->cola_ready_prioridad) +list_size(planificador->cola_ready);
        }
       
        if (procesos_ready > 0  && !planificador->planificacion_detenida) {  
            log_info(logger_kernel, "hay un proceso en ready"); //despues borrar
            t_pcb* siguiente_proceso;// = malloc(sizeof(t_pcb));      
            if (planificador->algoritmo == FIFO) { 
                siguiente_proceso = obtener_proximo_proceso(planificador);
                log_info(logger_kernel, "Proximo pid a enviar: %d",siguiente_proceso->pid);
                enviar_proceso_a_cpu(siguiente_proceso,conexion_cpu_dispatch);
                log_info(logger_kernel, "Proceso enviado pid: %d",siguiente_proceso->pid);            
                list_add(planificador->cola_exec,siguiente_proceso); 
                log_info(logger_kernel, "Proceso agregado a lista de ejeucion pid: %d",siguiente_proceso->pid);             
                //free(siguiente_proceso); 
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
	
	free(args);
}


void lanzar_interrupcion_fin_quantum (void* args) {
    t_args_fin_q* args_fin_q = (t_args_fin_q*) args;
    int quantum = args_fin_q->quantum;
    int pid = args_fin_q->pid;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    uint32_t motivo = FIN_QUANTUM;
   
    sleep(quantum / 1000);   
    paquete = crear_paquete(INTERRUPCION_KERNEL); 
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
    agregar_a_paquete(paquete, &motivo, sizeof(uint32_t));   
    enviar_paquete(paquete, conexion_cpu_interrupt);  
    log_info(logger_kernel, "Enviando interrupcion FIN de QUANTUM\n");
    free(paquete);
}

void actualizar_quantum(t_pcb* proceso){ // recibo contexto actualizado desde cpu TODO: Cambiar nombre
    
    temporal_stop(cronometro); // mover a la interrupcion de IO y tambien de recurso     
      
    proceso->tiempo_ejecucion = temporal_gettime(cronometro);
	
	proceso->quantum = cfg_kernel->QUANTUM - proceso->tiempo_ejecucion; // actualizo el nuevo quantum restante
}

void largo_plazo_nuevo_ready() {
   log_info(logger_kernel, "PLANIFICADOR LARGO PLAZO INICIADO"); //Despues borrar
   
    while (1) {

         if (list_size(planificador->cola_new) > 0  && !planificador->planificacion_detenida) {
             log_info(logger_kernel, "hay un proceso en new"); //despues borrar
          t_pcb* proceso_nuevo = malloc(sizeof(t_pcb));  
            if (planificador->grado_multiprogramacion_actual < planificador->grado_multiprogramacion) {
                proceso_nuevo = list_remove(planificador->cola_new, 0);
                list_add(planificador->cola_ready, proceso_nuevo);
                planificador->grado_multiprogramacion_actual++;
                log_info(logger_kernel, "PID: %d - Estado Anterior: NEW - Estado Actual: READY",pcb->pid);
            }
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

void mandar_proceso_a_finalizar(uint32_t pid){
    printf("ME METI AL mandar_proceso_a_finalizar\n");
   t_pcb* pcb_a_procesar = malloc(sizeof(t_pcb));
   pcb_a_procesar = encontrar_proceso_pid(planificador->cola_exec,pid);
   printf("ENCONTRE PCB A PROCESAR\n");
   eliminar_proceso(planificador,pcb_a_procesar);
   printf("ELIMINE PROCESO\n");
   liberar_memoria_pcb(pcb_a_procesar);
   printf("LIBERE MEMORIA PCB\n");
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