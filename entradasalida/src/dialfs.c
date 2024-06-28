#include "../include/dialfs.h"

int block_count;
int bitmap_size_in_bytes ;
int fd_bitmap;
FILE* archivo_bitmap;
void* bitmap;
t_bitarray* bitarray;
char * path_archivo_bitmap ;
char * path_archivo_bloques;

void iniciar_interfaz_dialfs (int socket_kernel, int socket_memoria) {
    
    uint32_t response;
    op_code cop;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    t_list* lista_paquete =  malloc(sizeof(t_list));
    
    path_archivo_bitmap  = string_new();
    path_archivo_bloques = string_new();    

    string_append(&path_archivo_bitmap,cfg_entrada_salida->PATH_BASE_DIALFS); 
    string_append(&path_archivo_bitmap,"/bitmap.dat");
    log_info(logger_entrada_salida, "variables cargadas  %s",path_archivo_bitmap);
    string_append(&path_archivo_bloques,cfg_entrada_salida->PATH_BASE_DIALFS); 
    string_append(&path_archivo_bloques,"/bloques.dat");


    //BITMAP//
    if(crear_bitmap (path_archivo_bitmap)>=0 ) {
        log_info(logger_entrada_salida, "Bitmap creado correctamente");
    }
    else {
        log_info(logger_entrada_salida, "Error en creacion de bitmap");
        return EXIT_FAILURE;
    }

    // ARCHIVO DE BLOQUES//
    if(crear_archivo_bloques (path_archivo_bloques,cfg_entrada_salida->BLOCK_SIZE, cfg_entrada_salida->BLOCK_COUNT)>=0 ) {
        log_info(logger_entrada_salida, "Archivo de bloques iniciado correctamente");
    }
    else {
        log_info(logger_entrada_salida, "Error en inicio de Archivo de bloques");
        return EXIT_FAILURE;
    }

    cargar_directorio_fcbs(cfg_entrada_salida->PATH_BASE_DIALFS);

    log_info(logger_entrada_salida, "Interfaz %s de tipo DIALFS iniciada",cfg_entrada_salida->NOMBRE_INTERFAZ);  
    
    while (socket_kernel != -1) {

        if (recv(socket_kernel, &cop, sizeof(int32_t), MSG_WAITALL) != sizeof(int32_t)) {
            log_info(logger_entrada_salida, "DISCONNECT!");

            break;
        }
    switch (cop) {
            
            case HANDSHAKE :

                log_info(logger_entrada_salida, "Handshake realizado con Kernel");
                response = HANDSHAKE_OK;
                if (send(socket_kernel, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Error al enviar respuesta de handshake a kernel");
                    free(paquete);
                    break;
                }
                break;

            case HANDSHAKE_OK :

                log_info(logger_entrada_salida, "Handshake recibido handshake exitosamente con Kernel");
                
                break;                
            
            case IO_FS_CREATE :
                
                log_info(logger_entrada_salida, "IO_FS_CREATE recibida desde Kernel");
                    
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_gestion_archivo* archivo_nuevo = malloc(sizeof(t_io_gestion_archivo));
                archivo_nuevo = deserializar_fs_gestion (lista_paquete);
                crear_archivo(archivo_nuevo->nombre_archivo);
                list_clean(lista_paquete);
                free(archivo_nuevo);
                response = IO_K_GEN_SLEEP_FIN;

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                    break;
                }
                break;
            case IO_FS_DELETE :
                
                log_info(logger_entrada_salida, "IO_FS_DELETE recibida desde Kernel");
                    
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_gestion_archivo* archivo_borrar = malloc(sizeof(t_io_gestion_archivo));
                archivo_borrar = deserializar_fs_gestion (lista_paquete);
                borrar_archivo(archivo_borrar->nombre_archivo);
                list_clean(lista_paquete);
                free(archivo_borrar);
                response = IO_K_GEN_SLEEP_FIN;

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                    break;
                }
                break;  
            case IO_FS_TRUNCATE : 
                
                log_info(logger_entrada_salida, "IO_FS_TRUNCATE recibida desde Kernel");
                    
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_gestion_archivo* archivo_truncar = malloc(sizeof(t_io_gestion_archivo));
                archivo_truncar = deserializar_fs_gestion (lista_paquete);
                log_info(logger_entrada_salida, "IO_FS_TRUNCATE Tamaño solicitado %d",archivo_truncar->tamanio_archivo);
                truncar_archivo(archivo_truncar->nombre_archivo, archivo_truncar->tamanio_archivo);
                list_clean(lista_paquete);
                free(archivo_truncar);
                response = IO_K_GEN_SLEEP_FIN;

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                    break;
                }
                break;    

            case IO_FS_READ : 
                
                log_info(logger_entrada_salida, "IO_FS_READ recibida desde Kernel");
                    
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_readwrite_archivo* archivo_leer = malloc(sizeof(t_io_readwrite_archivo));
                archivo_leer = deserializar_io_readwrite (lista_paquete);
                leer_archivo(archivo_leer,socket_memoria);
                list_clean(lista_paquete);
                free(archivo_leer);
                response = IO_K_GEN_SLEEP_FIN;

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                    break;
                }
                break;  
           case IO_FS_WRITE : 
                
                log_info(logger_entrada_salida, "IO_FS_WRITE recibida desde Kernel");
                 
                t_io_direcciones_fisicas* solicitud_datos_escribir = malloc (sizeof(t_io_direcciones_fisicas));    
                lista_paquete = recibir_paquete(socket_kernel);
                t_io_readwrite_archivo* archivo_escribir = malloc(sizeof(t_io_readwrite_archivo));                
                archivo_escribir = deserializar_io_readwrite (lista_paquete);
                char* datos_escribir = malloc(archivo_escribir->tamanio_operacion * sizeof(char));
                solicitud_datos_escribir->pid = archivo_escribir->pid;
                solicitud_datos_escribir->direcciones_fisicas = list_create ();
                list_add(solicitud_datos_escribir->direcciones_fisicas,archivo_escribir->direcciones_fisicas);
                solicitud_datos_escribir->tamanio_operacion = archivo_escribir->tamanio_operacion;
               //reenvio la solicitud a memoria
                enviar_io_df(solicitud_datos_escribir, socket_memoria, IO_FS_WRITE);
               
                //Espero respuesta de memoria
                if (recv(socket_memoria, &cop, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                  log_info(logger_entrada_salida, "DISCONNECT!");
                 }

                 if (cop == IO_FS_WRITE_M) {
                    
                    t_list* lista_paquete_nueva = list_create();     
                    t_io_output* io_output_recibido = malloc(sizeof(t_io_output));
                    
                    lista_paquete_nueva = recibir_paquete(socket_memoria);                   
                    io_output_recibido = deserializar_output(lista_paquete_nueva);                 
                 
                    datos_escribir = io_output_recibido->output;
                    printf("Datos recibido para escribir:  %s \n",datos_escribir); // despues borrar

                    escribir_archivo(archivo_escribir,datos_escribir);
                    list_clean(lista_paquete);
                    free(archivo_escribir);
                    response = IO_K_GEN_SLEEP_FIN;

                    if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                        log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                        break;
                    }
                  }
                break;                                                                                            

            default:
                response = OPERACION_INVALIDA;
                if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, "Operacion invalida enviada desde kernel");
                    break;
                }
                break;
        }
    }
}

