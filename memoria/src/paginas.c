#include "../include/paginas.h"




//Funcion que crea la tabla de paginas a partir de un pid
void crear_proceso(uint32_t proceso_pid){

    log_info(logger_memoria, "Creacion del proceso PID %i \n", proceso_pid);
    log_info(logger_memoria, "Iniciando estructuras \n");

    //Guardo en una varia de tipo struct la tabla creada
    t_tabla_de_paginas *tabla_de_paginas = crear_tabla_pagina(proceso_pid);
    log_info(logger_memoria, "Creada tabla de paginas \n");
    list_add(lista_tablas_de_paginas, tabla_de_paginas);
}




//Funcion que a partir de un pid devuelve una tabla de paginas
t_tabla_de_paginas *crear_tabla_pagina(uint32_t pid){


    //Inicializo una variable tabla de pagina de tipo struct y le asigno un tamaño
    t_tabla_de_paginas *tabla_de_paginas = malloc(sizeof(t_tabla_de_paginas));

    //Le asigno valores a mi variable creada
    tabla_de_paginas->id = pid;
    tabla_de_paginas->lista_de_paginas = list_create();
    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_info(logger_memoria, "Creacion de tabla de paginas: \n");
    log_info(logger_memoria, "PID: %i - Tamaño: %d", pid, 0);

    return tabla_de_paginas;
}



//Funcion que a partir del tamaño del proceso devuelve la cantidad de frames
uint32_t calcular_marcos(uint32_t tamanio_proceso){

    //Calculamos la cantidad de marcos
    uint32_t cantidad_de_marcos = floor(tamanio_proceso / cfg_memoria->TAM_PAGINA);

    //Si el resto es distinto a cero la cantidad de marcos la incrementamos en 1
    if (tamanio_proceso % cfg_memoria->TAM_PAGINA != 0)
        cantidad_de_marcos++;
    return cantidad_de_marcos;
}





//Funcion que dado un id de proceso buscamos la tabla de pagina y la retorna
t_tabla_de_paginas *busco_tabla_de_paginas_por_PID(uint32_t proceso_pid){

    log_trace(logger_memoria, "Buscando la Tabla de Paginas por PID");

    t_tabla_de_paginas *tabla_de_paginas;

    //Recorremos la lista que contiene tabla de paginas
    for (int i = 0; i < list_size(lista_tablas_de_paginas); i++){

        //Sacamos una tabla de paginas de la lista
        tabla_de_paginas = list_get(lista_tablas_de_paginas, i);
        //Si el id de la tabla es el mismo que la tabla, la retorna
        if (proceso_pid == tabla_de_paginas->id)
            return tabla_de_paginas;
    }

    log_error(logger_memoria, "PID - %d No se encontro la Tabla de Paginas", proceso_pid);
    abort();
}


//Funcion que dado un marco, busco en la lista de paginas y lo retorna
t_pagina *busco_pagina_por_marco(t_list *lista_de_paginas, uint32_t marco){

    log_trace(logger_memoria, "Buscando Pagina por Marco: %i", marco);

    t_pagina *pagina;

    //Recorremos la lista de paginas perteneciente a la tabla
    for (int i = 0; i < list_size(lista_de_paginas); i++){

        //Sacamos una pagina de la lista
        pagina = list_get(lista_de_paginas, i);
        //Si el marco de la pagina es le mismo que buscaba lo retorna
        if (marco == pagina->marco) {
            log_trace(logger_memoria, "Encontre pagina: %i", i);
            return pagina;
        }
    }
    log_trace(logger_memoria, "No se encontro pagina");
    return NULL;
}



uint32_t obtener_numero_pagina(t_list* lista_de_paginas, uint32_t marco_buscado){

    //FUncion auxiliar para find
    bool condicion(void* pag){
        
        t_pagina* pagina = (t_pagina*) pag;
        return pagina->marco == marco_buscado;
    }

    //Aqui hago la busqueda de la pagina
    t_pagina* pagina_buscada = (t_pagina*) list_find(lista_de_paginas, &condicion);

    return pagina_buscada->posicion;
}



