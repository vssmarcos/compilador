#ifndef AST_H
#define AST_H

/*================================================================
 * ast.h — Definição dos nós da Árvore Sintática Abstrata (AST)
 *
 * Cada construção da linguagem G-V1 vira um nó com tipo próprio.
 * Todos os nós compartilham a mesma struct No, mas usam campos
 * diferentes dependendo do tipo.
 *================================================================*/

/*----------------------------------------------------------------
 * Tipos de nós — um para cada construção da linguagem
 *----------------------------------------------------------------*/
typedef enum {

    /* Estrutura do programa */
    NO_PROGRAMA,       /* raiz da árvore: esq = DeclPrograma          */
    NO_BLOCO,          /* esq = VarSection (ou NULL), dir = ListaCmd  */
    NO_LISTA_CMD,      /* esq = Comando, dir = próximo (lista ligada)  */

    /* Declarações de variáveis */
    NO_LISTA_DECL,     /* esq = DeclVar, dir = próxima (lista ligada) */
    NO_DECL_VAR,       /* valor = nome, dir = tipo (NO_TIPO_INT/CAR)  */
    NO_TIPO_INT,       /* folha: representa o tipo int                 */
    NO_TIPO_CAR,       /* folha: representa o tipo car                 */

    /* Comandos */
    NO_CMD_EXPR,       /* esq = Expr                                   */
    NO_CMD_LEIA,       /* valor = nome do identificador                */
    NO_CMD_ESCREVA,    /* esq = Expr                                   */
    NO_CMD_ESCREVA_STR,/* valor = cadeia de caracteres                 */
    NO_CMD_NOVALINHA,  /* folha                                        */
    NO_CMD_SE,         /* esq = cond, dir = cmd_entao, extra = cmd_senao (ou NULL) */
    NO_CMD_ENQUANTO,   /* esq = cond, dir = cmd_corpo                  */

    /* Expressões — operadores binários */
    NO_ATRIB,          /* esq = IDENT (destino), dir = Expr (valor)    */
    NO_OP_OU,          /* esq = op1, dir = op2                         */
    NO_OP_E,           /* esq = op1, dir = op2                         */
    NO_OP_IGUAL,       /* esq = op1, dir = op2                         */
    NO_OP_DIFER,       /* esq = op1, dir = op2                         */
    NO_OP_MENOR,       /* esq = op1, dir = op2                         */
    NO_OP_MAIOR,       /* esq = op1, dir = op2                         */
    NO_OP_MENIG,       /* esq = op1, dir = op2  (<=)                   */
    NO_OP_MAIIG,       /* esq = op1, dir = op2  (>=)                   */
    NO_OP_SOMA,        /* esq = op1, dir = op2                         */
    NO_OP_SUB,         /* esq = op1, dir = op2                         */
    NO_OP_MUL,         /* esq = op1, dir = op2                         */
    NO_OP_DIV,         /* esq = op1, dir = op2                         */

    /* Expressões — operadores unários */
    NO_OP_NEG,         /* esq = operando  (menos unário: -x)           */
    NO_OP_NAO,         /* esq = operando  (negação lógica: !x)         */

    /* Folhas — valores terminais */
    NO_IDENT,          /* valor = nome do identificador                */
    NO_INT_CONST,      /* valor = string do número ("42", "-1")        */
    NO_CAR_CONST,      /* valor = string do caractere ("'a'")          */
    NO_CADEIA          /* valor = string literal ("\"oi\"")            */

} TipoNo;

/*----------------------------------------------------------------
 * Struct do nó — usada para TODOS os tipos de nó
 *
 *   tipo  → qual construção esse nó representa
 *   linha → linha no fonte (para erros semânticos)
 *   valor → lexema (para IDENT, constantes, strings)
 *   esq   → filho esquerdo
 *   dir   → filho direito
 *   extra → terceiro filho (se/senao, bloco com varSection)
 *----------------------------------------------------------------*/
typedef struct No {
    TipoNo      tipo;
    int         linha;
    char       *valor;
    struct No  *esq;
    struct No  *dir;
    struct No  *extra;
} No;

/*----------------------------------------------------------------
 * Funções para criar nós
 *----------------------------------------------------------------*/

/* Cria nó com dois filhos (o caso mais comum) */
No *criar_no(TipoNo tipo, int linha, No *esq, No *dir);

/* Cria nó com três filhos (se/senao, bloco) */
No *criar_no3(TipoNo tipo, int linha, No *esq, No *dir, No *extra);

/* Cria nó folha com valor textual (ident, constantes) */
No *criar_no_folha(TipoNo tipo, int linha, const char *valor);

/* Libera a árvore inteira recursivamente */
void liberar_ast(No *no);

/* Imprime a árvore (útil para depuração) */
void imprimir_ast(No *no, int nivel);

#endif