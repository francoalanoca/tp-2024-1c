#include "consola.h"

 int identificador_pid;
 pthread_mutex_t mutex_pid;

//Funcion que implementa el inicio de la consola iterativa
void iniciar_consola_interactiva(int conexion){
    conexion_memoria = conexion;
    char* leido;
    //Asignamos el caracter > a la variable leido
	leido = readline("> ");
	log_info(logger_kernel, leido);
    bool validacion_leido;

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while (strcmp(leido, "\0") != 0){

        //paso lo leido a validar
        validacion_leido = validacion_de_instruccion_de_consola(leido);

        //Si lo validado no fue reconocido pasa por el if
        if (!validacion_leido){

            log_error(logger_kernel, "Comando de CONSOLA no reconocido");
            free(leido);
            leido = readline("> ");
            log_info(logger_kernel, leido);
            continue;   //salto el resto del while
        }
        
        //Una vez validado paso a atender lo leido
        atender_instruccion_validada(leido);
		free(leido);
        leido = readline("> ");
	}

	free(leido);
}


//FUncnion que evalua lo ingresado por consola y verifica si es alguno de los protocolos
bool validacion_de_instruccion_de_consola(char* leido){
    bool resultado_validacion = false;

    //Paso lo leido por consola a un vector para saber luego si es un comando de consola
    char** comando_consola = string_split(leido, "");

    //Comparo si lo leido coincide con alguno de los comandos
    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){
        resultado_validacion = true;
    }else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){
        resultado_validacion = true;
    }else{
        log_error(logger_kernel, "Comando no reconocido");
        resultado_validacion = false;
    }
    
    string_array_destroy(comando_consola);

    return resultado_validacion;
}



void atender_instruccion_validada(char* leido){
    char** comando_consola = string_split(leido, "");
    t_buffer* un_buffer = crear_buffer();

    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0){    //EJECUTAR_SCRIPT [PATH]
        cargar_string_al_buffer(un_buffer, comando_consola[1]);     //[PATH]

        //Procedo a ejecuratar el script

    } else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0) { //INICIAR_PROCESO [NOMBRE]
        if (comando_consola[1] == NULL) {
            fprintf(stderr, "Error: se debe proporcionar el nombre del proceso.\n");
            return;
        }
        cargar_string_al_buffer(un_buffer, comando_consola[1]); //[NOMBRE]

        // Procedo a iniciar el proceso
        f_iniciar_proceso(un_buffer);

    }else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){    //FINALIZAR_PROCESO [PID]
       
        pid_t pid = atoi(comando_consola[1]);
        if (kill(pid, SIGTERM) == 0) {
            printf("Proceso con PID %d finalizado exitosamente.\n", pid);
        } else {
            if (errno == ESRCH) {
                printf("No se encontró un proceso con PID %d.\n", pid);
            } else if (errno == EPERM) {
                printf("No tienes permiso para finalizar el proceso con PID %d.\n", pid);
            } else {
                printf("Ocurrió un error al finalizar el proceso con PID %d: %s\n", pid, strerror(errno));
            }
        }
       
    }else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){    //DETENER_PLANIFICACION
        
        detener_planificacion(planificador);

    }else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){    //INICIAR_PLANIFICACION
        
        inicializar_planificador(); //algoritmno, quantum?? ---> quiero preguntar.

    }else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){    //MULTIPROGRAMACION [VALOR]
        
        int valor = atoi(comando_consola[1]);
        ajustar_multiprogramacion(valor);
      
    }else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){   //PROCESO_ESTADO
        
        pid_t pid = atoi(comando_consola[1]);
        mostrar_estado_proceso(pid);
      
    }else{
        log_error(logger_kernel, "Comando no reconocido que logro pasar el filtro!!!");
        exit(EXIT_FAILURE);
    }
    
    string_array_destroy(comando_consola);
    
}

//Funcion que carga las instrucciones de path para iniciar un proceso en New
void f_iniciar_proceso(t_buffer* un_buffer) {
    char* path = extraer_string_del_buffer(un_buffer);  // Extraemos el path del buffer
    if (path == NULL) {
        fprintf(stderr, "Error: no se pudo extraer el path del buffer.\n");
        return;
    }

    char* nombre = extraer_string_del_buffer(un_buffer);  // Extraemos el nombre del buffer
    if (nombre == NULL) {
        fprintf(stderr, "Error: no se pudo extraer el nombre del buffer.\n");
        free(path);
        return;
    }

    log_trace(logger_kernel, "BUFFER: [PATH: %s, NOMBRE: %s]", path, nombre);
    eliminar_buffer(un_buffer);

    t_pcb* pcb = crear_pcb(path,nombre);
    if (pcb == NULL) {
        fprintf(stderr, "Error: no se pudo crear el PCB.\n");
        free(path);
        free(nombre);
        return;
    }


    //enviar_creacion_de_proceso_a_memoria(t_pcb* pcb, int conexion_memoria)

    //int cop = recibir_operacion(conexion_memoria);
    //if(cop==CREAR_PROCESO_FIN)

    // Cambiar el estado del PCB a ESTADO_READY
    if (agregar_proceso( planificador, pcb)){

    log_info(logger_kernel, "Proceso enviado a new");

    };

    imprimir_pcb(pcb);

    //pcb2 obtener_proximo_proceso(t_planificador* planificador)
    enviar_pcb_a_cpu_por_dispatch(pcb2); //declarar pcb2

    destruir_pcb(pcb);
    free(path);
    free(nombre);

}










