#include "escalonador.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Escalonador* escalonador_ativo = NULL;

void wrapper_tarefa() {
    // chama a funcao q sera executada
    escalonador_ativo->tarefa_atual->rotina();

    printf("A tarefa %d foi finalizada.\n", escalonador_ativo->tarefa_atual->id);
    escalonador_ativo->tarefa_atual->estado = FINALIZADA;

    return;
}

void criar_tarefa(RotinaTarefa rotina)
{
    if (escalonador_ativo->contador >= TAMANHO_FILA) {
        printf("Numero maximo de tarefas atingido\n");
        return;
    }

    Tarefa* nova_tarefa = &escalonador_ativo->fila[escalonador_ativo->contador];

    nova_tarefa->id = escalonador_ativo->contador;
    nova_tarefa->estado = PRONTA;
    nova_tarefa->rotina = rotina;

    //salva o contexto atual da CPU (serve para inicializar tudo, pois ainda so contem lixo)
    getcontext(&nova_tarefa->contexto);

    //MUITO IMPORTANTE: aloca a pilha de cada tarefa
    void* pilha = malloc(TAMANHO_PILHA);
    
    nova_tarefa->contexto.uc_stack.ss_sp = pilha;
    nova_tarefa->contexto.uc_stack.ss_size = TAMANHO_PILHA;
    nova_tarefa->contexto.uc_link = &escalonador_ativo->contexto;

    makecontext(&nova_tarefa->contexto, (void (*)(void))wrapper_tarefa, 0);

    escalonador_ativo->contador++;

    printf("Tarefa %d criada.\n", nova_tarefa->id);
}

// round-robin cooperativo
static Tarefa* encontrar_tarefa() {
    // determina onde comecar a busca
    int comeco = (escalonador_ativo->tarefa_atual == NULL) ? 0 : (escalonador_ativo->tarefa_atual->id + 1);

    for (int i = 0; i < escalonador_ativo->contador; ++i) {
        // busca circular
        int index = (comeco + i) % escalonador_ativo->contador;

        if (escalonador_ativo->fila[index].estado == PRONTA) {
            // retorna a primeira tarefa pronta para executar
            return &escalonador_ativo->fila[index];
        }
    }

    // n tem tarefa pronta, retorna VAZIO
    return NULL;
}

void inicializar_escalonador(Escalonador* escalonador) {
    escalonador->contador = 0;
    escalonador->tarefa_atual = NULL;

    for (int i = 0; i < TAMANHO_FILA; ++i) {
        escalonador->fila[i].estado = FINALIZADA;
    }

    escalonador_ativo = escalonador;
    printf("Escalonador iniciado.\n");
}

void escalonador() {
    while (1) {
        Tarefa* prox_tarefa = encontrar_tarefa();
        
        if (prox_tarefa) {
            escalonador_ativo->tarefa_atual = prox_tarefa;
            escalonador_ativo->tarefa_atual->estado = EXECUTANDO;

            //salva o contexto do escalonador e troca para o contexto da tarefa atual
            swapcontext(&escalonador_ativo->contexto, &escalonador_ativo->tarefa_atual->contexto);
        } else {
            printf("Todas as tarefas foram finalizadas.\n");
            break;
        }
    }
    escalonador_ativo->contador = 0;
    escalonador_ativo->tarefa_atual = NULL;
}

void yield() {
    escalonador_ativo->tarefa_atual->estado = PRONTA;

    // salva o estado atual e volta o contexto pro escalonador
    swapcontext(&escalonador_ativo->tarefa_atual->contexto, &escalonador_ativo->contexto);
}
