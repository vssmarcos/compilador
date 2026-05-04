#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symtab.h"

void init_pilha(PilhaTabela *p) {
    p->topo = NULL;
}

// Chamada toda vez que entra num escopo {. Cria um espoco novo, vazio e coloca no topo
void push_escopo(PilhaTabela *p) {
    Escopo *e = malloc(sizeof(Escopo)); // memoria de um novo escopo
    if (!e) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
    e->entradas = NULL;
    e->anterior = p->topo; // novo anterior
    p->topo = e; // novo topo
}
// Inserção no escopo atual
void inserir(PilhaTabela *p, const char *nome, TipoNo tipo) {
    /* Adiciona uma variável recém declarada na tabela de símbolos do escopo atual */
    /* Recebe: p -> pilha de escopos; nome -> o lexema do identificador; tipo -> int/car */
    /* Checa redeclaração no mesmo escopo (verifica no bloco atual se tem nome igual a uma declarada)*/
    for (EntradaTabela *e = p->topo->entradas; e != NULL; e = e->prox) {
        if (strcmp(e->nome, nome) == 0) {
            printf("ERRO: variavel '%s' ja declarada no mesmo escopo\n", nome);
            exit(1);
        }
    }
    // Cria uma entrada nova
    EntradaTabela *ent = malloc(sizeof(EntradaTabela));
    if (!ent) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
    ent->nome   = strdup(nome);
    ent->tipo   = tipo;
    ent->offset = 0;
    ent->prox   = p->topo->entradas;
    p->topo->entradas = ent;
}

EntradaTabela *buscar(PilhaTabela *p, const char *nome) {
    for (Escopo *e = p->topo; e != NULL; e = e->anterior) {
        for (EntradaTabela *ent = e->entradas; ent != NULL; ent = ent->prox) {
            if (strcmp(ent->nome, nome) == 0)
                return ent;
        }
    }
    return NULL;
}

// Chamada ao sair de um bloco }, faz duas limpezas
void pop_escopo(PilhaTabela *p) {
    if (!p->topo) return;
    // pego escopo atual   
    Escopo *e = p->topo;
    EntradaTabela *ent = e->entradas;
    while (ent) {
        EntradaTabela *prox = ent->prox;
        free(ent->nome);
        free(ent);
        ent = prox;
    }
    p->topo = e->anterior;
    free(e);
}