/////////////////////////////////////////////////INICIAR FS////////////////////////////////////


int crear_bitmap (char * path_archivo_bitmap) {
    block_count = cfg_entrada_salida->BLOCK_COUNT;
    bitmap_size_in_bytes = (block_count + 7) / 8; // 1 bit por bloque

    fd_bitmap = open(path_archivo_bitmap, O_RDWR | O_CREAT, 0666); // Abre archivo para escritura/lectura o lo crea


    archivo_bitmap = fopen(path_archivo_bitmap, "r+");  // Abre archivo para escritura/lectura
    if (archivo_bitmap == NULL) {
        archivo_bitmap = fopen(path_archivo_bitmap, "w+");  // Abre archivo para escritura/lectura
        log_info(logger_entrada_salida, "ARCHIVO DE BITMAP NO ENCONTRADO SE CREA UNO NUEVO");
    }
    fd_bitmap = fileno(archivo_bitmap);

    if (fd_bitmap == -1) {
        perror("Error al abrir o crear el archivo");
        return -1;
    }

    // levanto la info del file descriptor
    struct stat st;
    int stat_result = fstat(fd_bitmap, &st);

    if (stat_result == -1) {
        perror("Error al obtener información del archivo");
        close(fd_bitmap);
        return -1;
    }

    if (st.st_size == 0) {
        // El archivo está vacío, establece el tamaño del archivo
        log_info(logger_entrada_salida, "ESTABLECER EL TAMAÑO DEL BITMAP  : %d Bytes" ,bitmap_size_in_bytes);
        if (ftruncate(fd_bitmap, bitmap_size_in_bytes) == -1) {
            perror("Error al establecer el tamaño del archivo");
            close(fd_bitmap);
            return -1;
        }

        // Mapea el archivo a la memoria
         bitmap = mmap(NULL, block_count , PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);

        if (bitmap == MAP_FAILED) {
            perror("Error al mapear el archivo");
            close(fd_bitmap);
            return -1;
        }

        log_info(logger_entrada_salida, "BITMAP MAPEADO A MEMORIA ");
        // Inicializa el bitmap
        bitarray = bitarray_create_with_mode(bitmap, bitmap_size_in_bytes, LSB_FIRST);
        log_info(logger_entrada_salida, "BITMAP CARGADO EN BITARRAY con : %d bits", bitarray_get_max_bit(bitarray));
        // Marca el primer bloque como utilizado
        //bitarray_set_bit(bitarray, 0);

    } else {
        log_info(logger_entrada_salida, "EL ARCHIVO DEL BITMAP YA EXISTE ");
        // El archivo ya existe, mapea el archivo a la memoria y cargor el bitarray para manejarlo
        bitmap = mmap(NULL, block_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);


        if (bitmap == MAP_FAILED) {
            perror("Error al mapear el archivo");
            close(fd_bitmap);
            return -1;
        }

        // Inicializa el bitmap
        bitarray = bitarray_create(bitmap, bitmap_size_in_bytes);
        log_info(logger_entrada_salida, "BITMAP CARGADO EN BITARRAY ");
        //bitarray = bitarray_create_with_mode(bitmap, block_count, LSB_FIRST);

    }

    return 0;
}// fin crearbitmap

