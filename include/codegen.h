#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/* Percorre a AST e emite assembly MIPS para stdout.
 * Deve ser chamado após analisar_semantico(). */
void gerar_codigo(No *raiz);

#endif