t_pcb* crear_pcb(char* path, char* nombre) {
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    if (nuevo_pcb == NULL) {
        log_error(logger_kernel, "No se pudo asignar memoria para el nuevo PCB\n");
        return NULL;
    }

    pthread_mutex_lock(&mutex_process_id);
    process_id++;
    nuevo_pcb->pid = process_id;
    pthread_mutex_unlock(&mutex_process_id);

    nuevo_pcb->program_counter = 0;
    nuevo_pcb->path = strdup(path);  // Guardar el path del proceso
    nuevo_pcb->nombre_proceso = strdup(nombre);  // Guardar el nombre del proceso
    nuevo_pcb->lista_recursos_pcb = list_create(); // hacerlo en diccionario 
    nuevo_pcb->estado = ESTADO_NEW;

    if (pthread_mutex_init(&nuevo_pcb->mutex_lista_recursos, NULL) != 0) {
        log_error(logger_kernel, "No se pudo inicializar el mutex para el PCB\n");
        free(nuevo_pcb->path);
        free(nuevo_pcb->nombre_proceso);
        list_destroy(nuevo_pcb->lista_recursos_pcb);
        free(nuevo_pcb);
        return NULL;
    }

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

    printf("PCB creado exitosamente\n");
    return nuevo_pcb;
}

void imprimir_pcb(t_pcb* pcb) {
    log_info(logger_kernel, "<PCB_%d> [%s] [Nombre: %s] [Estado: %d]", pcb->pid, pcb->path, pcb->nombre_proceso, pcb->estado);
}

void enviar_pcb_a_cpu_por_dispatch(t_pcb* pcb) {

    // Cambiar el estado del PCB a ESTADO_EJECUTANDO antes de enviarlo a la CPU
    //cambiar_estado(pcb, ESTADO_RUNNING);


    t_paquete* un_paquete = crear_paquete(conexion_cpu_dispatch);
    agregar_a_paquete(un_paquete, pcb->pid, sizeof(uint32_t));
    agregar_a_paquete(un_paquete, pcb->program_counter, sizeof(uint32_t)); // cambiar a agregar_a_paquete y agregar sizeof.

    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.PC), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.AX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.BX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.CX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.DX), sizeof(uint8_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.EAX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.EBX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.ECX), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.SI), sizeof(uint32_t));
    agregar_a_paquete(un_paquete, &(pcb->registros_cpu.DI), sizeof(uint32_t));

    enviar_paquete(un_paquete, conexion_cpu_dispatch);
    eliminar_paquete(un_paquete);
}

void destruir_pcb(t_pcb* pcb) {
    if (pcb->path) free(pcb->path);
    if (pcb->nombre_proceso) free(pcb->nombre_proceso);
    list_destroy(pcb->lista_recursos_pcb);
    pthread_mutex_destroy(&pcb->mutex_lista_recursos);
    free(pcb);
}

void cambiar_estado(t_pcb* pcb, estado_pcb prox_estado) {
    pcb->estado = prox_estado;
}


void mostrar_estado_proceso(pid_t pid) {
    pthread_mutex_lock(&mutex_lista_procesos);

    // Verifica que lista_procesos esté inicializada y no sea NULL
    if (lista_procesos == NULL) {
        log_error(logger_kernel, "La lista de procesos no está inicializada.\n");
        pthread_mutex_unlock(&mutex_lista_procesos);
        return;
    }

    t_pcb* pcb = list_find_with_args(lista_procesos, encontrar_por_pid, &pid);
    
    if (pcb == NULL) {
        log_error(logger_kernel, "No se encontró el proceso con PID %d.\n", pid);
    } else {
        const char* estado_str;
        switch (pcb->estado) {
            case ESTADO_NEW:
                estado_str = "NUEVO";
                break;
            case ESTADO_READY:
                estado_str = "LISTO";
                break;
            case ESTADO_RUNNING:
                estado_str = "EJECUTANDO";
                break;
            case ESTADO_BLOCKED:
                estado_str = "BLOQUEADO";
                break;
            case ESTADO_EXIT:
                estado_str = "TERMINADO";
                break;
            default:
                estado_str = "DESCONOCIDO";
                break;
        }

        log_info(logger_kernel, "Estado del proceso con PID %d [%s]: %s\n", pid, pcb->nombre_proceso, estado_str);
    }

    pthread_mutex_unlock(&mutex_lista_procesos);
}

void ajustar_multiprogramacion(int nuevo_valor) {
    pthread_mutex_lock(&mutex_lista_procesos_listos);

    grado_multiprogramacion = nuevo_valor;
    log_info(logger_kernel, "Grado de multiprogramación ajustado a %d\n", nuevo_valor);

    // Aca se podría agregar lógica adicional para mover procesos entre estados
    // dependiendo del nuevo grado de multiprogramación.

    pthread_mutex_unlock(&mutex_lista_procesos_listos);
}

// Función para encontrar un PCB por PID
bool encontrar_por_pid(void* elemento, void* pid_ptr) {
    t_pcb* pcb = (t_pcb*)elemento;
    uint32_t pid = *(uint32_t*)pid_ptr;
    return pcb->pid == pid;