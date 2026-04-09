#include <stdio.h>
#include <stdlib.h>
#include "../include/semantico.h"
#include "../include/symtab.h"

/* ---------------------------------------------------------------- */
static void erro_sem(const char *msg, int linha) {
    printf("ERRO: %s linha %d\n", msg, linha);
    exit(1);
}

/* Protótipos internos */
static void   analisar_bloco     (No *no, PilhaTabela *p);
static void   analisar_lista_cmd (No *no, PilhaTabela *p);
static void   analisar_comando   (No *no, PilhaTabela *p);
TipoNo        analisar_expr_tipo (No *no, PilhaTabela *p);

/* ----------------------------------------------------------------
 * Insere todas as variáveis de uma NO_LISTA_DECL no escopo do topo.
 * Cada NO_DECL_VAR tem:
 *   esq  = NO_IDENT com 1º nome
 *   dir  = NO_TIPO_INT ou NO_TIPO_CAR
 *   extra = NO_LISTA_DECL com nomes adicionais (vírgula)
 * ---------------------------------------------------------------- */
static void inserir_decls(No *lista, PilhaTabela *p) {
    for (No *ld = lista; ld != NULL; ld = ld->dir) {
        No *decl = ld->esq;                  /* NO_DECL_VAR              */
        TipoNo tipo = decl->dir->tipo;       /* NO_TIPO_INT ou NO_TIPO_CAR */

        /* 1º nome */
        inserir(p, decl->esq->valor, tipo);

        /* nomes adicionais após vírgula */
        for (No *extra = decl->extra; extra != NULL; extra = extra->dir)
            inserir(p, extra->esq->valor, tipo);
    }
}

/* ---------------------------------------------------------------- */
static void analisar_bloco(No *no, PilhaTabela *p) {
    int tem_decls = (no->esq != NULL);

    if (tem_decls) {
        push_escopo(p);
        inserir_decls(no->esq, p);
    }

    analisar_lista_cmd(no->dir, p);

    if (tem_decls)
        pop_escopo(p);
}

/* ---------------------------------------------------------------- */
static void analisar_lista_cmd(No *no, PilhaTabela *p) {
    for (No *lc = no; lc != NULL; lc = lc->dir)
        analisar_comando(lc->esq, p);
}

/* ---------------------------------------------------------------- */
static void analisar_comando(No *no, PilhaTabela *p) {
    if (!no) return;

    switch (no->tipo) {
        case NO_CMD_EXPR:
            analisar_expr_tipo(no->esq, p);
            break;

        case NO_CMD_LEIA:
            if (!buscar(p, no->valor))
                erro_sem("variavel nao declarada", no->linha);
            break;

        case NO_CMD_ESCREVA:
            analisar_expr_tipo(no->esq, p);
            break;

        case NO_CMD_ESCREVA_STR:
        case NO_CMD_NOVALINHA:
            break;

        case NO_CMD_SE:
            analisar_expr_tipo(no->esq, p);
            analisar_comando(no->dir, p);
            if (no->extra)
                analisar_comando(no->extra, p);
            break;

        case NO_CMD_ENQUANTO:
            analisar_expr_tipo(no->esq, p);
            analisar_comando(no->dir, p);
            break;

        case NO_BLOCO:
            analisar_bloco(no, p);
            break;

        default:
            break;
    }
}

/* ---------------------------------------------------------------- */
TipoNo analisar_expr_tipo(No *no, PilhaTabela *p) {
    if (!no) return NO_TIPO_INT; /* não deve ocorrer */

    TipoNo t1, t2;

    switch (no->tipo) {

        case NO_INT_CONST:
            return NO_TIPO_INT;

        case NO_CAR_CONST:
            return NO_TIPO_CAR;

        case NO_IDENT: {
            EntradaTabela *e = buscar(p, no->valor);
            if (!e)
                erro_sem("variavel nao declarada", no->linha);
            return e->tipo;
        }

        case NO_ATRIB: {
            EntradaTabela *e = buscar(p, no->esq->valor);
            if (!e)
                erro_sem("variavel nao declarada", no->esq->linha);
            t1 = e->tipo;
            t2 = analisar_expr_tipo(no->dir, p);
            if (t1 != t2)
                erro_sem("tipos incompativeis na atribuicao", no->linha);
            return t1;
        }

        /* Operadores aritméticos: exigem int, retornam int */
        case NO_OP_SOMA:
        case NO_OP_SUB:
        case NO_OP_MUL:
        case NO_OP_DIV:
            t1 = analisar_expr_tipo(no->esq, p);
            t2 = analisar_expr_tipo(no->dir, p);
            if (t1 != NO_TIPO_INT || t2 != NO_TIPO_INT)
                erro_sem("operador aritmetico requer int", no->linha);
            return NO_TIPO_INT;

        /* Menos unário: exige int, retorna int */
        case NO_OP_NEG:
            t1 = analisar_expr_tipo(no->esq, p);
            if (t1 != NO_TIPO_INT)
                erro_sem("operador aritmetico requer int", no->linha);
            return NO_TIPO_INT;

        /* Operadores relacionais: mesmo tipo nos dois lados, retornam int */
        case NO_OP_MENOR:
        case NO_OP_MAIOR:
        case NO_OP_MENIG:
        case NO_OP_MAIIG:
        case NO_OP_IGUAL:
        case NO_OP_DIFER:
            t1 = analisar_expr_tipo(no->esq, p);
            t2 = analisar_expr_tipo(no->dir, p);
            if (t1 != t2)
                erro_sem("operandos de tipos diferentes", no->linha);
            return NO_TIPO_INT;

        /* Operadores lógicos: exigem int, retornam int */
        case NO_OP_OU:
        case NO_OP_E:
            t1 = analisar_expr_tipo(no->esq, p);
            t2 = analisar_expr_tipo(no->dir, p);
            if (t1 != NO_TIPO_INT || t2 != NO_TIPO_INT)
                erro_sem("operador logico requer int", no->linha);
            return NO_TIPO_INT;

        /* Negação lógica: exige int, retorna int */
        case NO_OP_NAO:
            t1 = analisar_expr_tipo(no->esq, p);
            if (t1 != NO_TIPO_INT)
                erro_sem("operador logico requer int", no->linha);
            return NO_TIPO_INT;

        default:
            return NO_TIPO_INT;
    }
}

/* ----------------------------------------------------------------
 * Ponto de entrada
 * ---------------------------------------------------------------- */
void analisar_semantico(No *raiz) {
    PilhaTabela p;
    init_pilha(&p);
    /* raiz é NO_PROGRAMA; esq é o NO_BLOCO principal */
    analisar_bloco(raiz->esq, &p);
}