void cerrar_bitmap() {
    munmap(bitmap, bitmap_size_in_bytes);
    close(fd_bitmap);
}


FILE *archivo_bloques;
int fd_archivo_bloques;

int crear_archivo_bloques (char * path_archivo_bloques, int block_size, int block_count) {

    archivo_bloques = fopen(path_archivo_bloques, "r+");  // Abre archivo para escritura/lectura
    if (archivo_bloques == NULL) {
        archivo_bloques = fopen(path_archivo_bloques, "w+");  // Abre archivo para escritura/lectura
        log_info(logger_entrada_salida, "ARCHIVO DE BLOQUES NO ENCONTRADO SE CREA UNO NUEVO");
    }
    uint32_t file_size_in_bytes = block_size * block_count;

    fd_archivo_bloques = fileno(archivo_bloques);

    // calculo el tamaño actual del archivo
    fseek(archivo_bloques, 0, SEEK_END);
    long tamanio_actual = ftell(archivo_bloques);
    rewind(archivo_bloques); // posiciono el puntero al inicio del archivo

    if (tamanio_actual == 0) {
        // El archivo está vacío, establece el tamaño del archivo
        if (ftruncate(fd_archivo_bloques, file_size_in_bytes) == -1) {
            perror("Error al establecer el tamaño del archivo");
            close(fd_archivo_bloques);
            return -1;
        }
    }
    return 0;
}// fin cargar archivo de bloques

//////////////////////////////////////////////ESTRUCTURAS//////////////////////////////////////
 t_FCB* inicializar_fcb(char* nombre_archivo, uint32_t tamanio_archivo, uint32_t primer_bloque) {
    t_FCB* fcb = malloc(sizeof(t_FCB));
    if (!fcb) {
        perror("Error al inicializar el FCB");
        exit(EXIT_FAILURE);
    }
    memset(fcb,0, sizeof (t_FCB));

    fcb->nombre_archivo = strdup(nombre_archivo);
    fcb->tamanio_archivo = tamanio_archivo;
    fcb->primer_bloque = primer_bloque;
    return fcb;
}

t_FCB* cargar_fcb(t_config *file_fcb) {
    t_FCB* fcb = malloc(sizeof(t_FCB));
    if (!fcb) {
        perror("Error al cargar el FCB");
        exit(EXIT_FAILURE);
    }
    fcb->nombre_archivo = strdup(config_get_string_value(file_fcb, "nombre_archivo"));
    //log_info(logger_entradasalida, "ENTRO EN CARGAR FCB");
    fcb->tamanio_archivo = config_get_int_value(file_fcb, "tamanio_archivo");
    fcb->primer_bloque = config_get_int_value(file_fcb, "primer_bloque");  
    return fcb;
}


void persistir_fcb(t_FCB *fcb) {

    char path_directory_fcb [100] ;
    strcpy(path_directory_fcb, cfg_entrada_salida->PATH_BASE_DIALFS);
    char file_path[100]; // Tamaño suficiente para almacenar la ruta completa del archivo

    snprintf(file_path, sizeof(file_path), "%s/%s",path_directory_fcb,fcb->nombre_archivo);

    t_config* file_fcb = config_create(file_path);
    if (file_fcb == NULL) {
        log_info(logger_entrada_salida, "ERROR AL CREAR  CONFIG PARA PERSISTIR ARCHIVO FCB %s", file_path);
    }

    char* tamanio_archivo = uint32_to_string(fcb->tamanio_archivo);
    char* primer_bloque = uint32_to_string(fcb->primer_bloque);

    config_set_value(file_fcb,"nombre_archivo", fcb->nombre_archivo );
    log_info(logger_entrada_salida, "PERSISTIDO PARAMETRO nombre_archivo: %s", fcb->nombre_archivo);

    config_set_value(file_fcb,"tamanio_archivo", tamanio_archivo );
    log_info(logger_entrada_salida, "PERSISTIDO PARAMETRO tamanio_archivo: %d", fcb->tamanio_archivo);

    config_set_value(file_fcb,"primer_bloque", primer_bloque );
    log_info(logger_entrada_salida, "PERSISTIDO PARAMETRO primer_bloque: %d", fcb->primer_bloque);

    if (!config_save(file_fcb)){
        perror("Error al guardar fcb");
    };

    config_destroy(file_fcb);

}

