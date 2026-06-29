#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

/*================================================================
 * symtab.h — Tabela de Símbolos para G-V1 e G-V2
 *
 * A tabela é uma pilha de escopos. Cada escopo é uma lista ligada
 * de entradas. G-V2 adiciona suporte a vetores e funções.
 *================================================================*/

/*----------------------------------------------------------------
 * Tipo da entrada: diferencia variável, vetor, função e parâmetro
 *----------------------------------------------------------------*/
typedef enum {
    KIND_VAR,       /* variável escalar local ou global    */
    KIND_VET,       /* vetor local ou global               */
    KIND_FUNC,      /* nome de função                      */
    KIND_PARAM,     /* parâmetro escalar                   */
    KIND_PARAM_VET  /* parâmetro vetor (passa por referência) */
} TipoKind;

/*----------------------------------------------------------------
 * InfoParam — metadados de um parâmetro formal de função.
 * Usado dentro de EntradaTabela quando kind == KIND_FUNC.
 *----------------------------------------------------------------*/
typedef struct InfoParam {
    char             *nome;      /* nome do parâmetro        */
    TipoNo            tipo;      /* NO_TIPO_INT ou NO_TIPO_CAR */
    int               eh_vetor;  /* 1 se é vetor, 0 se escalar */
    struct InfoParam *prox;      /* próximo parâmetro         */
} InfoParam;

/*----------------------------------------------------------------
 * EntradaTabela — uma entrada na tabela de símbolos
 *
 * Campos usados conforme o kind:
 *   KIND_VAR / KIND_PARAM:    nome, tipo, offset, is_global
 *   KIND_VET / KIND_PARAM_VET: nome, tipo, offset, tam, is_global
 *   KIND_FUNC:                 nome, tipo (retorno), num_params, params
 *----------------------------------------------------------------*/
typedef struct EntradaTabela {
    char             *nome;       /* lexema do identificador         */
    TipoKind          kind;       /* qual tipo de símbolo é          */
    TipoNo            tipo;       /* tipo base (ou tipo de retorno)  */
    int               offset;     /* offset em bytes relativo a $fp  */
    int               tam;        /* tamanho do vetor (KIND_VET)     */
    int               is_global;  /* 1 = var global (.data), 0 = local */
    int               num_params; /* número de parâmetros (KIND_FUNC) */
    InfoParam        *params;     /* lista de parâmetros (KIND_FUNC)  */
    struct EntradaTabela *prox;   /* próxima entrada no mesmo escopo  */
} EntradaTabela;

/*----------------------------------------------------------------
 * Escopo e pilha de escopos
 *----------------------------------------------------------------*/
typedef struct Escopo {
    EntradaTabela *entradas;  /* lista de entradas deste escopo */
    struct Escopo *anterior;  /* escopo abaixo na pilha         */
} Escopo;

typedef struct {
    Escopo *topo;
} PilhaTabela;

/*----------------------------------------------------------------
 * Operações sobre a pilha
 *----------------------------------------------------------------*/
void           init_pilha   (PilhaTabela *p);
void           push_escopo  (PilhaTabela *p);
void           pop_escopo   (PilhaTabela *p);
EntradaTabela *buscar       (PilhaTabela *p, const char *nome);

/* Inserção de variável escalar (KIND_VAR) — mantém compatibilidade G-V1 */
void inserir(PilhaTabela *p, const char *nome, TipoNo tipo);

/* G-V2: inserção de vetor (KIND_VET) */
void inserir_vetor(PilhaTabela *p, const char *nome, TipoNo tipo, int tam);

/* G-V2: inserção de função (KIND_FUNC) */
void inserir_func(PilhaTabela *p, const char *nome, TipoNo tipo_ret,
                  int num_params, InfoParam *params);

/* G-V2: inserção de parâmetro escalar ou vetor */
void inserir_parametro(PilhaTabela *p, const char *nome, TipoNo tipo, int eh_vetor);

#endif