//Funcion que escribe una valor con un tamaño, de un proceso con la df
char* escribir_memoria(uint32_t proceso_pid, uint32_t direccion_fisica, char* valor, uint32_t tamanio_a_escribir){

    char* escrito;

    //Aca se almacenara lo que ya fue escrito como un contador
    uint32_t espacio_escrito = 0;
    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    //SI hay algo para escribir
    if (tamanio_a_escribir > 0){   
    
        //Creamos las variables necesarias para la algrebra de punteros (frame|offset)
        uint32_t marco = floor(direccion_fisica / cfg_memoria->TAM_PAGINA);
        uint32_t offset = direccion_fisica % cfg_memoria->TAM_PAGINA;

        //numero de pagina asociado a tal marco
        uint32_t numero_pagina_actual = obtener_numero_pagina(tabla_de_paginas->lista_de_paginas, marco);

        //El espacio disponible en el marco va ser el tamaño de este menos lo que se desplaza el proceso
        uint32_t espacio_restante_marco = cfg_memoria->TAM_PAGINA - offset;

        
        //SI no es 0 y es <= al espacio del frame(marco)
        if (tamanio_a_escribir <= espacio_restante_marco){
            
            //Procedemos a copiar (donde se va guardar, que se va guardar(desde donde), tamanio de lo que se va a guardar)
            memcpy(memoria + direccion_fisica, valor, tamanio_a_escribir);

            escrito = "OK";
        }
        //Si es > ocupamos lo que quede y luego buscamos otro espacio
        else{
        
            //Este memcopy utiliza el restante del frame
            memcpy(memoria + direccion_fisica, valor, espacio_restante_marco);  

            //Actualizo variables luego de escribir
            espacio_escrito = espacio_restante_marco;
            tamanio_a_escribir = tamanio_a_escribir - espacio_escrito;

            //Calculo cantidad de marcos en base al tamanio restante
            uint32_t cant_marcos_a_escribir = calcular_marcos(tamanio_a_escribir);

            //Recorro los marcos estimados para el tamanio a escribir
            for (int i = 0; i < cant_marcos_a_escribir; i++){

                //Busco la pagina siguiente y actualizo df
                numero_pagina_actual +=1;
                t_pagina* pagina = list_get(tabla_de_paginas->lista_de_paginas, numero_pagina_actual);
                direccion_fisica = pagina->marco * cfg_memoria->TAM_PAGINA;
                
                //SI el tamanio sigue siendo mayor a la pagina
                if(tamanio_a_escribir > cfg_memoria->TAM_PAGINA){
                    
                    //Este memcopy utiliza todo el frame
                    memcpy(memoria + direccion_fisica, valor + espacio_escrito, cfg_memoria->TAM_PAGINA);
                    
                    //Actualizo variables luego de escribir (luego hace for de nuevo)
                    tamanio_a_escribir -= cfg_memoria->TAM_PAGINA;
                    espacio_escrito += cfg_memoria->TAM_MEMORIA;
                }
                else{ //Si es <= escribo y envio respuesta
                    memcpy(memoria + direccion_fisica, valor + espacio_escrito, tamanio_a_escribir);
                    espacio_escrito += tamanio_a_escribir;

                    escrito = "OK";
                }
               
            }
            
        }

    
        
    }
    
    return escrito;
}