void init_fcb_dict() {
    fcb_dict = dictionary_create();
}

t_dictionary* fcb_dict;
void agregar_fcb_to_dict(t_FCB* fcb) {
    dictionary_put(fcb_dict, fcb->nombre_archivo, fcb);
}

bool termina_en_txt(const char *nombre) {
    const char *ext = strrchr(nombre, '.');
    return ext != NULL && strcmp(ext, ".txt") == 0;
}

void cargar_directorio_fcbs(char* path_fcb ){
    DIR *directorio_fcb = opendir(path_fcb);
    struct dirent *fcb;

    if(directorio_fcb == NULL){
        log_info(logger_entrada_salida, "No se pudo abrir el directorio de fcb ");
        exit(1);
    }

    fcb_dict = dictionary_create();

    log_info(logger_entrada_salida, "Cargando directorio fcb en diccionario");
    while ((fcb = readdir(directorio_fcb)) != NULL) {
        // Verificar que el directorio no sea "." ni ".." (directorios especiales)
        if (strcmp(fcb->d_name, ".") != 0 && strcmp(fcb->d_name, "..") != 0  && termina_en_txt(fcb->d_name)) {
            // aca se puede crear un nuevo t_fcb para cada archivo y asociarlo al nombre del archivo en el diccionario
            t_FCB* nuevo_fcb = buscar_cargar_fcb(fcb->d_name);
           // Agregar el nuevo_fcb al diccionario con el nombre del archivo como clave
            dictionary_put(fcb_dict, fcb->d_name, nuevo_fcb);
        }
    }

    closedir(directorio_fcb);
}

t_FCB* buscar_cargar_fcb(char* nombre) {

    t_FCB* fcb = malloc(sizeof (t_FCB));
    if (fcb == NULL)  {
        log_info(logger_entrada_salida, "NO SE PUDO ASIGNAR MEMORIA AL FCB");
    };
    char path_fcb [100] ;
    strcpy(path_fcb, cfg_entrada_salida->PATH_BASE_DIALFS); // Directorio donde se encuentran los fcbs
    char file_path[100]; // Tamaño suficiente para almacenar la ruta completa del archivo
    snprintf(file_path, sizeof(file_path), "%s/%s",path_fcb,nombre);
    t_config* file_fcb;

    //cargar el fcb del archivo
    if((file_fcb = config_create(file_path)) == NULL){ //config_create: Devuelve un puntero hacia la estructura creada o NULL en caso de no encontrar el archivo en el path especificado
        log_info(logger_entrada_salida, "ARCHIVO NO ENCONTRADO : %s",file_path);
    }else {
        log_info(logger_entrada_salida, "ARCHIVO ENCONTRADO : %s",file_path);
    }

    fcb = cargar_fcb(file_fcb);
    return  fcb;
}

//////////////////////////////////////////////FUNCIONALIDADES//////////////////////////////////

uint32_t  crear_archivo(char* nombre){
    t_FCB* fcb;
    char path_fcb [100] ;
    strcpy(path_fcb, cfg_entrada_salida->PATH_BASE_DIALFS);
    char file_path[100]; // Tamaño suficiente para almacenar la ruta completa del archivo
    //snprintf(file_path, sizeof(file_path), "%s/%s%s",path_fcb,nombre,".config" );
    snprintf(file_path, sizeof(file_path), "%s/%s",path_fcb,nombre);
    if(  dictionary_has_key(fcb_dict,nombre) ) {
        log_info(logger_entrada_salida, "Ya existe el archivo: %s  ",nombre);// LOG OBLIGATORIO
        return 1;
     }else 
     {   
        log_info(logger_entrada_salida, "Crear Archivo: %s  ",nombre);// LOG OBLIGATORIO

        FILE* file_fcb_vacio = fopen(file_path,"w");

        if (file_fcb_vacio == NULL)  {
            perror("Error al crear el archivo de fcb vacio para ");
        }
        uint32_t posicion_bit_libre = encontrar_bit_libre(bitarray);
        if (posicion_bit_libre >=0) {
            
            fcb = inicializar_fcb(nombre,0, posicion_bit_libre);
            persistir_fcb(fcb);
            dictionary_put(fcb_dict, fcb->nombre_archivo, fcb);
        
            // actualizo el bitmap en memoria
            bitarray_set_bit(bitarray, posicion_bit_libre);
            sincronizar_bitmap ();
            
            log_info(logger_entrada_salida, "ARCHIVO  CREADO EN: %s",file_path);
            return 1;
        }else {
            log_info(logger_entrada_salida, "No hay espacio para crear el archivi solicitado"); 
        }    
    } // fin else de existencia de archivo    
}


