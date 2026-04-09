#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symtab.h"

void init_pilha(PilhaTabela *p) {
    p->topo = NULL;
}

void push_escopo(PilhaTabela *p) {
    Escopo *e = malloc(sizeof(Escopo));
    if (!e) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
    e->entradas = NULL;
    e->anterior = p->topo;
    p->topo = e;
}

void inserir(PilhaTabela *p, const char *nome, TipoNo tipo) {
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

void pop_escopo(PilhaTabela *p) {
    if (!p->topo) return;
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
