#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define ALTO 8
#define ANCHO 12
#define MAX_PALABRAS 10

char tablero[ALTO][ANCHO];
int activa[ALTO][ANCHO];

char *respuestas[MAX_PALABRAS];
char *pistas[MAX_PALABRAS];
char *alt_respuestas[MAX_PALABRAS];
char *alt_pistas[MAX_PALABRAS];

int filas[MAX_PALABRAS];
int cols[MAX_PALABRAS];
int largos[MAX_PALABRAS];
int dirs[MAX_PALABRAS];          // 0 horizontal, 1 vertical
int adivinadas[MAX_PALABRAS];
int n_palabras = 0;

pthread_mutex_t mutex_tablero;

void agregar(int fila, int col, int dir, int largo, char *resp, char *pista, char *alt_resp, char *alt_pista){
    int i = n_palabras;
    int k, r, c;

    filas[i] = fila;
    cols[i] = col;
    dirs[i] = dir;
    largos[i] = largo;
    respuestas[i] = resp;
    pistas[i] = pista;
    alt_respuestas[i] = alt_resp;
    alt_pistas[i] = alt_pista;
    adivinadas[i] = 0;

    for(k = 0; k < largo; k++){
        r = fila + (dir == 1 ? k : 0);
        c = col + (dir == 0 ? k : 0);
        activa[r][c] = 1;
    }

    n_palabras++;
}

// CARGA DE LETRAS DEL CRUCIGRAMA
void cargar_tablero(){
    int i, j;

    for(i = 0; i < ALTO; i++){
        for(j = 0; j < ANCHO; j++){
            tablero[i][j] = ' ';
            activa[i][j] = 0;
        }
    }
    n_palabras = 0;

    agregar(0, 3, 0, 5, "QUESO",    "Lacteo amarillo",      "FRENO",    "Dispositivo para reducir la velocidad de un coche");
    agregar(2, 3, 0, 5, "NOCHE",    "Cuando no hay sol",    "BUCLE",    "Estructura de control que repite un bloque de código mientras se cumpla una condición");
    agregar(4, 3, 0, 5, "SALSA",    "Baile picante",        "BOLSA",    "Donde guardas las compras");
    agregar(0, 5, 1, 8, "ESCALERA", "Para subir pisos (Singular)",     "ESCULTOR", "Artista que hace estatuas");
    agregar(0, 7, 1, 3, "ORE",      "Rezar en pasado",      "OYE",      "Escuchar en imperativo");
    agregar(4, 6, 1, 3, "SUR",      "Punto cardinal",       "SOL",      "Estrella visible durante el dia");
}

// IMPRESION DE TABLERO
void imprimir_tablero(){
    system("clear"); // Limpiar pantalla (ANSI)

    printf("\n    --- LA CASA DE HOJAS ---\n");

    for(int i = 0; i < ALTO; i++){
        // --- Fila de numeros verticales
        printf("    ");
        for(int j = 0; j < ANCHO; j++){
            int num_v = 0;
            for(int n = 0; n < n_palabras; n++) {
                if(filas[n] == i && cols[n] == j && dirs[n] == 1) {
                    num_v = n + 1;
                    break;
                }
            }
            if(num_v > 0) {
                printf(" %-2d", num_v);
            } else {
                printf("   ");
            }
        }
        printf("\n");

        //  Fila de numeros horizontales
        printf(" ");
        for(int j = 0; j < ANCHO; j++){
            if(j == 0) printf("   ");

            int n_h = 0;
            for(int n = 0; n < n_palabras; n++) {
                if(filas[n] == i && cols[n] == j && dirs[n] == 0) {
                    n_h = n + 1;
                    break;
                }
            }

            if(activa[i][j]){
                // Si hay inicio horizontal, pegamos el número a la izquierda
                if(n_h > 0) printf("\b\b%-2d", n_h);

                if(tablero[i][j] == ' '){
                     printf("[_]");
                } else {
                    printf("[%c]", tablero[i][j]);
                }
            } else {
                printf("   ");
            }
        }
        printf("\n");
    }

}


//Notificacion de cambio de palabra
void aviso_actualizacion(int sig) {
    printf("\n\a[NOTIFICACION]: ¡El tiempo se agoto! Una palabra no adivinada ha cambiado.\n");
    sleep(2);
    kill(getpid(),SIGUSR1);
}

// Control de tiempo de cambio
void* hilo_cronometro(void* arg) {
    while(1) {
        // Tiempo aleatorio entre 45 y 60 segundos
        int espera = (rand() % 16) + 45;
        sleep(espera);

        pthread_mutex_lock(&mutex_tablero);
        // Identificar cuales no se han adivinado
        int disponibles[MAX_PALABRAS];
        int count = 0;
        for(int i = 0; i < n_palabras; i++) {
            if(!adivinadas[i]) {
                disponibles[count++] = i;
            }
        }

        // Si hay palabras disponibles, elegir una al azar
        if(count > 0) {
            int elegido = disponibles[rand() % count];

            // Intercambio de palabras
            char *temp_r = respuestas[elegido];
            char *temp_p = pistas[elegido];

            respuestas[elegido] = alt_respuestas[elegido];
            pistas[elegido] = alt_pistas[elegido];

            alt_respuestas[elegido] = temp_r;
            alt_pistas[elegido] = temp_p;
        }
        pthread_mutex_unlock(&mutex_tablero);

        kill(getpid(), SIGALRM); // Enviamos señal de alarma al propio proceso
    }
    return NULL;
}