uint32_t  borrar_archivo(char* nombre) {
    t_FCB* fcb_eliminar = malloc(sizeof(t_FCB));
    fcb_eliminar = dictionary_get(fcb_dict,nombre);    
    char * path_archivo_borrar = string_new();    

    string_append(&path_archivo_borrar,cfg_entrada_salida->PATH_BASE_DIALFS); 
    string_append(&path_archivo_borrar,"/");
    string_append(&path_archivo_borrar,nombre);

// liberar bloques en bitmap
    if (fcb_eliminar->tamanio_archivo > 0) {
        for (int i = fcb_eliminar->primer_bloque; i < fcb_eliminar->tamanio_archivo; i++) {
            bitarray_clean_bit(bitarray, i);
            sincronizar_bitmap ();
        }  
    } else {
            bitarray_clean_bit(bitarray, fcb_eliminar->primer_bloque); // archivos recien creados sin truncar
            sincronizar_bitmap ();
    }  
// borrar archivo fcb fisico 
 if (remove(path_archivo_borrar) == 0) {
        log_info(logger_entrada_salida, "Archivo eliminado exitosamente: %s", path_archivo_borrar);
    } else {
        log_error(logger_entrada_salida, "Error al eliminar el archivo: %s", path_archivo_borrar);
        perror("Error al eliminar el archivo");
    } 
// borrar fcb del diccionario
//dictionary_remove_and_destroy veo si lo uso
    dictionary_remove(fcb_dict,nombre);
// mostrar como queda el diccionario
    log_info(logger_entrada_salida, "Tamanio diccionario: %d", dictionary_size(fcb_dict));
         
return 1 ;
}

uint32_t  truncar_archivo(char* nombre, uint32_t tamanio ){
    t_FCB* fcb;
    uint32_t tamanio_actual;
    fcb = buscar_cargar_fcb(nombre); //ver si usar esta o traer desde el diccionario
    tamanio_actual = fcb->tamanio_archivo;
    uint32_t tamanio_total = tamanio_actual+ tamanio;
    log_info(logger_entrada_salida, "Truncar Archivo: %s - Tamaño: %d ",fcb->nombre_archivo,tamanio );// LOG OBLIGATORIO

    if(fcb->tamanio_archivo > tamanio) {
      log_info(logger_entrada_salida, "SE PROCEDERA A ACHICAR AL ARCHIVO: %s",fcb->nombre_archivo);
      achicar_archivo(tamanio,fcb);
    }else {
      log_info(logger_entrada_salida, "SE PROCEDERA A AGRANDAR AL ARCHIVO: %s",fcb->nombre_archivo);
      agrandar_archivo(tamanio_total,nombre);

    }
     imprimir_estado_bitarray();
    return 1;
}

void achicar_archivo(uint32_t tamanio, t_FCB* fcb) {
  
    uint32_t cant_bloques_desasignar = ceil((fcb->tamanio_archivo - tamanio) / cfg_entrada_salida->BLOCK_SIZE); 
    uint32_t cant_bloques_asignados = ceil(fcb->tamanio_archivo / cfg_entrada_salida->BLOCK_SIZE);
    uint32_t primer_bloque = fcb->primer_bloque;
    uint32_t bloque_final = primer_bloque+cant_bloques_asignados -1 ;
    //liberar posiciones en bitmap desde el bloque final
    for (int i = bloque_final; i > bloque_final-cant_bloques_desasignar; i--) {
        bitarray_clean_bit(bitarray,i);
    }
    sincronizar_bitmap();
    fcb->tamanio_archivo = tamanio;
    log_info(logger_entrada_salida, "Truncar Archivo: %s - Tamaño: %d: " ,fcb->nombre_archivo ,tamanio);

}

void agrandar_archivo(uint32_t nuevo_tamanio, char* nombre_archivo) {
    int nueva_posicion_inicial = hay_espacio_disponible(nuevo_tamanio); 
    t_FCB* fcb = malloc(sizeof( t_FCB));
    fcb = dictionary_get(fcb_dict,nombre_archivo);

    if (nueva_posicion_inicial>= 0 ) {
        if (fcb->tamanio_archivo == 0 ) { // para archivos recien creados
            log_info(logger_entrada_salida, "Archivo nunca usado");
            int tamanio_en_bloques = ceil( nuevo_tamanio /cfg_entrada_salida->BLOCK_SIZE);
            
            bitarray_clean_bit(bitarray,fcb->primer_bloque);                // borro la posicion actual en el bitmap
            log_info(logger_entrada_salida, "Primer bloque borrado: %d", fcb->primer_bloque);
            log_info(logger_entrada_salida, "Nueva posicion inicial: %d - Nuevo tamaño en bloques: %d ",nueva_posicion_inicial, tamanio_en_bloques);
            for (int i = 0; i <tamanio_en_bloques; i++) {
                    bitarray_set_bit(bitarray,i+nueva_posicion_inicial);
                }
            fcb->primer_bloque = nueva_posicion_inicial; 
            fcb->tamanio_archivo = nuevo_tamanio;        
        }else {   
            fcb->tamanio_archivo = nuevo_tamanio; 
            mover_archivo(fcb, nueva_posicion_inicial);        
        }
    fcb->primer_bloque = nueva_posicion_inicial;
    sincronizar_bitmap();
    persistir_fcb(fcb);
    dictionary_put(fcb_dict,fcb->nombre_archivo, fcb);
    log_info(logger_entrada_salida, "Truncar Archivo: %s - Tamaño: %d: " ,fcb->nombre_archivo ,nuevo_tamanio);    
    }else {
        log_info(logger_entrada_salida, "No hay más bloques disponibles"); // no verfico el caso sin espacio ISSUE #3568
    }   


}