//Funcion que lee algo de una tamaño en base a una df y un proceso(pid)
char* leer_memoria(uint32_t proceso_pid, uint32_t direccion_fisica, uint32_t tamanio_a_leer){

    //Reservo espacio para la variable que voy a devolver
    char* leido = (char*) malloc(tamanio_a_leer);

    //Aca se almacenara lo que ya fue leido como un contador
    uint32_t espacio_leido = 0;

    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    //Mientras haya algo para leer
    if (tamanio_a_leer > 0){
        
        //Creamos las variables necesarias para la algrebra de punteros (frame|offset)
        uint32_t marco = floor(direccion_fisica / cfg_memoria->TAM_PAGINA);
        uint32_t offset = direccion_fisica % cfg_memoria->TAM_PAGINA;

        uint32_t numero_pagina_actual = obtener_numero_pagina(tabla_de_paginas->lista_de_paginas, marco);

        //El espacio disponible en el marco va ser el tamaño de este menos lo que se desplaza el proceso
        uint32_t espacio_restante_marco = cfg_memoria->TAM_PAGINA - offset;

    
        //SI no es 0 y < al espacio del frame(marco)
        if (tamanio_a_leer <= espacio_restante_marco){

            //Procedemos a copiar (donde se va guardar, que se va guardar(desde donde), tamanio de lo que se va a guardar)
            memcpy(leido, memoria + direccion_fisica, tamanio_a_leer);

            return leido;
        }
        else{   //Si es > ocupamos lo que quede y luego buscamos otro espacio

            //Este memcopy utiliza el restante del frame
            memcpy(leido, memoria + direccion_fisica, espacio_restante_marco);

            //Actualizo variables luego de leer
            espacio_leido = espacio_restante_marco;
            tamanio_a_leer = tamanio_a_leer - espacio_leido;

            //Calculo cantidad de marcos en base al tamanio restante
            uint32_t cant_marcos_a_leer = calcular_marcos(tamanio_a_leer);

            //Recorro los marcos estimados para el tamanio a leer
            for (int i = 0; i < cant_marcos_a_leer; i++){
                
                //Busco la pagina siguiente y actualizo df
                numero_pagina_actual +=1;
                t_pagina* pagina = list_get(tabla_de_paginas->lista_de_paginas, numero_pagina_actual);
                direccion_fisica = pagina->marco * cfg_memoria->TAM_PAGINA;

                //SI el tamanio sigue siendo mayor a la pagina
                if (tamanio_a_leer > cfg_memoria->TAM_PAGINA){

                    //Este memcopy utiliza todo el frame
                    memcpy(leido + espacio_leido, memoria + direccion_fisica, cfg_memoria->TAM_PAGINA);

                    //Actualizo variables luego de leer (luego hace for de nuevo)
                    tamanio_a_leer -= cfg_memoria->TAM_PAGINA;
                    espacio_leido += cfg_memoria->TAM_PAGINA;
                }
                else{   //Si es <= lee y envio respuesta
                    memcpy(leido + espacio_leido, memoria + direccion_fisica, tamanio_a_leer);
                    espacio_leido += tamanio_a_leer;

                    return leido;
                }

            }
            

        }

        
    }
    
    return leido;
}




char* copiar_solicitud(uint32_t proceso_pid, uint32_t direccion_fisica, char* valor){

    char* copiado;

    copiado = escribir_memoria(proceso_pid, direccion_fisica, valor, strlen(valor));

    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_info(logger_memoria, "Acceso a espacio de usuario: \n");
    log_info(logger_memoria, "PID: %d - Acción: ESCRIBIR - Direccion fisica: %d - Tamaño: %ld", proceso_pid, direccion_fisica, strlen(valor));
    return copiado;
}



uint32_t buscar_marco_pagina(uint32_t proceso_pid, uint32_t numero_de_pagina){

    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    t_pagina *pagina = list_get(tabla_de_paginas->lista_de_paginas, numero_de_pagina);
    
    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_debug(logger_memoria, "ACCESO TABLA PAGINAS: \n");
    log_info(logger_memoria, "PID: %d - Página: %d - Marco: %d", proceso_pid, numero_de_pagina, pagina->marco);


    return pagina->marco;
}




uint32_t obtener_marco_libre(){
    //Recorro la memoria para marcar los frames libres
    for (uint32_t i = 0; i < cantidad_frames_memoria; i++){
        //Si la posicion del array esta disponible
        if (!bitarray_test_bit(bitmap_frames, i)){
             log_info(logger_memoria, "marco libre : %d",i);
            return i;
        }
    }
    log_trace(logger_memoria, "No se obtuvo un marco libre");
    return -1;
}



