#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symtab.h"

/*================================================================
 * symtab.c — Implementação da Tabela de Símbolos para G-V1/G-V2
 *
 * Usa uma pilha de escopos. Cada escopo é uma lista ligada de
 * entradas. G-V2 adiciona vetores, funções e parâmetros.
 *================================================================*/

void init_pilha(PilhaTabela *p) {
    p->topo = NULL;
}

/* push_escopo: chamado ao entrar em um novo bloco ou função.
 * Cria um escopo vazio e empilha no topo. */
void push_escopo(PilhaTabela *p) {
    Escopo *e = malloc(sizeof(Escopo));
    if (!e) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
    e->entradas = NULL;
    e->anterior = p->topo;
    p->topo = e;
}

/* pop_escopo: chamado ao sair de um bloco. Remove e libera o escopo do topo. */
void pop_escopo(PilhaTabela *p) {
    if (!p->topo) return;
    Escopo *e = p->topo;
    EntradaTabela *ent = e->entradas;
    while (ent) {
        EntradaTabela *prox = ent->prox;
        free(ent->nome);
        /* Libera lista de parâmetros se for função */
        if (ent->kind == KIND_FUNC) {
            InfoParam *ip = ent->params;
            while (ip) {
                InfoParam *prox_ip = ip->prox;
                free(ip->nome);
                free(ip);
                ip = prox_ip;
            }
        }
        free(ent);
        ent = prox;
    }
    p->topo = e->anterior;
    free(e);
}

/* buscar: percorre a pilha do topo até a base procurando o nome.
 * Retorna a entrada encontrada ou NULL se não existir. */
EntradaTabela *buscar(PilhaTabela *p, const char *nome) {
    for (Escopo *e = p->topo; e != NULL; e = e->anterior)
        for (EntradaTabela *ent = e->entradas; ent != NULL; ent = ent->prox)
            if (strcmp(ent->nome, nome) == 0)
                return ent;
    return NULL;
}

/* --- Funções auxiliares internas --- */

/* Verifica redeclaração no escopo atual */
static void checar_redecl(PilhaTabela *p, const char *nome) {
    for (EntradaTabela *e = p->topo->entradas; e != NULL; e = e->prox) {
        if (strcmp(e->nome, nome) == 0) {
            printf("ERRO: '%s' ja declarado no mesmo escopo\n", nome);
            exit(1);
        }
    }
}

/* Aloca e inicializa uma entrada básica */
static EntradaTabela *nova_entrada(const char *nome, TipoKind kind, TipoNo tipo) {
    EntradaTabela *ent = malloc(sizeof(EntradaTabela));
    if (!ent) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
    ent->nome       = strdup(nome);
    ent->kind       = kind;
    ent->tipo       = tipo;
    ent->offset     = 0;
    ent->tam        = 0;
    ent->is_global  = 0;
    ent->num_params = 0;
    ent->params     = NULL;
    ent->prox       = NULL;
    return ent;
}

/* Empilha a entrada no escopo do topo */
static void empilhar_entrada(PilhaTabela *p, EntradaTabela *ent) {
    ent->prox = p->topo->entradas;
    p->topo->entradas = ent;
}

/*----------------------------------------------------------------
 * inserir: variável escalar (KIND_VAR). Compatível com G-V1.
 *----------------------------------------------------------------*/
void inserir(PilhaTabela *p, const char *nome, TipoNo tipo) {
    checar_redecl(p, nome);
    EntradaTabela *ent = nova_entrada(nome, KIND_VAR, tipo);
    empilhar_entrada(p, ent);
}

/*----------------------------------------------------------------
 * inserir_vetor: vetor local ou global (KIND_VET).
 *   tam = número de elementos
 *----------------------------------------------------------------*/
void inserir_vetor(PilhaTabela *p, const char *nome, TipoNo tipo, int tam) {
    checar_redecl(p, nome);
    EntradaTabela *ent = nova_entrada(nome, KIND_VET, tipo);
    ent->tam = tam;
    empilhar_entrada(p, ent);
}

/*----------------------------------------------------------------
 * inserir_func: insere o nome de uma função no escopo atual.
 *   tipo_ret  = NO_TIPO_INT ou NO_TIPO_CAR
 *   num_params = número de parâmetros
 *   params    = lista ligada de InfoParam (pode ser NULL)
 *
 * A tabela de funções é inserida no escopo "de fora" da função,
 * para que outras funções e o programa principal possam chamá-la.
 *----------------------------------------------------------------*/
void inserir_func(PilhaTabela *p, const char *nome, TipoNo tipo_ret,
                  int num_params, InfoParam *params) {
    checar_redecl(p, nome);
    EntradaTabela *ent = nova_entrada(nome, KIND_FUNC, tipo_ret);
    ent->num_params = num_params;
    ent->params     = params;
    empilhar_entrada(p, ent);
}

/*----------------------------------------------------------------
 * inserir_parametro: insere um parâmetro formal no escopo da função.
 *   eh_vetor = 1 para parâmetro vetor (passar por referência)
 *              0 para parâmetro escalar
 *----------------------------------------------------------------*/
void inserir_parametro(PilhaTabela *p, const char *nome, TipoNo tipo, int eh_vetor) {
    checar_redecl(p, nome);
    TipoKind kind = eh_vetor ? KIND_PARAM_VET : KIND_PARAM;
    EntradaTabela *ent = nova_entrada(nome, kind, tipo);
    empilhar_entrada(p, ent);
}
