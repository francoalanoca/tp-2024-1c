#include "../include/paginas.h"

t_list* lista_tablas_de_paginas = list_create();


//Funcion que crea un proceso a partir de un pid y el tamanio de un proceso
void crear_proceso(int proceso_pid, int tamanio_proceso)
{
    printf("Creacion del proceso PID %i - Tamanio %i", proceso_pid, tamanio_proceso);
    printf("Iniciando estructuras");

    //Guardo en una varia de tipo struct la tabla creada
    t_tabla_de_paginas *tabla_de_paginas = crear_tabla_pagina(proceso_pid, tamanio_proceso);
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, calcular_marcos(tamanio_proceso));

    list_add(lista_tablas_de_paginas, tabla_de_paginas);
}




//Funcion que a partir de un pid y tamaño de un proceso devuelve una tabla de paginas
t_tabla_de_paginas *crear_tabla_pagina(int pid, int tamanio_proceso){


    //Inicializo una variable tabla de pagina de tipo struct y le asigno un tamaño
    t_tabla_de_paginas *tabla_de_paginas = malloc(sizeof(t_tabla_de_paginas));

    //Le asigno valores a mi variable creada
    tabla_de_paginas->id = pid;
    tabla_de_paginas->lista_de_paginas = list_create();

    //Recorro mientras sea menor a la cantidad de frames
    for (int i = 0; i < calcular_marcos(tamanio_proceso); i++){

        t_pagina *pagina = malloc(sizeof(t_pagina));

        pagina->marco = -1;                 //No se le asigno un marco
        pagina->posicion = -1;              //No se le asigno una posicion
        pagina->presencia = false;          //No esta en memoria hasta ahora
        pagina->modificado = false;         //NO fue modificado
       
        list_add(tabla_de_paginas->lista_de_paginas, pagina);
    }

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



/*
//Falta implentar funciones de busqueda para que funcione


void escribir_memoria(int proceso_pid, int direccion_fisica, void* valor, int tamanio){
    int marco = direccion_fisica / cfg_memoria->TAM_PAGINA;

    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);

    t_pagina *pagina = busco_pagina_por_marco(tabla_de_paginas->lista_de_paginas, marco);

    memcpy(memoria + direccion_fisica, valor, tamanio);

    pagina->modificado = true;
    // actualizamos ultima referencia
    gettimeofday(&pagina->tiempo_ultima_referencia, NULL);
}



void* leer_memoria(int direccion_fisica, int tamanio){
    void* leido = malloc(tamanio);
    memcpy(leido, memoria + direccion_fisica, tamanio);
    return leido;
}

*/