#include "../include/dialfs.h"

/////////////////////////////////////////////////INICIAR FS////////////////////////////////////

char * path_archivo_bloques = cfg_entrada_salida->PATH_BASE_DIALFS +"bloques.dat";
char* path_archivo_bitmap = cfg_entrada_salida->PATH_BASE_DIALFS+"bitmap.dat";
char* path_fcb = cfg_entrada_salida->PATH_BASE_DIALFS;
    //BITMAP//
    if(crear_bitmap ()>=0 ) {
        log_info(logger_entrada_salida, "Bitmap creado correctamente");
    }
    else {
        log_info(logger_entrada_salida, "Error en creacion de bitmap");
        return EXIT_FAILURE;
    }

    // ARCHIVO DE BLOQUES//
    if(crear_archivo_bloques (cfg_entrada_salida->BLOCK_SIZE, cfg_entrada_salida->BLOCK_COUNT)>=0 ) {
        log_info(logger_entrada_salida, "Archivo de bloques creado correctamente");
    }
    else {
        log_info(logger_entrada_salida, "Error en creacion de Archivo de bloques");
        return EXIT_FAILURE;
    }

    cargar_directorio_fcbs();

int crear_bitmap () {
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

int crear_archivo_bloques ( int block_size, int block_count) {

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
    strcpy(path_directory_fcb, path_fcb);
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


void cargar_directorio_fcbs(){
    DIR *directorio_fcb = opendir(path_fcb);
    struct dirent *fcb;

    if(directorio_fcb == NULL){
        log_info(logger_entrada_salida, "No se pudo abrir el directorio de fcb ");
        exit(1);
    }

    fcb_dict = dictionary_create();

    while ((fcb = readdir(directorio_fcb)) != NULL) {
        // Verificar que el directorio no sea "." ni ".." (directorios especiales)
        if (strcmp(fcb->d_name, ".") != 0 && strcmp(fcb->d_name, "..") != 0) {
            // Aquí puedes crear un nuevo t_fcb para cada archivo y asociarlo al nombre del archivo en el diccionario
            t_FCB* nuevo_fcb = buscar_cargar_fcb(fcb->d_name);
           // Agregar el nuevo_fcb al diccionario con el nombre del archivo como clave
            dictionary_put(fcb_dict, fcb->d_name, nuevo_fcb);
        }
    }

    closedir(directorio_fcb);
}

//////////////////////////////////////////////FUNCIONALIDADES//////////////////////////////////