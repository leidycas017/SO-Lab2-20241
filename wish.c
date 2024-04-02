/*
    Programa: Ejecución de lote de comandos
    Autores:  [Leidy Castaño Castaño], [Yuly Yesenia Alvear Romo], [Omar Alberto Torres]
    Profesor: [Dany Alexandro Munera ]
    Curso:    [Sistemas operativos y laboratorio]
    Fecha:    [Abril 10 del 2024]
*/
/********************************Insercción de librería standard***********************************/
//Correcto falta funcionalidad deredireccion
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
#define MAXIMA_LOGITUD_LINEA  512
#define MAXIMO_BUFFER  1024
#define EXEC_SUCCESS(status) ((status) == 0) 
//#define  MAX_INPUT_LENGTH 1024
/**************************************** Variables globales ***************************************/

char **rutas; 
int nume_rutas = 0;

/*********************************** Definición prototipos de funciones *****************************/

FILE* getFile(int argc, char *argv[]);
char** almacenarArgumentos(char *token, char *line_copy) ;
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
void  proceso(char * comando);
int contiene_ampersand(const char *cadena);
void redirigir_salida_entrada_a_archivos(char *comando);

/***************************************** Función principal ***************************************/
