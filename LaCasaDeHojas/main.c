#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>

#define ALTO 8
#define ANCHO 12
#define MAX_PALABRAS 10
#define MAX_RESP 32

char tablero[ALTO][ANCHO];
int activa[ALTO][ANCHO];

int filas[MAX_PALABRAS];
int cols[MAX_PALABRAS];
int dirs[MAX_PALABRAS];          // 0 horizontal, 1 vertical
int largos[MAX_PALABRAS];
int adivinadas[MAX_PALABRAS];
int n_palabras = 0;

char *respuestas[MAX_PALABRAS];
char *pistas[MAX_PALABRAS];

int idx_cambio = -1;
char nueva_palabra[MAX_RESP];

int num_pregunta = -1;
char respuesta[MAX_RESP];

int segundos = 0;
int juego_activo = 1;

pthread_mutex_t mutex;

pid_t pid_gestor;
pid_t pid_input;

int flag_cambio = 0;
int flag_input = 0;
int flag_salida = 0;


void agregar(int fila, int col, int dir, int largo, char *resp, char *pista){
    int i = n_palabras;
    int k, r, c;

    filas[i] = fila;
    cols[i] = col;
    dirs[i] = dir;
    largos[i] = largo;
    respuestas[i] = resp;
    pistas[i] = pista;
    adivinadas[i] = 0;

    for(k = 0; k < largo; k++){
        r = fila + (dir == 1 ? k : 0);
        c = col + (dir == 0 ? k : 0);
        activa[r][c] = 1;
    }

    n_palabras++;
}


void cargar_tablero(){
    int i, j;

    for(i = 0; i < ALTO; i++){
        for(j = 0; j < ANCHO; j++){
            tablero[i][j] = ' ';
            activa[i][j] = 0;
        }
    }

    agregar(0, 3, 0, 5, "QUESO",    "lacteo amarillo");
    agregar(2, 3, 0, 5, "NOCHE",    "cuando no hay sol");
    agregar(4, 3, 0, 5, "SALSA",    "baile y comida picante");
    agregar(0, 5, 1, 8, "ESCALERA", "para subir pisos");
    agregar(0, 7, 1, 3, "ORE",      "rezar en pasado");
    agregar(4, 6, 1, 3, "SUR",      "punto cardinal");
}


void imprimir_tablero(){
    int i, j;
    char c;

    printf("\n");
    for(i = 0; i < ALTO; i++){
        printf("  ");
        for(j = 0; j < ANCHO; j++){
            if(activa[i][j]){
                c = tablero[i][j];
                if(c == ' '){
                    printf("[_]");
                } else {
                    printf("[%c]", c);
                }
            } else {
                printf("   ");
            }
        }
        printf("\n");
    }
    printf("\n");
}


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


// Falta
void aplicar_cambio(int idx, char *nueva){

}


void validar_respuesta(int num, char *resp){
    int i = num - 1;
    int k, r, c;

    if(i < 0 || i >= n_palabras) return;
    if(adivinadas[i] == 1) return;

    if(strcasecmp(resp, respuestas[i]) == 0){
        adivinadas[i] = 1;
        for(k = 0; k < largos[i]; k++){
            r = filas[i] + (dirs[i] == 1 ? k : 0);
            c = cols[i] + (dirs[i] == 0 ? k : 0);
            tablero[r][c] = respuestas[i][k];
        }
    }
}


void al_cambio(int sig){
    flag_cambio = 1;
}

void al_input(int sig){
    flag_input = 1;
}

void al_salida(int sig){
    flag_salida = 1;
}


void redibujar(){
    int i, todas = 1;

    printf("\n=== La Casa de Hojas (t = %ds) ===\n", segundos);
    imprimir_tablero();
    imprimir_pistas();

    for(i = 0; i < n_palabras; i++){
        if(adivinadas[i] == 0) todas = 0;
    }
    if(todas == 1){
        printf("\nDescubriste todas las palabras.\n");
        juego_activo = 0;
    }
}


void* hilo_dibujo(void* arg){
    while(juego_activo == 1 && flag_salida == 0){
        sleep(1);

        pthread_mutex_lock(&mutex);

        segundos++;

        if(flag_cambio == 1){
            flag_cambio = 0;
            if(idx_cambio >= 0){
                aplicar_cambio(idx_cambio, nueva_palabra);
                idx_cambio = -1;
                printf("\n>> Cambio una palabra.\n");
                redibujar();
            }
        }

        if(flag_input == 1){
            flag_input = 0;
            if(num_pregunta > 0){
                validar_respuesta(num_pregunta, respuesta);
                num_pregunta = -1;
                redibujar();
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}


int main(){
    pthread_t hilo;
    int status;

    pthread_mutex_init(&mutex, 0);

    cargar_tablero();

    signal(SIGUSR1, al_cambio);
    signal(SIGUSR2, al_input);
    signal(SIGINT, al_salida);

    pid_gestor = fork();
    if(pid_gestor == 0){
        // Falta
        while(1) pause();
        exit(0);
    }

    pid_input = fork();
    if(pid_input == 0){
        // Falta
        while(1) pause();
        exit(0);
    }

    pthread_mutex_lock(&mutex);
    redibujar();
    pthread_mutex_unlock(&mutex);

    pthread_create(&hilo, NULL, hilo_dibujo, NULL);

    while(juego_activo == 1 && flag_salida == 0){
        pause();
    }

    juego_activo = 0;
    pthread_join(hilo, NULL);

    kill(pid_gestor, SIGTERM);
    kill(pid_input, SIGTERM);
    waitpid(pid_gestor, &status, 0);
    waitpid(pid_input, &status, 0);

    pthread_mutex_destroy(&mutex);

    printf("Fin del juego.\n");
    return 0;
}