void leer_archivo(t_io_readwrite_archivo* archivo, int socket){
    char* datos_leidos = malloc(archivo->tamanio_operacion * sizeof(char));
    t_io_memo_escritura* input = malloc (sizeof(t_io_memo_escritura));
    t_FCB* fbc_leer = malloc (sizeof(t_FCB));
    fbc_leer = dictionary_get(fcb_dict,archivo->nombre_archivo);
    int primer_bloque =  fbc_leer->primer_bloque;  
    int puntero_archivo_bloques = (cfg_entrada_salida->BLOCK_SIZE*primer_bloque+1)+archivo->puntero_archivo;
    
    // pararse en el byte a leer, y traer toda la info
    if (fseek(archivo_bloques,puntero_archivo_bloques, SEEK_SET)!= 0){
        log_error(logger_entrada_salida,"Error al mover el puntero de archivo al bloque: %d ",archivo->puntero_archivo);
    }  else{
        log_info(logger_entrada_salida, "PUNTERO POSICIONADO: %d",archivo->puntero_archivo );
    };
    if (fread(datos_leidos, archivo->tamanio_operacion, 1, archivo_bloques)<= 0){
        log_error(logger_entrada_salida,"Error al leer el archivo ");
    }  else{
        log_info(logger_entrada_salida, "PID: %d - Leer Archivo: %s - Tamaño a Leer: %d- Puntero Archivo: %d", archivo->pid, archivo->nombre_archivo, archivo->tamanio_operacion, archivo->puntero_archivo);
        log_info(logger_entrada_salida," Datos leidos, %s",datos_leidos);
        input->pid =archivo->pid;
        input->direcciones_fisicas =list_create();
        list_add_all(input->direcciones_fisicas,archivo->direcciones_fisicas);
        input->input = datos_leidos;
        input->input_length =string_length(datos_leidos) + 1;
        log_info(logger_entrada_salida, "input lenght: %d",input->input_length);
        enviar_input(input, socket, IO_FS_READ);
    };
}

void escribir_archivo(t_io_readwrite_archivo* archivo, char* datos_escribir){
    t_FCB* fbc_escribir = malloc (sizeof(t_FCB));
    fbc_escribir = dictionary_get(fcb_dict,archivo->nombre_archivo);
    int primer_bloque =  fbc_escribir->primer_bloque;  
    int puntero_archivo_bloques = (cfg_entrada_salida->BLOCK_SIZE*primer_bloque+1)+archivo->puntero_archivo;
    
    // pararse en el byte a escribir,
    if (fseek(archivo_bloques,puntero_archivo_bloques, SEEK_SET)!= 0){
        log_error(logger_entrada_salida,"Error al mover el puntero de archivo al bloque: %d ",archivo->puntero_archivo);
    }  else{
        log_info(logger_entrada_salida, "PUNTERO POSICIONADO: %d",archivo->puntero_archivo );
    };
    if (fwrite(datos_escribir, archivo->tamanio_operacion, 1, archivo_bloques)<= 0){
        log_info(logger_entrada_salida,"Error al escribir el archivo: %d ");
   
    }  else{
        fflush(archivo_bloques); //agrego esto para "garantizar" que se escriba el archivo de bloques si no tengo que esperar un fclose
        log_info(logger_entrada_salida, "PID: %d - Escribir Archivo: %s - Tamaño a Escribir: %d- Puntero Archivo: %d", archivo->pid, archivo->nombre_archivo, archivo->tamanio_operacion, archivo->puntero_archivo);
        log_info(logger_entrada_salida,"Datos escritos, %s",datos_escribir);
      
    };
}
////////////////////////////////////////////// UTILIDAD/////////////////////////////////////////////////
t_io_gestion_archivo* deserializar_fs_gestion (t_list* lista_paquete){
    
    t_io_gestion_archivo* nuevo_archivo = malloc(sizeof(t_interfaz));
    nuevo_archivo->pid = *(uint32_t*)list_get(lista_paquete, 0);
    nuevo_archivo->nombre_archivo_length = *(uint32_t*)list_get(lista_paquete, 1);
    nuevo_archivo->nombre_archivo = list_get(lista_paquete, 2);  
    nuevo_archivo->tamanio_archivo = *(uint32_t*)list_get(lista_paquete, 3);  
	return nuevo_archivo;
}

