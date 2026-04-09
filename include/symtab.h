#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

/* Uma entrada na tabela: identificador + tipo */
typedef struct EntradaTabela {
    char                *nome;   /* lexema do identificador          */
    TipoNo               tipo;   /* NO_TIPO_INT ou NO_TIPO_CAR       */
    int                  offset; /* offset em bytes relativo a $fp   */
    struct EntradaTabela *prox;  /* próxima entrada no mesmo escopo  */
} EntradaTabela;

/* Um escopo = uma tabela de símbolos (lista ligada de entradas) */
typedef struct Escopo {
    EntradaTabela *entradas;  /* lista de entradas deste escopo */
    struct Escopo *anterior;  /* escopo abaixo na pilha         */
} Escopo;

/* A pilha de escopos */
typedef struct {
    Escopo *topo;
} PilhaTabela;

/* Inicializa a pilha vazia */
void init_pilha(PilhaTabela *p);

/* Cria novo escopo e empilha no topo */
void push_escopo(PilhaTabela *p);

/* Insere identificador no escopo do topo */
void inserir(PilhaTabela *p, const char *nome, TipoNo tipo);

/* Pesquisa do topo à base; retorna entrada ou NULL */
EntradaTabela *buscar(PilhaTabela *p, const char *nome);

/* Remove e libera o escopo do topo */
void pop_escopo(PilhaTabela *p);

#endif