// VERIFICACION DE RESPUESTA DEL USUARIO
void pedir_palabra() {
    int num;
    char intento[20];

    printf("\nIntroduce el numero de la palabra que quieres adivinar\n");


    if (scanf("%d", &num) != 1) {
    printf("Entrada inválida\n");
    sleep(1);
    while(getchar() != '\n'); // limpiar buffer
    return;
}

    // Validar que el numero sea correcto
    if (num < 1 || num > n_palabras) {
        printf("Numero no valido.\n");
        sleep(1);
        return;
    }

    int i = num - 1; // Ajustamos al indice del arreglo (0 a 5)

    pthread_mutex_lock(&mutex_tablero);
    if (adivinadas[i]) {
        printf("¡Esa palabra ya la adivinaste!\n");
        sleep(1);
        pthread_mutex_unlock(&mutex_tablero);
        return;
    }

    printf("Pista: %s (%d letras)\n", pistas[i], largos[i]);
    printf("Tu respuesta:\n");
    scanf("%19s", intento);
    printf("\n ");

    // Convertir a mayusculas para evitar errores
    for(int j = 0; intento[j]; j++) {
        if(intento[j] >= 'a' && intento[j] <= 'z') intento[j] -= 32;
    }

    if (strcmp(intento, respuestas[i]) == 0) {
        printf("¡CORRECTO!\n");
        adivinadas[i] = 1;

        // Escribir la palabra en el tablero visual
        for (int k = 0; k < largos[i]; k++) {
            int r = filas[i] + (dirs[i] == 1 ? k : 0);
            int c = cols[i] + (dirs[i] == 0 ? k : 0);
            tablero[r][c] = respuestas[i][k];
        }
        sleep(1);
    } else {
        printf("Incorrecto. Sigue intentando.\n");
        sleep(1);
    }
    pthread_mutex_unlock(&mutex_tablero);

    sleep(1);
}

// IMPRESION DE PISTAS
void imprimir_pistas(){
    int i;

    printf("Horizontales:\n");
    for(i = 0; i < n_palabras; i++){
        if(dirs[i] == 0){
            printf("  %d. %s (%d letras)\n", i+1, pistas[i], largos[i]);
        }
    }

    printf("\nVerticales:\n");
    for(i = 0; i < n_palabras; i++){
        if(dirs[i] == 1){
            printf("  %d. %s (%d letras)\n", i+1, pistas[i], largos[i]);
        }
    }
}
void imprimir (int sig) {
    pthread_mutex_lock(&mutex_tablero);
    imprimir_tablero();
    pthread_mutex_unlock(&mutex_tablero);
    imprimir_pistas();
}
// JUEGO
void jugar() {
    pthread_t thread_id;
    signal(SIGALRM, aviso_actualizacion);
    srand(time(NULL));

    // Crear hilo para el cronómetro
    int hil = pthread_create(&thread_id, NULL, hilo_cronometro, NULL);
    if(hil != 0)  {
        perror("Error al crear hilo");
        return;
    }

    while(1) {
        kill(getpid(),SIGUSR1);

        pedir_palabra();

        int todas = 1;
        pthread_mutex_lock(&mutex_tablero);
        for(int i = 0; i < n_palabras; i++) {
            if(!adivinadas[i]) todas = 0;
        }
        pthread_mutex_unlock(&mutex_tablero);

        // Condicion de victoria
        if(todas) {
            imprimir_tablero();
            printf("\n¡FELICIDADES! Has completado La Casa de Hojas.\n");
            break;
        }
    }
}


int main(){
    pid_t pid = fork();
    int status;
    signal(SIGUSR1,imprimir);
    pthread_mutex_init(&mutex_tablero, NULL);
    if (pid < 0) {
        perror("Error en fork");
        return 1;
    }
    else if (pid == 0) {
        // El proceso hijo ejecuta el juego
        cargar_tablero();
        printf("\n                   La Casa de Hojas               \n");
        printf("====================================================\n");
        printf("\n------------------- INSTRUCCIONES -------------------\n");
        printf("- Las palabras se actualizaran en cualquier momento \n  entre 45 segundos y 60 segundos de manera aleatoria.\n");
        printf("- Las palabras adivinadas no se actualizaran.\n");
        printf("- Al seleccionar una palabra para adivinar no se actualizara \n  el tablero hasta fallar o adivinar, pero el tiempo seguira\n");
        printf("- Recibira una notificacion con cada actualizacion.\n");
        printf("- Presiona ENTER para comenzar.\n");
        printf("------------------------------------------------------\n");
        getchar();
        jugar();
        exit(0);
    }
    else {

        waitpid(pid, &status, 0);
        printf("\nEl juego ha terminado. ¡Gracias por jugar!\n");
    }

    pthread_mutex_destroy(&mutex_tablero);
    return 0;
}
