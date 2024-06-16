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
                t_io_crear_archivo* archivo_nuevo = malloc(sizeof(t_io_crear_archivo));
                archivo_nuevo = deserializar_fs_creacion (lista_paquete);
                crear_archivo(archivo_nuevo->nombre_archivo);
                list_destroy(lista_paquete);
                free(archivo_nuevo);
                response = IO_K_GEN_SLEEP_FIN;

                 if (send(socket_kernel, &response, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    log_error(logger_entrada_salida, " Error al enviar IO_K_GEN_SLEEP_FIN a Kernel");
                    break;
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
        log_info(logger_entrada_salida, "ESTABLECER EL TAMAÑO DEL BITMAP  : %d : " ,bitmap_size_in_bytes);
        if (ftruncate(fd_bitmap, bitmap_size_in_bytes) == -1) {
            perror("Error al establecer el tamaño del archivo");
            close(fd_bitmap);
            return -1;
        }

        // Mapea el archivo a la memoria
         bitmap = mmap(NULL, block_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);

        if (bitmap == MAP_FAILED) {
            perror("Error al mapear el archivo");
            close(fd_bitmap);
            return -1;
        }

        log_info(logger_entrada_salida, "BITMAP MAPEADO A MEMORIA ");
        // Inicializa el bitmap
        bitarray = bitarray_create_with_mode(bitmap, block_count, LSB_FIRST);
        log_info(logger_entrada_salida, "BITMAP CARGADO EN BITARRAY ");
        // Marca el primer bloque como utilizado
        bitarray_set_bit(bitarray, 0);

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
        bitarray = bitarray_create(bitmap, block_count);
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
///////////////////////////////////////////////PETICIONES//////////////////////////////////////
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


char* uint32_to_string (uint32_t number) {
    char* str ;
    if (asprintf(&str, "%u", number)== -1) {
        //error al asignar memoria
        return NULL;
    }
    return str;
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

    log_info(logger_entrada_salida, "Crear Archivo: %s  ",nombre);// LOG OBLIGATORIO

    FILE* file_fcb_vacio = fopen(file_path,"w");

    if (file_fcb_vacio == NULL)  {
        perror("Error al crear el archivo de fcb vacio para ");
    }
    uint32_t posicion_bit_libre = encontrar_bit_libre(bitarray);
    if (posicion_bit_libre >0) {
        
        fcb = inicializar_fcb(nombre,0, posicion_bit_libre);
        persistir_fcb(fcb);
        dictionary_put(fcb_dict, fcb->nombre_archivo, fcb);
       
         // actualizo el bitmap en memoria
        bitarray_set_bit(bitarray, posicion_bit_libre);
        memcpy(bitmap, bitarray->bitarray, bitmap_size_in_bytes);
        int resultado_sync = msync(bitmap, bitmap_size_in_bytes, MS_SYNC);
        if (resultado_sync == -1) {
            perror("Error al sincronizar con msync el bitmap");
            // Manejar el error según sea necesario
        } else {
            log_info(logger_entrada_salida, "SINCRONIZACION DE BITMAP EXITOSA");
        }
        
        log_info(logger_entrada_salida, "ARCHIVO  CREADO EN: %s",file_path);
        return 1;
    }else {
        log_info(logger_entrada_salida, "No hay espacio para crear el archivi solicitado"); 
    }    
}

uint32_t encontrar_bit_libre(t_bitarray* bitarray_in) {

    log_info(logger_entrada_salida, "tamaño del bitarray %d %d", bitarray_in->size, bitarray_test_bit(&bitarray_in, 0));

    uint32_t i;
    for (i = 0; i < bitarray_in->size; i++) {

        if (!bitarray_test_bit(bitarray_in, i)) {
            log_info(logger_entrada_salida, "Acceso a Bitmap - Bloque: %d - Estado: libre", i); //LOG OBLIGATORIO
            return i;
        }else {
            log_info(logger_entrada_salida, "Acceso a Bitmap - Bloque: %d - Estado: ocupado", i); //LOG OBLIGATORIO
        }
    }
    return -1; // Retorna -1 si no se encuentra ningún bit en 0
}

////////////////////////////////////////////// UTILIDAD/////////////////////////////////////////////////
t_io_crear_archivo* deserializar_fs_creacion (t_list* lista_paquete){
    
    t_io_crear_archivo* nuevo_archivo = malloc(sizeof(t_interfaz));
    nuevo_archivo->pid = *(uint32_t*)list_get(lista_paquete, 0);
    nuevo_archivo->nombre_archivo_length = *(uint32_t*)list_get(lista_paquete, 1);
    nuevo_archivo->nombre_archivo = list_get(lista_paquete, 2);    
	return nuevo_archivo;

}