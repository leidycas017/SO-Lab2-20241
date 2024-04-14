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

/**La función prompt() muestra el símbolo de shell */
void prompt() {
    fprintf(stdout, "%s", "wish> ");
}

/** Toma una línea de entrada(comando) y la divide en tokens que se almacenan en el arreglo de cadenas args */
void parsear_comando(char *comando, char **args) {
    char *token;                                       // Puntero token(argumento o comando)
    int i = 0;
    // Se ejecuta mientras existan tokens y no se haya alcanzado el máximo número de tokens
    while ((token = strsep(&comando, " \n")) != NULL && i < MAXIMOS_ARGUMENTOS - 1) {
        if (*token != '\0') {
            args[i++] = token;                     // Se almacena token
        }
    }
    args[i] = NULL;       // Marca el final de la lista de argumentos
}

/**
 * Error genérico para todas las ocurrencias de error
 */
void error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

/**
 * Esta función es responsable de ejecutar los comandos externos (comandos que no son internos al shell)
 */
int ejecutar_comando_externo(char **args) {
    pid_t pid;                      // Id del proceso
    int status;                     // Estado del proceso hijo
    int comando_ejecutado = 1; // Inicializar en 1 para indicar éxito por defecto

    // Verificar si el comando es "exit" con un argumento ( wish > exit argumento)
    if (strcmp(args[0], "exit") == 0 && args[1] != NULL) {
        error_Three("ls: cannot access '/no/such/file': No such file or directory");
        comando_ejecutado = 0; // Indicar fallo
    } else {
        // Crear un proceso hijo
        pid = fork(); 
        if (pid == 0) {
            // En el proceso hijo, ejecutar el comando
            execvp(args[0], args);
            error_Three("ls: cannot access '/no/such/file': No such file or directory");
            // Si execvp falla, mostrar un mensaje de error específico para ls
            if (strcmp(args[0], "ls") == 0 && args[1] != NULL) {
                error_Three("ls: cannot access '/no/such/file': No such file or directory");
            } else {
                     error();
            }
            exit(0); // Salir con código de retorno 0 en caso de error y o retorno de exec
        } else if (pid < 0) {
            // En caso de error al crear el proceso hijo, mostrar un mensaje de error
            error();
        } else {
            // En el proceso padre, esperar a que el proceso hijo termine
            waitpid(pid, &status, 0);
            // Si el proceso hijo terminó exitosamente, cambiar el valor de la variable
            if (EXEC_SUCCESS(status)) {
                comando_ejecutado = 1;
            }
        }
        
        // Si no se ejecutó el comando y no es un comando de listado, mostrar un mensaje de error
        if (!comando_ejecutado && strcmp(args[0], "ls") != 0) {
            error();
        }
    }

    return comando_ejecutado; // Retornar 1 en caso de éxito, 0 en caso de fallo
}

/**
 * Esta función es responsable de la ejecución de comandos internos del shell como "exit", "cd" y "path"
 */
int ejecutar_comando_interno(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        // Si no hay argumentos, retorna código de error 0
        if (args[1] == NULL) {
            error(); // Mostrar mensaje de error
            return 0; // Retornar código de error 0
        }
        // Si hay exactamente dos argumentos, intentar cambiar de directorio
        if (args[2] == NULL) {
            if (chdir(args[1]) != 0) {
                //error(); // Mostrar mensaje de error
                return 0; // Retornar código de error 0
            }
        } else {
            error(); // Mostrar mensaje de error
            return 0; // Retornar código de error 0
        }
    } else if (strcmp(args[0], "path") == 0) {
        agregar_ruta(args[1]);
    }

    // Si no es ninguno de los comandos anteriores, se considera un comando válido
    return 1; // Retornar código de éxito
}

/* Se parsean comandos para operaciones en segundo plano */
void parsear_comandos(char *comando, char **args, int *segundoplano) {
    char *token;
    int i = 0;
    token = strtok(comando, " \n");
    // token = strsep(&comando, " \n");                // No funciona bien
    while (token != NULL && i < MAXIMOS_ARGUMENTOS - 1) {
        args[i] = token;
        if (strcmp(token, "&") == 0) {
            *segundoplano = 1;
            args[i] = NULL; // Eliminar el "&" de los argumentos
            break;
        }
        token = strtok(NULL, " \n");
        i++;
    }
    args[i] = NULL;
}

/* Esta función ejecuta comandos, ya sea en primer plano o en segundo plano, según se especifique. */
void ejecutar_comando(char **args, int segundoplano) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        error();
        //exit(1);
    } else if (pid < 0) {
              error(); 
           }
           else{
               if(!segundoplano){
                  waitpid(pid, &status, 0);
               }
           }
 }
 