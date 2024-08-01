#include "../include/consola.h"

 int identificador_pid;
 pthread_mutex_t mutex_pid;
 pthread_mutex_t mutex_process_id = PTHREAD_MUTEX_INITIALIZER; // Definición

int process_id = 0; // Definición
 t_pcb* pcb2;


// Función que implementa el inicio de la consola interactiva
void iniciar_consola_interactiva(int conexion) {
    
    conexion_memoria = conexion;
    char* leido = malloc(sizeof(char) * 22);

 
    printf("INICIA CONSOLA\n");
    leido = readline("> ");

    bool validacion_leido;
    //add_history(leido);
    // El resto, las vamos leyendo y logueando hasta recibir un string vacío
    while (strcmp(leido, "\0") != 0) {

        // Dividir el comando en partes
        char** comando_consola = malloc(sizeof(char) * 22);
        comando_consola = string_split(leido, " ");

        validacion_leido = validacion_de_instruccion_de_consola(comando_consola);

        // Si lo validado no fue reconocido pasa por el if
        if (!validacion_leido) {
            log_error(logger_kernel, "Comando de CONSOLA no reconocido");
            string_array_destroy(comando_consola);
            free(leido);
            printf("INICIA CONSOLA\n");
            leido = readline("> ");
            log_info(logger_kernel, leido);
            continue;   // Salto el resto del while
        }

        // Una vez validado, paso a atender lo leído
        atender_instruccion_validada(comando_consola);
        string_array_destroy(comando_consola);
        free(leido);
        printf("INICIA CONSOLA\n");
        leido = readline("> ");
    }

    free(leido);
}



// Función que evalúa lo ingresado por consola y verifica si es alguno de los protocolos
bool validacion_de_instruccion_de_consola(char** comando_consola) {

    bool resultado_validacion = false;

    // Comparo si lo leído coincide con alguno de los comandos
    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0) {
        resultado_validacion = true;
    } else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0) {
        resultado_validacion = true;
    } else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0) {
        resultado_validacion = true;
    } else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0) {
        resultado_validacion = true;
    } else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0) {
        resultado_validacion = true;
    } else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0) {
        resultado_validacion = true;
    } else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0) {
        resultado_validacion = true;
    } else {
        log_error(logger_kernel, "Comando no reconocido");
        resultado_validacion = false;
    }

    return resultado_validacion;
}


void atender_instruccion_validada(char** comando_consola){
       //t_buffer* un_buffer = crear_buffer();

    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0) {    // EJECUTAR_SCRIPT [PATH]
        if (comando_consola[1] == NULL) {
            fprintf(stderr, "Error: se debe proporcionar el path del script.\n");
            return;
        }

        // Procedo a ejecutar el script
        f_ejecutar_script(comando_consola[1]);

    } else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0) { // INICIAR_PROCESO [NOMBRE]
    printf("ENTRE A INICIAR_PROCESO\n");
    printf("RUTA ARCHIVO: %s ",comando_consola[1]);
        if (comando_consola[1] == NULL) {
            fprintf(stderr, "Error: se debe proporcionar el nombre del proceso.\n");
            return;
        }

        // Procedo a iniciar el proceso
        f_iniciar_proceso(comando_consola[1]);  
    
    }else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){    //FINALIZAR_PROCESO [PID]
       printf("ME METI AL FINALIZAR_PROCESO\n");
        uint32_t pid = atoi(comando_consola[1]);
       printf("OBTENGO PID: %u\n",pid);
        mandar_proceso_a_finalizar(pid);
        log_info(logger_kernel, "Finaliza el proceso %u - Motivo: INTERRUPTED_BY_USER ", pid);

       
    }else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){    //DETENER_PLANIFICACION
        if(!planificador->planificacion_detenida){
            detener_planificacion(planificador);
        }
        else{
            printf("LA PLANIFICACION YA ESTA DETENIDA\n");
        }
        
     

    }else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){    //INICIAR_PLANIFICACION
        printf("ENTRE A INICIAR_PLANIFICACION\n");
        //algortimo = obtener_algoritmo_planificador(cfg_kernel->ALGORITMO_PLANIFICACION);
       // planificador = inicializar_planificador(algortimo, cfg_kernel->QUANTUM, cfg_kernel->GRADO_MULTIPROGRAMACION ); 
        if(planificador->planificacion_detenida){
            printf("ENTRE A IF INICIAR_PLANIFICACION\n");
            sem_post(&sem_planificar);
        // Hilo para mantener la ejecución andando y no deneter la consola}

        }
        else{
            printf("LA PLANIFICACION YA ESTA INICIADA\n");
        }
        

    }else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){    //MULTIPROGRAMACION [VALOR]
        
        int valor = atoi(comando_consola[1]);
   
        ajustar_multiprogramacion(valor, planificador);
      
      
    }else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){   //PROCESO_ESTADO
        
       // pid_t pid = atoi(comando_consola[1]);
       
         mostrar_estado_proceso(planificador);
      
      
    }else{
        log_error(logger_kernel, "Comando no reconocido que logro pasar el filtro!!!");
        exit(EXIT_FAILURE);
    }
    
    
}

