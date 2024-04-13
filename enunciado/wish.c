/*  
 * Programa: Ejecución de lote de comandos
 * Autores:  [Leidy Castaño Castaño], [Yuly Yesenia Alvear Romo], [Omar Alberto Torres]
 * Profesor: [Dany Alexandro Munera ]
 * Curso:    [Sistemas operativos y laboratorio]
 * Fecha:    [Abril 10 del 2024]
 */

/******************************** Insercción de librería standard ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/********************************* Definición de constantes del programa ****************************/
#define MAXIMA_LONGITUD_PATH 512
#define MAXIMA_LONGITUD_COMANDO 512
#define MAXIMOS_ARGUMENTOS 512
#define MAXIMA_LOGITUD_LINEA  512
#define MAXIMO_BUFFER  1024
#define EXEC_SUCCESS(status) ((status) == 0) 
#define  MAX_INPUT_LENGTH 1024

/**************************************** Variables globales ***************************************/
char **rutas; 
int nume_rutas = 0;
int capacidad_rutas = 256;

/*********************************** Definición prototipos de funciones *****************************/
FILE* getFile(int argc, char *argv[]);
char** almacenarArgumentos(char *token, char *line_copy) ;
void error();
void error_Three(char *msg);
void prompt();
void parsear_comando(char *comando, char **args);
int ejecutar_comando_externo(char **args);
int ejecutar_comando_interno(char **args);
void inicializar_rutas();
void procesoOne(char *comando);
int procesoTres(int argc, char *argv[]);
void parsear_comandos(char *comando, char **args, int *segundoplano);
void ejecutar_comando(char **args, int segundoplano);
void procesoTwo(char *comando);
void  proceso(char * comando);
int contiene_ampersand(const char *cadena);
void redirigir_salida_entrada_a_archivos(char *comando);
void agregar_ruta(const char *nueva_ruta);
void liberar_rutas();

/***************************************** Función principal ***************************************/

/** La funcion main controla el flujo del program y ciclo del proceso interactivo ******************/
int main(int argc, char *argv[]) {
    inicializar_rutas();
    nume_rutas = sizeof(rutas) / sizeof(rutas[0]);
    char comando[MAXIMA_LONGITUD_COMANDO];

    if (argc == 2) {
        procesoTres(argc, argv); // Se ejecuta proceso batch si se proporciona un argumento
        return 0;
    }

    while (1) {
        prompt();
        if (fgets(comando, MAXIMA_LONGITUD_COMANDO, stdin) == NULL) {
            // Si fgets retorna NULL, indica que se llegó al final del archivo (EOF)
            break; // Salir del bucle
        }

        // Eliminar el carácter de nueva línea del final del comando
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit") == 0) {
            break; // Salir del bucle si se ingresa "exit"
        }

        if (strcmp(comando, "") == 0) {
            continue; // Volver a solicitar el comando si se ingresó una línea vacía
        }

        if (contiene_ampersand(comando)) {
            proceso(comando); // Se ejecutan comandos en segundo plano
        } else if (strchr(comando, '>') || strchr(comando, '<')) {
            redirigir_salida_entrada_a_archivos(comando); // Se maneja el redireccionamiento
        } else {
            procesoOne(comando); // Se procesan los comandos internos/externos
        }
    }
    liberar_rutas();
    return 0;
}

/*********************Liberar memoria dinámica******************************/
void error_Three(char *msg){
    write(STDERR_FILENO, msg, strlen(msg)); 
}
void liberar_rutas() {
    for (int i = 0; i < nume_rutas; i++) {
        free(rutas[i]);
    }
    free(rutas);
}

/****************Inicializa ruta para busqueda de comandos************/
void agregar_ruta(const char *nueva_ruta) {
    if (nume_rutas >= capacidad_rutas) { // Verificar si hay espacio suficiente
        // Duplicar la capacidad del arreglo
        capacidad_rutas *= 2;
        rutas = realloc(rutas, capacidad_rutas * sizeof(char*));
        if (rutas == NULL) {
            error();
           return;
        }
    }
    // Agregar la nueva ruta
    rutas[nume_rutas++] = strdup(nueva_ruta);
}

/**
 *   La funcion inicializar rutas crea dinámicamente un array predeterminando algunas rutas
 *   compatibles con Unix y sistema operativo MAC
 */
void inicializar_rutas() {
    rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));
    rutas[0] = strdup("/bin/");
    rutas[1] = strdup("/usr/bin/");
    rutas[2] = strdup("/path/to/date/directory/");
    rutas[3] = strdup("/path/to/pwd/directory/");
    rutas[4] = strdup("/boot/");
    rutas[5] = strdup("/dev/");
    rutas[6] = strdup("/etc/");
    rutas[7] = strdup("/home/");
    rutas[8] = strdup("/lib/");
 }

/******************************** Procesos por lotes **********************************************/
/**
 * La funcion procesoTres es responsable del procesamiento del lote de comandos
 *    ./wish comandos.sh
 *    La funcion obtiene la ruta en donde esta el programa shell del sistema operativo
 *    Se crea un hijo en el que se ejecuta execvp(args[0], args), con esto le estamos diciendo al sistema operativo
 *    que queremos que ejecute el programa shell pasándole el script con los comandos. Esto es transferimos
 *    el control a shell para que ejecute todos los comandos del script y ya no tenemos que iterar sobre el script
 *    para ejecutar los comandos. La magia la ordena el sistema operativo al shell.
 */
 
int procesoTres(int argc, char *argv[]) {
    FILE *script = fopen(argv[1], "r");
    
    if (argc != 2 || script == NULL) {
        error(); 
        return 0; // Retorna 1 en caso de error
    }

    // Crear un proceso hijo
    pid_t pid = fork();
    if (pid < 0) {
        error();
        return 0; // Retorna 1 en caso de error
    } else if (pid == 0) {
        // Proceso hijo
        char *comando = strdup("/bin/sh"); // Obtener la ruta del shell
        char *args[] = {comando, argv[1], NULL}; // Argumentos para ejecutar el script
        execvp(args[0], args); // Ejecutar el script de shell
        error(); // Si execvp falla
        return 1;
    } else {
        // Proceso padre
        int status;
        waitpid(pid, &status, 0);
        if (!EXEC_SUCCESS(status)) {
            error(); // Mostrar mensaje de error si la ejecución falla
            return 0; // Retorna 1 en caso de error
        }
    }

    return 1; // Retorna 1 si todo se ejecutó correctamente
}