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

    }else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0){  //INICIAR_PROCESO [PATH]
        cargar_string_al_buffer(un_buffer, comando_consola[1]);     //[PATH]
        //cargar_string_al_buffer(un_buffer, comando_consola[2]);   //Si el protocolo tuviera un 2do parametro [SIZE] se descomenta esto
        //cargar_string_al_buffer(un_buffer, comando_consola[3]);   //Si el protocolo tuviera un 3er parametro [PRIORIDAD] se descomenta esto

        //Procedo a iniciar el proceso
        f_iniciar_proceso(un_buffer);

    }else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0){    //FINALIZAR_PROCESO [PID]
       
       /* pid_t pid = atoi(comando_consola[1]);
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
       */
    }else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0){    //DETENER_PLANIFICACION
        
        //detener_planificacion();

    }else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0){    //INICIAR_PLANIFICACION
        
        //iniciar_planificacion();

    }else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0){    //MULTIPROGRAMACION [VALOR]
        
      /*int valor = atoi(comando_consola[1]);
        ajustar_multiprogramacion(valor);
      */
    }else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0){   //PROCESO_ESTADO
        
      /*pid_t pid = atoi(comando_consola[1]);
        mostrar_estado_proceso(pid);
      */
    }else{
        log_error(logger_kernel, "Comando no reconocido que logro pasar el filtro!!!");
        exit(EXIT_FAILURE);
    }
    
    string_array_destroy(comando_consola);
    
}

//Funcion que carga las instrucciones de path para iniciar un proceso en New
void f_iniciar_proceso(t_buffer* un_buffer) {
    char* path = extraer_string_del_buffer(un_buffer);  // Extraemos del buffer
    if (path == NULL) {
        fprintf(stderr, "Error: no se pudo extraer el path del buffer.\n");
        return;
    }
    log_trace(logger_kernel, "BUFFER (%d): [PATH: %s]", path);
    eliminar_buffer(un_buffer);

    t_pcb* pcb = crear_pcb(path);
    if (pcb == NULL) {
        fprintf(stderr, "Error: no se pudo crear el PCB.\n");
        free(path);
        return;
    }

    imprimir_pcb(pcb);
    enviar_pcb_a_cpu_por_dispatch(pcb);
    destruir_pcb(pcb);
    free(path);
}

t_pcb* crear_pcb(char* path) {
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    if (nuevo_pcb == NULL) {
        log_error(logger_kernel, "No se pudo asignar memoria para el nuevo pcb\n");
        return NULL;
    }

    pthread_mutex_lock(&mutex_process_id);
    process_id++;
    nuevo_pcb->pid = process_id;
    pthread_mutex_unlock(&mutex_process_id);

    nuevo_pcb->program_counter = 0;
    nuevo_pcb->path = strdup(path);
    nuevo_pcb->lista_recursos_pcb = list_create();

    if (pthread_mutex_init(&nuevo_pcb->mutex_lista_recursos, NULL) != 0) {
        log_error(logger_kernel, "No se pudo inicializar el mutex para el pcb\n");
        free(nuevo_pcb->path);
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

void imprimir_pcb(t_pcb* un_pcb) {
    log_info(logger_kernel, "<PCB_%d> [%s]", un_pcb->pid, un_pcb->path);
}

void enviar_pcb_a_cpu_por_dispatch(t_pcb* una_pcb) {
    t_paquete* un_paquete = crear_super_paquete(conexion_cpu_dispatch);
    cargar_int_al_super_paquete(un_paquete, una_pcb->pid);
    cargar_int_al_super_paquete(un_paquete, una_pcb->program_counter);

    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.PC), sizeof(uint32_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.AX), sizeof(uint8_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.BX), sizeof(uint8_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.CX), sizeof(uint8_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.DX), sizeof(uint8_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.EAX), sizeof(uint32_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.EBX), sizeof(uint32_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.ECX), sizeof(uint32_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.SI), sizeof(uint32_t));
    cargar_milanesa_al_super_paquete(un_paquete, &(una_pcb->registros_cpu.DI), sizeof(uint32_t));

    enviar_paquete(un_paquete, conexion_cpu_dispatch);
    eliminar_paquete(un_paquete);
}

void destruir_pcb(t_pcb* un_pcb) {
    if (un_pcb->path) free(un_pcb->path);
    list_destroy(un_pcb->lista_recursos_pcb);
    pthread_mutex_destroy(&un_pcb->mutex_lista_recursos);
    free(un_pcb);
}

void cambiar_estado(t_pcb* un_pcb, estado_pcb prox_estado) {
    un_pcb->estado = prox_estado;
}