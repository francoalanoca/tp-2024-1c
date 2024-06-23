#include "../include/paginas.h"




//Funcion que crea la tabla de paginas a partir de un pid
void crear_proceso(int proceso_pid){

    printf("Creacion del proceso PID %i", proceso_pid);
    printf("Iniciando estructuras");

    //Guardo en una varia de tipo struct la tabla creada
    t_tabla_de_paginas *tabla_de_paginas = crear_tabla_pagina(proceso_pid);
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, sizeof(tabla_de_paginas));

    list_add(lista_tablas_de_paginas, tabla_de_paginas);
}




//Funcion que a partir de un pid devuelve una tabla de paginas
t_tabla_de_paginas *crear_tabla_pagina(int pid){


    //Inicializo una variable tabla de pagina de tipo struct y le asigno un tamaño
    t_tabla_de_paginas *tabla_de_paginas = malloc(sizeof(t_tabla_de_paginas));

    //Le asigno valores a mi variable creada
    tabla_de_paginas->id = pid;
    tabla_de_paginas->lista_de_paginas = list_create();


    return tabla_de_paginas;
}



//Funcion que a partir del tamaño del proceso devuelve la cantidad de frames
int calcular_marcos(int tamanio_proceso){

    //Calculamos la cantidad de marcos
    int cantidad_de_marcos = tamanio_proceso / cfg_memoria->TAM_PAGINA;

    //Si el resto es distinto a cero la cantidad de marcos la incrementamos en 1
    if (tamanio_proceso % cfg_memoria->TAM_PAGINA != 0)
        cantidad_de_marcos++;
    return cantidad_de_marcos;
}





//Funcion que dado un id de proceso buscamos la tabla de pagina y la retorna
t_tabla_de_paginas *busco_tabla_de_paginas_por_PID(int proceso_pid){

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
t_pagina *busco_pagina_por_marco(t_list *lista_de_paginas, int marco){

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



void escribir_memoria(int proceso_pid, int direccion_fisica, void* valor, int tamanio){
    int marco = direccion_fisica / cfg_memoria->TAM_PAGINA;

    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    t_pagina *pagina = busco_pagina_por_marco(tabla_de_paginas->lista_de_paginas, marco);

    memcpy(memoria + direccion_fisica, valor, tamanio);

    //Actualizamos que fue modificado
    pagina->modificado = true;
}



void* leer_memoria(int direccion_fisica, int tamanio){
    void* leido = malloc(tamanio);
    memcpy(leido, memoria + direccion_fisica, tamanio);
    return leido;
}





int buscar_marco_pagina(int proceso_pid, int numero_de_pagina){

    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    t_pagina *pagina = list_get(tabla_de_paginas->lista_de_paginas, numero_de_pagina);

    log_debug(logger_memoria, "ACCESO TABLA PAGINAS");
    log_info(logger_memoria, "PID: %d - Página: %d - Marco: %d", proceso_pid, numero_de_pagina, pagina->marco);

    if (pagina->presencia){
        return pagina->marco;
    }

    return -1;
}



char* administrar_resize(uint32_t proceso_pid, uint32_t tamanio_proceso){


    //Reservo una cantidad de marcos por tamaño de proceso
    int marcos_a_reservar = calcular_marcos(tamanio_proceso);

    //Recorro mientras sea menor a la cantidad de frames
    for (int i = 0; i < marcos_a_reservar; i++){

        t_pagina *pagina = malloc(sizeof(t_pagina));

        pagina->marco = -1;                 //No se le asigno un marco
        pagina->posicion = -1;              //No se le asigno una posicion
        pagina->presencia = false;          //No esta en memoria hasta ahora
        pagina->modificado = false;         //NO fue modificado
       
        list_add(tabla_de_paginas->lista_de_paginas, pagina);
    }


    //Si el tamaño es 0
    //if(tamanio_proceso == 0)

    //SI el tamaño es menor al frame
    //if(tamanio_proceso < )
    //Si el tamaño es mayor al frame (Page faul)

}





/*
//Puede que falte detallar mas de cerrar del todo el proceso
//Funcion que en base al id de un proceso finalizamos sus estructuras
void finalizar_proceso(int proceso_pid){

    log_trace(logger_memoria, "Liberacion del proceso PID %i", proceso_pid);
    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    //Recorremos la lista de paginas
    for (int j = 0; j < list_size(tabla_de_paginas->lista_de_paginas); j++){

        //Sacamos la pagina de la lista
        t_pagina *pagina = list_get(tabla_de_paginas->lista_de_paginas, j);

        //Si la pagina se encuentra presente la limpiamos
        if (pagina->presencia){

            pagina->presencia = false;
            bitarray_clean_bit(bitmap_frames, pagina->marco);
        }
    }
    
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, list_size(tabla_de_paginas->lista_de_paginas));
}
*/