op_code administrar_resize(uint32_t proceso_pid, uint32_t tamanio_proceso){

    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    //Calculo el tamanio del proceso luego de ser encontrado
    uint32_t tamanio_actual = list_size(tabla_de_paginas->lista_de_paginas) * cfg_memoria->TAM_PAGINA;
    int ultima_pagina = list_size(tabla_de_paginas->lista_de_paginas);
    uint32_t marcos_a_reservar = calcular_marcos(tamanio_proceso);

  

    if(tamanio_proceso == 0){
        return SOLICITUD_RESIZE_RTA;

    }else{      // Ampliación del proceso

        //Si el tamanio es ingresado es mayor al actual, ampliamos
        if (tamanio_proceso > tamanio_actual) { 

            //SI el tamanio ingresaso es menos al disponible
            if(tamanio_proceso <= espacio_disponible()){

                //Recorro desde la ultima pagina mientras sea menor a la cantidad de frames recein calculados
                for (int i = ultima_pagina; i < marcos_a_reservar; i++){

                    t_pagina *pagina = malloc(sizeof(t_pagina));
                    int marco_libre = obtener_marco_libre();
                    pagina->marco = marco_libre;                 
                    pagina->posicion = ultima_pagina+i;                    
                    bitarray_set_bit(bitmap_frames, marco_libre);
                    list_add(tabla_de_paginas->lista_de_paginas, pagina);
                    
                }

                log_trace(logger_memoria, "Log Obligatorio: \n");
                log_info(logger_memoria, "Ampliación de Proceso: \n");
                log_info(logger_memoria, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d", proceso_pid, tamanio_actual, tamanio_proceso);
                return SOLICITUD_RESIZE_RTA;
            
            }else{
                log_info(logger_memoria, "OUT OF MEMORY");
                return OUT_OF_MEMORY;
            }


        }else if (tamanio_proceso < tamanio_actual) {  // Reducción del proceso

            int marcos_a_liberar = list_size(tabla_de_paginas->lista_de_paginas) - marcos_a_reservar;

            //Recorro los marcos sobrantes y los libero
            for (int i = 0; i < marcos_a_liberar; i++) {

                t_pagina *pagina = list_remove(tabla_de_paginas->lista_de_paginas, list_size(tabla_de_paginas->lista_de_paginas) - 1);
                bitarray_clean_bit(bitmap_frames, pagina->marco);
                free(pagina);
            }

            log_trace(logger_memoria, "Log Obligatorio: \n");
            log_info(logger_memoria, "Reducción de Proceso: \n");
            log_info(logger_memoria, "PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d", proceso_pid, tamanio_actual, tamanio_proceso);
            return SOLICITUD_RESIZE_RTA;

        } else {  // No hay cambio en el tamaño
            return SOLICITUD_RESIZE_RTA;
        }
        

    }  
}


int espacio_disponible(){
    int frames_libres = 0;   

    for (uint32_t i = 0; i < bitarray_get_max_bit(bitmap_frames); i++){
    //Si la posicion del array esta disponible
        if (!bitarray_test_bit(bitmap_frames, i)){
            frames_libres++ ;
        }
    }
    log_info(logger_memoria, "Espacio disponible %d",frames_libres*cfg_memoria->TAM_PAGINA);
    return frames_libres*cfg_memoria->TAM_PAGINA;
}



t_miniPCB *busco_un_proceso_PID(uint32_t proceso_pid){

    log_trace(logger_memoria, "Buscando la lista de instrucciones por PID");

    t_miniPCB *proceso;

    //Recorremos la lista que contiene la lista de procesos
    for (int i = 0; i < list_size(lista_miniPCBs); i++){

        //Sacamos una tabla de paginas de la lista
        proceso = list_get(lista_miniPCBs, i);
        //Si el id del proceso es el mismo que el proceso, la retorna
        if (proceso_pid == proceso->pid)
            return proceso;
    }

    log_error(logger_memoria, "PID - %d No se encontro la lista de instrucciones", proceso_pid);
    abort();
}




//Funcion que en base al id de un proceso finalizamos sus estructuras
void finalizar_proceso(uint32_t proceso_pid){

    log_trace(logger_memoria, "Liberacion del proceso PID %i", proceso_pid);
    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);
    t_miniPCB *proceso = busco_un_proceso_PID(proceso_pid);

    //Recorremos la lista de paginas
    for (int j = 0; j < list_size(tabla_de_paginas->lista_de_paginas); j++){

        //Sacamos la pagina de la lista
        t_pagina *pagina = list_get(tabla_de_paginas->lista_de_paginas, j);

        //Marco el bit como disponible
        bitarray_clean_bit(bitmap_frames, pagina->marco);
        
    }
    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_info(logger_memoria, "Destruccion de tabla de paginas: \n");
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, list_size(tabla_de_paginas->lista_de_paginas));

    list_destroy_and_destroy_elements(tabla_de_paginas->lista_de_paginas, free);
    list_destroy_and_destroy_elements(proceso->lista_de_instrucciones, free);
    free(tabla_de_paginas);
    list_remove_element(lista_miniPCBs, proceso);
}