uint32_t encontrar_bit_libre(t_bitarray* bitarray_in) {

    log_info(logger_entrada_salida, "tamaño del bitarray %d %d",bitarray_get_max_bit(bitarray_in), bitarray_test_bit(&bitarray_in, 0));
    uint32_t i;
    for (i = 0; i < bitarray_get_max_bit(bitarray_in); i++) {
        if (!bitarray_test_bit(bitarray_in, i)) {
            log_info(logger_entrada_salida, "Acceso a Bitmap - Bloque: %d - Estado: libre", i); //LOG OBLIGATORIO
            return i;
        }else {
            log_info(logger_entrada_salida, "Acceso a Bitmap - Bloque: %d - Estado: ocupado", i); //LOG OBLIGATORIO
        }
    }
    return -1; // Retorna -1 si no se encuentra ningún bit en 0
}

void sincronizar_bitmap (){
    memcpy(bitmap, bitarray->bitarray, bitmap_size_in_bytes);
    int resultado_sync = msync(bitmap, bitmap_size_in_bytes, MS_SYNC);
    int resultado_fync= fsync(fd_bitmap);
    if (resultado_sync == -1 || resultado_fync == -1) {
        perror("Error al sincronizar el bitmap");
        // Manejar el error según sea necesario
    } else {
        log_info(logger_entrada_salida, "SINCRONIZACION DE BITMAP EXITOSA");
    }
}

int hay_espacio_disponible(int espacio_necesario) {
    int espacio_disponible;
    int posicion_inicial;

    if (hay_espacio_total_disponible(espacio_necesario)) {
        posicion_inicial =  hay_espacio_contiguo_disponible(espacio_necesario);
        if (posicion_inicial >= 0 ) {
            return posicion_inicial;
        }else {
           posicion_inicial = compactar(espacio_necesario);
            return posicion_inicial;
        }     
    } else {
        return -1;  
   }  
}

bool hay_espacio_total_disponible(int espacio_necesario){
    int espacio_disponible = 0;
    for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            espacio_disponible++;
        }
    }
    log_info(logger_entrada_salida,"Cantidad de bits %d:",  bitarray_get_max_bit(bitarray));
    log_info(logger_entrada_salida,"Bloques/bits libres %d:",  espacio_disponible);
   log_info(logger_entrada_salida,"Espacio total disponible %d:",  espacio_disponible*cfg_entrada_salida->BLOCK_SIZE);
return espacio_disponible*cfg_entrada_salida->BLOCK_SIZE >= espacio_necesario;
}   

int hay_espacio_contiguo_disponible(int espacio_necesario) {
    int bloques_necesarios = (espacio_necesario + cfg_entrada_salida->BLOCK_SIZE - 1) / cfg_entrada_salida->BLOCK_SIZE; // Redondeo hacia arriba
    int bloques_libres_consecutivos = 0;
    int posicion_inicial = -1; // Inicio con valor invalido para no confundir

    for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            if (bloques_libres_consecutivos == 0) {
                posicion_inicial = i; 
            }
            bloques_libres_consecutivos++;
            if (bloques_libres_consecutivos == bloques_necesarios) {
                return posicion_inicial; 
            }
        } else {
            // reinicio la cuenta y la posicion inicial
            bloques_libres_consecutivos = 0;
            posicion_inicial = -1;
        }
    }
    return -1; // No hay espacio contiguo suficiente
}

int compactar(int espacio_necesario){
    
    int espacio_disponible = 0;
    int posicion_inicio = -1;
    char* valor_bloque = malloc(cfg_entrada_salida->BLOCK_SIZE);
    t_FCB* fcb = malloc(sizeof(t_FCB));
    t_list* fcbs = list_create();
    fcbs = dictionary_elements(fcb_dict);
    int i = 0;

    while (posicion_inicio == -1 ) { // mientras no haya espacio 
   
        log_info(logger_entrada_salida, "No hay espacio contiguo suficiente. Compactando...");
               
        int posiciones_libres  = 0;
        int j  = 1;
        fcb = list_get(fcbs,i);
        int flg_stop = 0;
        log_info(logger_entrada_salida, "Nombre de archivo: %s",fcb->nombre_archivo); 
        //acumular las posiciones libres a la izquierda,
        if(fcb->primer_bloque == 0){flg_stop = 1;} // por si el primner bloque es 0 no se puede mover a la izquierda
        while (flg_stop == 0  && !bitarray_test_bit(bitarray, fcb->primer_bloque-j) ){
            posiciones_libres++;  
            if(fcb->primer_bloque-j > 0){j++;}// aumento el desplazamiento hacia la izquierda
            else {flg_stop = 1;} // para de moverme porque el indice solo llega a 0     
            log_info(logger_entrada_salida, "cuenta posicion libre: %d ", posiciones_libres);
        }
        if(posiciones_libres>0){
            //si hay espacio mover el archivo a la nueva posicion
            log_info(logger_entrada_salida, "Posiciones libres: %d",posiciones_libres);
            int nueva_posicion = fcb->primer_bloque-posiciones_libres;
            log_info(logger_entrada_salida, "Nueva posicion: %d",nueva_posicion);
            mover_archivo(fcb, nueva_posicion); 
            imprimir_estado_bitarray();               
        } 
                  
        posicion_inicio = hay_espacio_contiguo_disponible(espacio_necesario);
        if(i < list_size(fcbs)-1){i++;  log_info(logger_entrada_salida, "Indice archivo: %d",i);} // siguiente archivo
        else {i = 0;} // si termino la lista de archivo y no hay lugar arranco a moverlos de nuevo
           
    } 

    sincronizar_bitmap ();
    log_info(logger_entrada_salida, "Espacio contiguo suficiente encontrado en la posición %d.", posicion_inicio);
    usleep(cfg_entrada_salida->RETRASO_COMPACTACION);
    return posicion_inicio;
}    

