#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ast.h"
/*
Arvore sintatica abstrata é a representação do codigo fonte em memória
depois do parsing. Cada construção vira um nó na árvore
*/
/*----------------------------------------------------------------
 * criar_no: aloca um nó com dois filhos
 *----------------------------------------------------------------*/
No *criar_no(TipoNo tipo, int linha, No *esq, No *dir) {
    No *no = malloc(sizeof(No)); //aloca mempria
    if (!no) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
    no->tipo  = tipo;
    no->linha = linha;
    no->valor = NULL;
    no->esq   = esq;
    no->dir   = dir;
    no->extra = NULL;
    return no;
}

/*----------------------------------------------------------------
 * criar_no3: aloca um nó com três filhos
 * Usado em: NO_CMD_SE (cond / entao / senao)
 *           NO_BLOCO  (varSection / listaCmd / NULL)
 *----------------------------------------------------------------*/
No *criar_no3(TipoNo tipo, int linha, No *esq, No *dir, No *extra) {
    No *no = criar_no(tipo, linha, esq, dir);
    no->extra = extra;
    return no;
}

/*----------------------------------------------------------------
 * criar_no_folha: aloca um nó sem filhos, mas com valor textual
 * Usado em: NO_IDENT, NO_INT_CONST, NO_CAR_CONST, NO_CADEIA,
 *           NO_CMD_LEIA, NO_DECL_VAR
 *----------------------------------------------------------------*/
No *criar_no_folha(TipoNo tipo, int linha, const char *valor) {
    No *no = criar_no(tipo, linha, NULL, NULL);
    if (valor) {
        no->valor = malloc(strlen(valor) + 1);
        if (!no->valor) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
        strcpy(no->valor, valor);
    }
    return no;
}

/*----------------------------------------------------------------
 * liberar_ast: percorre a árvore e libera toda a memória
 * 
 *----------------------------------------------------------------*/
void liberar_ast(No *no) {
    if (!no) return;
    liberar_ast(no->esq);
    liberar_ast(no->dir);
    liberar_ast(no->extra);
    if (no->valor) free(no->valor);
    free(no);
}

/*----------------------------------------------------------------
 * nome_tipo: converte TipoNo para string (para depuração)
 *----------------------------------------------------------------*/
static const char *nome_tipo(TipoNo tipo) {
    switch (tipo) {
        case NO_PROGRAMA:        return "PROGRAMA";
        case NO_BLOCO:           return "BLOCO";
        case NO_LISTA_CMD:       return "LISTA_CMD";
        case NO_LISTA_DECL:      return "LISTA_DECL";
        case NO_DECL_VAR:        return "DECL_VAR";
        case NO_TIPO_INT:        return "TIPO_INT";
        case NO_TIPO_CAR:        return "TIPO_CAR";
        case NO_CMD_EXPR:        return "CMD_EXPR";
        case NO_CMD_LEIA:        return "CMD_LEIA";
        case NO_CMD_ESCREVA:     return "CMD_ESCREVA";
        case NO_CMD_ESCREVA_STR: return "CMD_ESCREVA_STR";
        case NO_CMD_NOVALINHA:   return "CMD_NOVALINHA";
        case NO_CMD_SE:          return "CMD_SE";
        case NO_CMD_ENQUANTO:    return "CMD_ENQUANTO";
        case NO_ATRIB:           return "ATRIB";
        case NO_OP_OU:           return "OP_OU";
        case NO_OP_E:            return "OP_E";
        case NO_OP_IGUAL:        return "OP_IGUAL";
        case NO_OP_DIFER:        return "OP_DIFER";
        case NO_OP_MENOR:        return "OP_MENOR";
        case NO_OP_MAIOR:        return "OP_MAIOR";
        case NO_OP_MENIG:        return "OP_MENIG";
        case NO_OP_MAIIG:        return "OP_MAIIG";
        case NO_OP_SOMA:         return "OP_SOMA";
        case NO_OP_SUB:          return "OP_SUB";
        case NO_OP_MUL:          return "OP_MUL";
        case NO_OP_DIV:          return "OP_DIV";
        case NO_OP_NEG:          return "OP_NEG";
        case NO_OP_NAO:          return "OP_NAO";
        case NO_IDENT:           return "IDENT";
        case NO_INT_CONST:       return "INT_CONST";
        case NO_CAR_CONST:       return "CAR_CONST";
        case NO_CADEIA:          return "CADEIA";
        default:                 return "???";
    }
}

/*----------------------------------------------------------------
 * imprimir_ast: imprime a árvore com indentação
 * Útil para depuração — mostra a estrutura gerada
 *----------------------------------------------------------------*/
void imprimir_ast(No *no, int nivel) {
    if (!no) return;

    /* indentação: 2 espaços por nível */
    for (int i = 0; i < nivel; i++) printf("  ");

    /* imprime tipo e valor se houver */
    if (no->valor)
        printf("[%s] \"%s\" (linha %d)\n", nome_tipo(no->tipo), no->valor, no->linha);
    else
        printf("[%s] (linha %d)\n", nome_tipo(no->tipo), no->linha);

    /* percorre filhos */
    imprimir_ast(no->esq,   nivel + 1);
    imprimir_ast(no->dir,   nivel + 1);
    imprimir_ast(no->extra, nivel + 1);
}