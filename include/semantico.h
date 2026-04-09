#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "ast.h"
#include "symtab.h"

/* Percorre a AST verificando escopos e tipos.
 * Imprime "ERRO: <msg> linha <N>" e termina se encontrar erro. */
void analisar_semantico(No *raiz);

/* Retorna o tipo de uma expressão (NO_TIPO_INT ou NO_TIPO_CAR).
 * Usada pelo gerador de código para escolher a instrução correta. */
TipoNo analisar_expr_tipo(No *no, PilhaTabela *p);

#endif