void escribir_bloque (int numero_bloque,  char *datos_escribir){ // los datos deberian ser void??

    if (fseek(archivo_bloques,(numero_bloque * cfg_entrada_salida->BLOCK_SIZE ) , SEEK_SET)!= 0){
        log_info(logger_entrada_salida,"Error al mover el puntero de archivo al bloque: %d ",numero_bloque);
        return -1;
    }  else{
        log_info(logger_entrada_salida, "PUNTERO POSICIONADO EN BLOQUE: %d ",numero_bloque );
    };

    if (fwrite(datos_escribir,  cfg_entrada_salida->BLOCK_SIZE, 1, archivo_bloques)<= 0){
        log_info(logger_entrada_salida,"Error al escribir el bloque: %d ",numero_bloque);
        return -1;
    }  else{
        fflush(archivo_bloques); //agrego esto para "garantizar" que se escriba el archivo de bloques si no tengo que esperar un fclose
        log_info(logger_entrada_salida, "BLOQUE: %d ESCRITO con valor %s",numero_bloque, datos_escribir);
    };
}

void leer_bloque (int numero_bloque, void *datos ) {

    if (fseek(archivo_bloques,(numero_bloque * cfg_entrada_salida->BLOCK_SIZE ), SEEK_SET)!= 0){
        log_info(logger_entrada_salida,"Error al mover el puntero de archivo al bloque: %d ",numero_bloque);
    }  else{
        log_info(logger_entrada_salida, "PUNTERO POSICIONADO EN BLOQUE: %d DESPLAZADO %d",numero_bloque,numero_bloque * cfg_entrada_salida->BLOCK_SIZE );
    };

    if (fread(datos, cfg_entrada_salida->BLOCK_SIZE, 1, archivo_bloques)<= 0){
        log_info(logger_entrada_salida,"Error al leer el bloque: %d ",numero_bloque);
    }  else{
        log_info(logger_entrada_salida, "BLOQUE: %d LEIDO",numero_bloque);

    };
}

void mover_archivo(t_FCB* fcb_archivo, int nueva_posicion_inicial){

    int posicion_inicial = fcb_archivo->primer_bloque;
    int tamanio_en_bloques = ceil( fcb_archivo->tamanio_archivo /cfg_entrada_salida->BLOCK_SIZE);
    log_info(logger_entrada_salida, "Tamaño en bytes: %d Tamaño en bloques: %d",fcb_archivo->tamanio_archivo,tamanio_en_bloques);
    if(fcb_archivo->tamanio_archivo == 0){
        bitarray_clean_bit(bitarray,posicion_inicial);               
	    bitarray_set_bit(bitarray,nueva_posicion_inicial);
    }else{    
        char* valor_bloque = malloc(cfg_entrada_salida->BLOCK_SIZE);
        int j = 0;    
        for (int i = posicion_inicial; i <= posicion_inicial+tamanio_en_bloques-1; i++){
            leer_bloque (i, valor_bloque );
            bitarray_clean_bit(bitarray, i);
            escribir_bloque (nueva_posicion_inicial+j,valor_bloque);
            bitarray_set_bit(bitarray, nueva_posicion_inicial+j);
            j++;
        }
    }
    // actualizo fcb en todas las estructuras
    fcb_archivo ->primer_bloque = nueva_posicion_inicial;
    dictionary_put(fcb_dict ,fcb_archivo->nombre_archivo, fcb_archivo);
    persistir_fcb(fcb_archivo);
    sincronizar_bitmap ();  
}

char* uint32_to_string (uint32_t number) {
    char* str ;
    if (asprintf(&str, "%u", number)== -1) {
        //error al asignar memoria
        return NULL;
    }
    return str;
}

void imprimir_estado_bitarray() {

    log_info(logger_entrada_salida, "ESTADO BITARRAY:");
    uint32_t i;
    for (i = 0; i < bitarray_get_max_bit(bitarray); i++) {
        if (bitarray_test_bit(bitarray, i)) {
            log_info(logger_entrada_salida,"%d",1);         
        }else {
            log_info(logger_entrada_salida,"%d",0); 
        }
    }
}