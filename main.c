#include <stdio.h>
#include "escalonador.h"

void rotina_A() {
    for (int i = 1; i <= 4; i++) {
        printf("Tarefa A [passo %d]\n", i);
        yield();
    }
}

void rotina_B() {
    for (int i = 1; i <= 4; i++) {
        printf("\t-> Tarefa B [passo %d]\n", i);
        yield();
        rotina_A();
    }
}

void rotina_C() {
    for (int i = 1; i <= 4; i++) {
        printf("\t\t--> Tarefa C [passo %d]\n", i);
        yield();
    }
}

void rotina_D() {
    for (int i = 1; i <= 4; i++) {
        printf("\t\t\t---> Tarefa D [passo %d]\n", i);
        yield();
    }
}

void rotina_E() {
    for (int i = 1; i <= 4; i++) {
        printf("\t\t\t\t----> Tarefa E [passo %d]\n", i);
        yield();
    }
}

int main(void) {
    Escalonador meu_escalonador;
    inicializar_escalonador(&meu_escalonador);

    printf("Criando 5 tarefas (A, B, C, D, E)...\n");
    criar_tarefa(rotina_A);
    criar_tarefa(rotina_B);
    criar_tarefa(rotina_C);
    criar_tarefa(rotina_D);
    criar_tarefa(rotina_E);

    printf("\n--- Iniciando Escalonador ---\n");
    escalonador();
    printf("--- Fim da Execução ---\n");
    
    return 0;
}