//Funcion que carga las instrucciones de path para iniciar un proceso en New
void f_iniciar_proceso(char* path) {


    t_pcb* pcb = crear_pcb(path);
    if (pcb == NULL) {
        fprintf(stderr, "Error: no se pudo crear el PCB.\n");
        free(path);
        return;
    }


    enviar_creacion_de_proceso_a_memoria(pcb,conexion_memoria);
    printf("ENTRO A WAIT SEM CREACION PROCESO\n");

    //ESTO DESCOMENTARLO PERO PONER UN SEM WAIT(SEM POST EN EL CASE DE PROTOCOLO KERNEL DE CREAR_PROCESO_KERNEL_FIN)
    sem_wait(&sem_rta_crear_proceso);
    // Cambiar el estado del PCB a ESTADO_READY
    if (agregar_proceso( planificador, pcb)){

    printf("PROCESO AGREAGADO\n");

    };


}










t_pcb* crear_pcb(char* path) {
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    if (nuevo_pcb == NULL) {
        log_error(logger_kernel, "No se pudo asignar memoria para el nuevo PCB\n");
        return NULL;
    }

    nuevo_pcb->pid = contador_pid;
    contador_pid+=1;
    nuevo_pcb->program_counter = 0;
    nuevo_pcb->path = strdup(path);  // Guardar el path del proceso
    printf("SE CREA NUEVO PCB CON PATH:%s, RECIBO%s\n",nuevo_pcb->path,path);
    nuevo_pcb->path_length = strlen(nuevo_pcb->path);
    nuevo_pcb->estado = ESTADO_NEW;

    // Inicializar registros CPU
    nuevo_pcb->registros_cpu.AX = 0;
    nuevo_pcb->registros_cpu.BX = 0;
    nuevo_pcb->registros_cpu.CX = 0;
    nuevo_pcb->registros_cpu.DX = 0;
    nuevo_pcb->registros_cpu.EAX = 0;
    nuevo_pcb->registros_cpu.EBX = 0;
    nuevo_pcb->registros_cpu.ECX = 0;
    nuevo_pcb->registros_cpu.EDX = 0;
    nuevo_pcb->registros_cpu.SI = 0;
    nuevo_pcb->registros_cpu.DI = 0;
    nuevo_pcb->registros_cpu.PC = nuevo_pcb->program_counter;
    nuevo_pcb->tiempo_ejecucion = 0;
    nuevo_pcb->quantum = NULL; 
    printf("PCB creado exitosamente\n");
    return nuevo_pcb;
}

void imprimir_pcb(t_pcb* pcb) {
    //log_info(logger_kernel, "<PCB_%d> [%s] [Nombre: %s] [Estado: %d]", pcb->pid, pcb->path, pcb->nombre_proceso, pcb->estado);
}

void enviar_pcb_a_cpu_por_dispatch(t_pcb* pcb) {

    // Cambiar el estado del PCB a ESTADO_EJECUTANDO antes de enviarlo a la CPU
    //cambiar_estado(pcb, ESTADO_RUNNING);


    t_paquete* un_paquete = crear_paquete(NUEVO_PROCESO);
    agregar_a_paquete(un_paquete, &pcb->pid, sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &pcb->program_counter, sizeof(uint32_t)); // cambiar a agregar_a_paquete y agregar sizeof.
    agregar_a_paquete(un_paquete, &(pcb->path_length), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, (pcb->path), pcb->path_length);
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.PC), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.AX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.BX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.CX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.DX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.EAX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.EBX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.ECX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.EDX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.SI), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.DI), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->estado), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->tiempo_ejecucion), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->quantum), sizeof(uint32_t));
    enviar_paquete(un_paquete, conexion_cpu_dispatch);
    eliminar_paquete(un_paquete);
}

