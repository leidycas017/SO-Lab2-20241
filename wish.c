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