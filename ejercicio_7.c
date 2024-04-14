#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

void proceso(int argc, char *argv[]) {
    switch (argc) {
        case 1:
            fprintf(stderr, "Uso: %s <command> [args]\n", argv[0]);
            exit(1);
            break;

        default: {
            struct timeval start, end;
            if (gettimeofday(&start, NULL) == -1) { // Obtener el tiempo de inicio
                fprintf(stderr, "Error al obtener el tiempo de inicio\n");
                exit(1);
            }

            pid_t pid = fork(); // Crear un proceso hijo
            if (pid < 0) {
                fprintf(stderr, "Error en la creación del hijo fork\n");
                exit(1);
            }

            if (pid == 0) { // Proceso hijo
                // Ejecutar el comando especificado con sus argumentos
                if (execvp(argv[1], &argv[1]) == -1) {
                    fprintf(stderr, "Error al ejecutar el comando\n");
                    exit(1);
                }
            } else { // Proceso padre
                int status;
                if (wait(&status) == -1) { // Esperar a que el proceso hijo termine
                    fprintf(stderr, "Error al esperar al proceso hijo\n");
                    exit(1);
                }
                if (gettimeofday(&end, NULL) == -1) { // Obtener el tiempo de finalización
                    fprintf(stderr, "Error al obtener el tiempo de finalización\n");
                    exit(1);
                }

                // Calcular el tiempo transcurrido
                double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

                // Imprimir el tiempo transcurrido
                fprintf(stdout, "Tiempo de duracion de ejecución del proceso: %f\n", elapsed_time);
            }
            break;
        }
    }
}
int main(int argc, char *argv[]) {
    proceso(argc, argv);
    return 0;
}