void destruir_pcb(t_pcb* pcb) {
    if (pcb->path) free(pcb->path);
    //if (pcb->nombre_proceso) free(pcb->nombre_proceso);
    //list_destroy(pcb->lista_recursos_pcb);
   // pthread_mutex_destroy(&pcb->mutex_lista_recursos);

    free(pcb);
}

void cambiar_estado(t_pcb* pcb, estado_pcb prox_estado) {
    pcb->estado = prox_estado;
}


// Función para encontrar un PCB por PID
bool encontrar_por_pid(void* elemento, void* pid_ptr) {
    t_pcb* pcb = (t_pcb*)elemento;
    uint32_t pid = *(uint32_t*)pid_ptr;
    return pcb->pid == pid;
}



void f_ejecutar_script(char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        log_error(logger_kernel, "No se pudo abrir el archivo de script en la ruta especificada: %s", path);
        return;
    }

    char* linea = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&linea, &len, file)) != -1) {
        // Removemos el salto de línea
        if (linea[read - 1] == '\n') {
            linea[read - 1] = '\0';
        }

        log_info(logger_kernel, "Ejecutando comando: %s", linea);

        // Validamos y ejecutamos cada línea del archivo
        if (validacion_de_instruccion_de_consola(linea)) {
            atender_instruccion_validada(linea);
        } else {
            log_error(logger_kernel, "Comando no reconocido en el script: %s", linea);
        }
    }

    free(linea);
    fclose(file);
}

//Kernel le envia a memoria lo que pide para crear un proceso
void enviar_creacion_de_proceso_a_memoria(t_pcb* pcb, int conexion_memoria) {
    t_paquete* paquete_enviar_creacion_de_proceso = crear_paquete(CREAR_PROCESO_KERNEL);

    agregar_a_paquete(paquete_enviar_creacion_de_proceso, &pcb->pid, sizeof(uint32_t));
    uint32_t path_length = strlen(pcb->path) + 1;
    //agregar_a_paquete(paquete_enviar_creacion_de_proceso, &path_length, sizeof(uint32_t));
    agregar_a_paquete(paquete_enviar_creacion_de_proceso, pcb->path, path_length);
    printf("PID: %d", pcb->pid);
    printf("PATH: %s", pcb->path);
    printf("PATH LENGTH: %d", path_length);
    enviar_paquete(paquete_enviar_creacion_de_proceso, conexion_memoria);

    printf("Se envió PCB\n");
    liberar_memoria_paquete(paquete_enviar_creacion_de_proceso);
}

void ajustar_multiprogramacion(int valor, t_planificador* planificador){
    planificador->grado_multiprogramacion = valor;
}

void mostrar_lista_procesos(t_list* lista, const char* nombre_cola) {

    printf("%s: ", nombre_cola);
    for(int i = 0; i < list_size(lista); i++) {
        t_pcb* proceso = list_get(lista, i);
        printf("%d", proceso->pid);
        if (i < list_size(lista) - 1) {
            printf(", ");
        }
    }
    printf("\n");
}
 // Mostrar los procesos bloqueados por interfaz
void mostrar_procesos_bloqueados(char* key, void* value) {
    t_list* lista_bloqueados = (t_list*)value;
    printf("Bloqueado %s: ", key);
    for (int i = 0; i < list_size(lista_bloqueados); i++) {
        t_proceso_data* proceso = list_get(lista_bloqueados, i);
        printf("%d", proceso->pcb->pid);
        if (i < list_size(lista_bloqueados) - 1) {
            printf(", ");
        }
    }
    printf("\n");
}
void mostrar_estado_proceso(t_planificador *planificador) {  
    
    mostrar_lista_procesos(planificador->cola_new, "New");
    mostrar_lista_procesos(planificador->cola_ready, "Ready");
    
    if (planificador->algoritmo == VIRTUAL_ROUND_ROBIN) {
        mostrar_lista_procesos(planificador->cola_ready_prioridad, "Ready prioridad");
    }
    
    mostrar_lista_procesos(planificador->cola_exec, "Ejecutando");
    
    dictionary_iterator(planificador->cola_blocked, mostrar_procesos_bloqueados);
    
    mostrar_lista_procesos(planificador->cola_exit, "Exit");
}