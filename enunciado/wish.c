/*
    Programa: Ejecución de lote de comandos
    Autores:  [Leidy Castaño Castaño], [Yuly Yesenia Alvear Romo], [Omar Alberto Torres]
    Profesor: [Dany Alexandro Munera ]
    Curso:    [Sistemas operativos y laboratorio]
    Fecha:    [Abril 10 del 2024]
*/
/********************************Insercción de librería standard***********************************/
// Correcto falta funcionalidad deredireccion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/*********************************Definición de constantes del programa ****************************/

#define MAXIMA_LONGITUD_PATH 512
#define MAXIMA_LONGITUD_COMANDO 512
#define MAXIMOS_ARGUMENTOS 512
#define MAXIMA_LOGITUD_LINEA 512
#define MAXIMO_BUFFER 1024
#define EXEC_SUCCESS(status) ((status) == 0)
// #define  MAX_INPUT_LENGTH 1024
/**************************************** Variables globales ***************************************/

char **rutas;
int nume_rutas = 0;

/*********************************** Definición prototipos de funciones *****************************/

FILE *getFile(int argc, char *argv[]);
char **almacenarArgumentos(char *token, char *line_copy);
void error();
void prompt();
void parsear_comando(char *comando, char **args);
void ejecutar_comando_externo(char **args);
void ejecutar_comando_interno(char **args);
void inicializar_rutas();
void procesoOne(char *comando);
void procesoTres(int argc, char *argv[]);
void parsear_comandos(char *comando, char **args, int *segundoplano);
void ejecutar_comando(char **args, int segundoplano);
void procesoTwo(char *comando);
void proceso(char *comando);
int contiene_ampersand(const char *cadena);
void redirigir_salida_entrada_a_archivos(char *comando);

/***************************************** Función principal ***************************************/
/** La funcion main controla el flujo del program y ciclo del proceso interactivo ******************/
int main(int argc, char *argv[])
{
    inicializar_rutas();
    char comando[MAXIMA_LONGITUD_COMANDO];

    if (argc == 2)
    {
        procesoTres(argc, argv); // Se ejecuta proceso batch si se proporciona un argumento
        return 0;
    }

    while (1)
    {
        prompt();
        if (fgets(comando, MAXIMA_LONGITUD_COMANDO, stdin) == NULL)
        {
            // Si fgets retorna NULL, indica que se llegó al final del archivo (EOF)
            break; // Salir del bucle
        }

        // Eliminar el carácter de nueva línea del final del comando
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit") == 0)
        {
            break; // Salir del bucle si se ingresa "exit"
        }

        if (strcmp(comando, "") == 0)
        {
            continue; // Volver a solicitar el comando si se ingresó una línea vacía
        }

        if (contiene_ampersand(comando))
        {
            proceso(comando); // Se ejecutan comandos en segundo plano
        }
        else if (strchr(comando, '>') || strchr(comando, '<'))
        {
            redirigir_salida_entrada_a_archivos(comando); // Se maneja el redireccionamiento
        }
        else
        {
            procesoOne(comando); // Se procesan los comandos internos/externos
        }
    }

    return 0;
}

/******************************** Procesos por lotes **********************************************/
/**La funcion ProcesoTres responsable del proesamiendo del lote de comandos*/
/**
 * Esta funcion nos generó muchos problemas por que al ejecutar el execv(--) el padre al parecer no podía
 * retomar el flujo normal del programa y el programa se quedaba en un ciclo infinito
 * la solucion fue crear una vía de comunicación entre el proceso padre y el proceso hijo.
 * la tubería creada por popen() basicamente es una area de memoria intermedia en donde el padre y el hijo se escrinben mutuamente
 * el padre escribe el comando a ejecutar el hijo lee el comando y lo ejecuta, para luego escribir
 * en la salida el resultado de su proceso, y el padre lee el resultado y lo manda al flujo de salida
 * que corresponda.
 */
/**
 *    ./wish comandos.sh
 */
void procesoTres(int argc, char *argv[])
{
    error();
    FILE *comandos = fopen(argv[1], "r");

    if (argc != 2)
    {
        error();
        exit(1);
    }
    if (comandos == NULL)
    {
        error();
        exit(1);
    }

    char linea[MAXIMA_LOGITUD_LINEA];

    // Leer cada línea del archivo de comandos
    while (fgets(linea, sizeof(linea), comandos) != NULL)
    {
        // Eliminar el carácter de nueva línea, si está presente
        linea[strcspn(linea, "\n")] = '\0';

        // Salir del bucle si se encuentra el comando "exit"
        if (strcmp(linea, "exit") == 0)
        {
            break;
        }

        FILE *fp = popen(linea, "r"); // Ejecutar el comando y abrir una tubería para leer su salida
        if (fp == NULL)
        {
            error();
            exit(1);
        }

        char buffer[MAXIMO_BUFFER];
        // Leer la salida del comando línea por línea y mostrarla en la consola
        while (fgets(buffer, sizeof(buffer), fp) != NULL)
        { // En fp viene la salida del hijo
            fprintf(stdout, "%s", buffer);
        }

        pclose(fp); // Cerrar el pipeline después de leer la salida del comando
    }

    // Cerrar el archivo después de procesar todas las líneas del archivo
    fclose(comandos);
    exit(0);
}