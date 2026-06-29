/* ================================================================
 * g-v2.y — Analisador Sintático do G-V2 (Bison)
 *
 * Estrutura de um programa G-V2:
 *
 *   Programa → DeclVarGlobais DeclFunc DeclPrograma
 *
 * onde:
 *   DeclVarGlobais → "global" '[' ListaDeclVar ']'  | vazio
 *   DeclFunc       → "funcao" '[' func1 func2 ... ']' | vazio
 *   DeclPrograma   → "principal" Bloco
 *
 * Diferenças do G-V1:
 *   1. VarSection usa '[' ']' em vez de '{' '}'
 *   2. Suporte a vetores nas declarações e expressões
 *   3. Funções com parâmetros e retorno
 *   4. Chamadas de função em expressões
 *   5. Comando retorne
 * ================================================================ */

%code requires { #include "../include/ast.h" }

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ast.h"
#include "../include/semantico.h"
#include "../include/codegen.h"

extern int   yylineno;
extern char *yytext;
extern int   yylex();
extern FILE *yyin;

No *raiz_ast = NULL;

void yyerror(const char *msg) {
    printf("ERRO: %s na linha %d\n", msg, yylineno);
    exit(1);
}

/*----------------------------------------------------------------
 * distribuir_tipo_lista — helper chamado em ListaDeclVar.
 *
 * Recebe a lista temporária de ListVar (itens sem tipo ainda) e
 * distribui o tipo a cada item, retornando uma NO_LISTA_DECL
 * encadeada pronta para ser usada na AST.
 *
 * Cada item em listvar é:
 *   NO_IDENT      → variável escalar → cria NO_DECL_VAR
 *   NO_DECL_VET   → variável vetor  → define dir=tipo
 *
 * resto é a NO_LISTA_DECL seguinte (do ListaDeclVar recursivo).
 *----------------------------------------------------------------*/
static No *distribuir_tipo_lista(No *listvar, No *tipo_base, No *resto) {
    /* Constrói a lista em ordem (da esquerda para direita).
     * Usamos uma lista temporária e depois a encadeamos com resto. */
    No *head = NULL;
    No *tail = NULL;

    for (No *lv = listvar; lv != NULL; lv = lv->dir) {
        No *item = lv->esq;
        /* Cria um nó de tipo novo para cada variável (evita compartilhamento) */
        No *tipo_novo = criar_no(tipo_base->tipo, tipo_base->linha, NULL, NULL);
        No *decl;

        if (item->tipo == NO_DECL_VET) {
            /* Vetor: o nó já existe (com valor=nome e tam=tamanho);
             * apenas definimos o tipo (dir) */
            item->dir = tipo_novo;
            decl = item;
        } else {
            /* Escalar: cria NO_DECL_VAR com esq=NO_IDENT, dir=tipo */
            decl = criar_no(NO_DECL_VAR, item->linha, item, tipo_novo);
        }

        No *entrada = criar_no(NO_LISTA_DECL, decl->linha, decl, NULL);
        if (!head) head = entrada;
        else        tail->dir = entrada;
        tail = entrada;
    }

    /* Encadeia com o resto (ListaDeclVar recursivo) */
    if (tail) tail->dir = resto;
    return head ? head : resto;
}
%}

/* ----------------------------------------------------------------
 * %union — tipos semânticos dos símbolos
 * ---------------------------------------------------------------- */
%union {
    No   *no;
    char *str;
}

/* ----------------------------------------------------------------
 * Tokens sem valor semântico
 * ---------------------------------------------------------------- */
%token PRINCIPAL
%token INT CAR
%token LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO FIMSE
%token ENQUANTO
%token OU E
%token IGUAL DIFERENTE
%token MAIORIGUAL MENORIGUAL
/* G-V2: novos tokens */
%token GLOBAL FUNCAO RETORNE

/* Tokens com valor semântico (str = lexema) */
%token <str> IDENTIFICADOR
%token <str> INTCONST
%token <str> CARCONST
%token <str> CADEIACARACTERES

/* ----------------------------------------------------------------
 * Tipos dos não-terminais
 * ---------------------------------------------------------------- */
%type <no> Programa DeclVarGlobais DeclFunc DeclPrograma
%type <no> ListaFuncoes Funcao
%type <no> ListaParametros ListaParametrosTail
%type <no> Bloco VarSection
%type <no> ListaDeclVar ListVar
%type <no> Tipo
%type <no> ListaComando Comando
%type <no> LValueExpr
%type <no> Expr OrExpr AndExpr EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr
%type <no> ListExpr

/* ----------------------------------------------------------------
 * Precedência e associatividade (idêntica ao G-V1)
 * ---------------------------------------------------------------- */
%left OU
%left E
%left IGUAL DIFERENTE
%left '<' '>' MAIORIGUAL MENORIGUAL
%left '+' '-'
%left '*' '/'
%right '!' UMINUS

%%

/* ================================================================
 * Programa — raiz da gramática G-V2
 *
 * Um programa tem (opcionalmente) variáveis globais,
 * (opcionalmente) funções, e obrigatoriamente um bloco principal.
 * ================================================================ */
Programa
    : DeclVarGlobais DeclFunc DeclPrograma
        { raiz_ast = criar_no3(NO_PROGRAMA_V2, yylineno, $1, $2, $3); }
    ;

/* ================================================================
 * DeclVarGlobais — seção "global [ ListaDeclVar ]"
 *
 * NO_DECL_VAR_GLOBAIS
 *   esq = NO_LISTA_DECL com as variáveis globais
 * ================================================================ */
DeclVarGlobais
    : GLOBAL VarSection
        { $$ = criar_no(NO_DECL_VAR_GLOBAIS, yylineno, $2, NULL); }
    | /* vazio */
        { $$ = NULL; }
    ;

/* ================================================================
 * DeclFunc — seção "funcao [ func1 func2 ... ]"
 *
 * Retorna uma lista encadeada de NO_FUNC via dir.
 * A lista é representada como NO_DECL_FUNC:
 *   esq = primeiro NO_FUNC
 *   dir = próximo NO_DECL_FUNC (ou NULL)
 * ================================================================ */
DeclFunc
    : FUNCAO '[' Funcao ListaFuncoes ']'
        { /* $3 = primeira função, $4 = restante */
          /* Encadeia: cria cabeça da lista */
          No *lista = criar_no(NO_DECL_FUNC, yylineno, $3, NULL);
          /* Encadeia o restante */
          No *cur = lista;
          for (No *lf = $4; lf != NULL; lf = lf->dir) {
              No *nova = criar_no(NO_DECL_FUNC, yylineno, lf->esq, NULL);
              cur->dir = nova;
              cur = nova;
          }
          $$ = lista; }
    | /* vazio */
        { $$ = NULL; }
    ;

/* Uma função individual: "nome ( params ) : tipo Bloco"
 *
 * NO_FUNC:
 *   valor = nome da função
 *   esq   = NO_LISTA_PARAM (parâmetros) ou NULL
 *   dir   = NO_TIPO_INT ou NO_TIPO_CAR (tipo de retorno)
 *   extra = NO_BLOCO (corpo)
 */
Funcao
    : IDENTIFICADOR '(' ListaParametros ')' ':' Tipo Bloco
        { No *fn = criar_no3(NO_FUNC, yylineno, $3, $6, $7);
          fn->valor = $1;
          $$ = fn; }
    ;

/* Lista de funções adicionais (zero ou mais) */
ListaFuncoes
    : Funcao ListaFuncoes
        { $$ = criar_no(NO_DECL_FUNC, yylineno, $1, $2); }
    | /* vazio */
        { $$ = NULL; }
    ;

/* ================================================================
 * Parâmetros — lista de parâmetros de uma função
 *
 * NO_LISTA_PARAM:
 *   esq = NO_PARAM ou NO_PARAM_VET
 *   dir = próximo NO_LISTA_PARAM (ou NULL)
 * ================================================================ */
ListaParametros
    : ListaParametrosTail
        { $$ = $1; }
    | /* vazio */
        { $$ = NULL; }
    ;

ListaParametrosTail
    /* id : tipo */
    : IDENTIFICADOR ':' Tipo
        { No *param = criar_no(NO_PARAM, yylineno, NULL, $3);
          param->valor = $1;
          $$ = criar_no(NO_LISTA_PARAM, yylineno, param, NULL); }
    /* id[] : tipo  (parâmetro vetor) */
    | IDENTIFICADOR '[' ']' ':' Tipo
        { No *param = criar_no(NO_PARAM_VET, yylineno, NULL, $5);
          param->valor = $1;
          $$ = criar_no(NO_LISTA_PARAM, yylineno, param, NULL); }
    /* id : tipo , mais */
    | IDENTIFICADOR ':' Tipo ',' ListaParametrosTail
        { No *param = criar_no(NO_PARAM, yylineno, NULL, $3);
          param->valor = $1;
          $$ = criar_no(NO_LISTA_PARAM, yylineno, param, $5); }
    /* id[] : tipo , mais */
    | IDENTIFICADOR '[' ']' ':' Tipo ',' ListaParametrosTail
        { No *param = criar_no(NO_PARAM_VET, yylineno, NULL, $5);
          param->valor = $1;
          $$ = criar_no(NO_LISTA_PARAM, yylineno, param, $7); }
    ;

/* ================================================================
 * DeclPrograma — "principal Bloco"
 * ================================================================ */
DeclPrograma
    : PRINCIPAL Bloco
        { $$ = $2; }
    ;

/* ================================================================
 * Bloco — corpo de código
 *
 * Em G-V2, as declarações de variáveis usam '[' ']' (VarSection).
 * Os comandos ainda ficam entre '{' '}'.
 * ================================================================ */
Bloco
    : '{' ListaComando '}'
        { $$ = criar_no3(NO_BLOCO, yylineno, NULL, $2, NULL); }
    | VarSection '{' ListaComando '}'
        { $$ = criar_no3(NO_BLOCO, yylineno, $1, $3, NULL); }
    ;

/* ================================================================
 * VarSection — seção de declaração: '[' ListaDeclVar ']'
 * ================================================================ */
VarSection
    : '[' ListaDeclVar ']'
        { $$ = $2; }
    ;

/* ================================================================
 * ListaDeclVar — lista de declarações de variáveis
 *
 * Formato: "ListVar : Tipo ; ListaDeclVar"
 *
 * Usa a função auxiliar distribuir_tipo_lista para atribuir o tipo
 * a cada elemento da ListVar.
 * ================================================================ */
ListaDeclVar
    : ListVar ':' Tipo ';' ListaDeclVar
        { $$ = distribuir_tipo_lista($1, $3, $5); }
    | ListVar ':' Tipo ';'
        { $$ = distribuir_tipo_lista($1, $3, NULL); }
    ;

/* ================================================================
 * ListVar — lista de identificadores (escalares e vetores) ANTES
 * de saber o tipo. Retorna NO_LISTA_DECL temporária onde cada esq é:
 *   NO_IDENT    → variável simples
 *   NO_DECL_VET → vetor (valor=nome, tam=tamanho, dir=NULL ainda)
 * ================================================================ */
ListVar
    /* id , resto */
    : IDENTIFICADOR ',' ListVar
        { No *id = criar_no_folha(NO_IDENT, yylineno, $1);
          $$ = criar_no(NO_LISTA_DECL, yylineno, id, $3); }
    /* id[N] , resto */
    | IDENTIFICADOR '[' INTCONST ']' ',' ListVar
        { No *vet = criar_no_folha_tam(NO_DECL_VET, yylineno, $1, atoi($3));
          $$ = criar_no(NO_LISTA_DECL, yylineno, vet, $6); }
    /* id  (último) */
    | IDENTIFICADOR
        { No *id = criar_no_folha(NO_IDENT, yylineno, $1);
          $$ = criar_no(NO_LISTA_DECL, yylineno, id, NULL); }
    /* id[N]  (último) */
    | IDENTIFICADOR '[' INTCONST ']'
        { No *vet = criar_no_folha_tam(NO_DECL_VET, yylineno, $1, atoi($3));
          $$ = criar_no(NO_LISTA_DECL, yylineno, vet, NULL); }
    ;

/* ================================================================
 * Tipo
 * ================================================================ */
Tipo
    : INT  { $$ = criar_no(NO_TIPO_INT, yylineno, NULL, NULL); }
    | CAR  { $$ = criar_no(NO_TIPO_CAR, yylineno, NULL, NULL); }
    ;

/* ================================================================
 * ListaComando e Comando
 * ================================================================ */
ListaComando
    : Comando
        { $$ = criar_no(NO_LISTA_CMD, yylineno, $1, NULL); }
    | Comando ListaComando
        { $$ = criar_no(NO_LISTA_CMD, yylineno, $1, $2); }
    ;

Comando
    : ';'
        { $$ = NULL; }

    | Expr ';'
        { $$ = criar_no(NO_CMD_EXPR, yylineno, $1, NULL); }

    /* retorne Expr; — G-V2 */
    | RETORNE Expr ';'
        { $$ = criar_no(NO_CMD_RETORNE, yylineno, $2, NULL); }

    /* leia LValueExpr; — G-V2 (cobre escalar e vetor) */
    | LEIA LValueExpr ';'
        { if ($2->tipo == NO_IDENT_VET)
              $$ = criar_no(NO_CMD_LEIA_VET, yylineno, $2, NULL);
          else
              $$ = criar_no_folha(NO_CMD_LEIA, yylineno, $2->valor); }

    | ESCREVA Expr ';'
        { $$ = criar_no(NO_CMD_ESCREVA, yylineno, $2, NULL); }

    | ESCREVA CADEIACARACTERES ';'
        { $$ = criar_no_folha(NO_CMD_ESCREVA_STR, yylineno, $2); }

    | NOVALINHA ';'
        { $$ = criar_no(NO_CMD_NOVALINHA, yylineno, NULL, NULL); }

    | SE '(' Expr ')' ENTAO Comando FIMSE
        { $$ = criar_no3(NO_CMD_SE, yylineno, $3, $6, NULL); }

    | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE
        { $$ = criar_no3(NO_CMD_SE, yylineno, $3, $6, $8); }

    | ENQUANTO '(' Expr ')' Comando
        { $$ = criar_no(NO_CMD_ENQUANTO, yylineno, $3, $5); }

    | Bloco
        { $$ = $1; }
    ;

/* ================================================================
 * LValueExpr — lado esquerdo de uma atribuição ou leia
 *
 * NO_IDENT_VET: valor=nome, esq=índice
 * NO_IDENT:     valor=nome
 * ================================================================ */
LValueExpr
    : IDENTIFICADOR '[' Expr ']'
        { $$ = criar_no(NO_IDENT_VET, yylineno, $3, NULL);
          $$->valor = $1; }
    | IDENTIFICADOR
        { $$ = criar_no_folha(NO_IDENT, yylineno, $1); }
    ;

/* ================================================================
 * Expressões — hierarquia de precedência
 *
 * Expr → LValueExpr '=' Expr  (atribuição, direita-associativa)
 *      | OrExpr
 * ================================================================ */
Expr
    : LValueExpr '=' Expr
        { $$ = criar_no(NO_ATRIB, yylineno, $1, $3); }
    | OrExpr
        { $$ = $1; }
    ;

OrExpr
    : OrExpr OU AndExpr
        { $$ = criar_no(NO_OP_OU, yylineno, $1, $3); }
    | AndExpr
        { $$ = $1; }
    ;

AndExpr
    : AndExpr E EqExpr
        { $$ = criar_no(NO_OP_E, yylineno, $1, $3); }
    | EqExpr
        { $$ = $1; }
    ;

EqExpr
    : EqExpr IGUAL DesigExpr
        { $$ = criar_no(NO_OP_IGUAL, yylineno, $1, $3); }
    | EqExpr DIFERENTE DesigExpr
        { $$ = criar_no(NO_OP_DIFER, yylineno, $1, $3); }
    | DesigExpr
        { $$ = $1; }
    ;

DesigExpr
    : DesigExpr '<' AddExpr
        { $$ = criar_no(NO_OP_MENOR, yylineno, $1, $3); }
    | DesigExpr '>' AddExpr
        { $$ = criar_no(NO_OP_MAIOR, yylineno, $1, $3); }
    | DesigExpr MAIORIGUAL AddExpr
        { $$ = criar_no(NO_OP_MAIIG, yylineno, $1, $3); }
    | DesigExpr MENORIGUAL AddExpr
        { $$ = criar_no(NO_OP_MENIG, yylineno, $1, $3); }
    | AddExpr
        { $$ = $1; }
    ;

AddExpr
    : AddExpr '+' MulExpr
        { $$ = criar_no(NO_OP_SOMA, yylineno, $1, $3); }
    | AddExpr '-' MulExpr
        { $$ = criar_no(NO_OP_SUB, yylineno, $1, $3); }
    | MulExpr
        { $$ = $1; }
    ;

MulExpr
    : MulExpr '*' UnExpr
        { $$ = criar_no(NO_OP_MUL, yylineno, $1, $3); }
    | MulExpr '/' UnExpr
        { $$ = criar_no(NO_OP_DIV, yylineno, $1, $3); }
    | UnExpr
        { $$ = $1; }
    ;

UnExpr
    : '-' PrimExpr %prec UMINUS
        { $$ = criar_no(NO_OP_NEG, yylineno, $2, NULL); }
    | '!' PrimExpr
        { $$ = criar_no(NO_OP_NAO, yylineno, $2, NULL); }
    | PrimExpr
        { $$ = $1; }
    ;

/* ================================================================
 * PrimExpr — expressões primárias (folhas ou parênteses)
 *
 * G-V2 adiciona:
 *   - Chamada de função: nome(args) ou nome()
 *   - Acesso a vetor:    nome[índice]
 * ================================================================ */
PrimExpr
    /* Chamada de função com argumentos */
    : IDENTIFICADOR '(' ListExpr ')'
        { No *call = criar_no(NO_CHAMADA_FUNC, yylineno, $3, NULL);
          call->valor = $1;
          $$ = call; }
    /* Chamada de função sem argumentos */
    | IDENTIFICADOR '(' ')'
        { No *call = criar_no(NO_CHAMADA_FUNC, yylineno, NULL, NULL);
          call->valor = $1;
          $$ = call; }
    /* Acesso a vetor: a[i] */
    | IDENTIFICADOR '[' Expr ']'
        { No *vet = criar_no(NO_IDENT_VET, yylineno, $3, NULL);
          vet->valor = $1;
          $$ = vet; }
    /* Identificador simples */
    | IDENTIFICADOR
        { $$ = criar_no_folha(NO_IDENT, yylineno, $1); }
    | INTCONST
        { $$ = criar_no_folha(NO_INT_CONST, yylineno, $1); }
    | CARCONST
        { $$ = criar_no_folha(NO_CAR_CONST, yylineno, $1); }
    | '(' Expr ')'
        { $$ = $2; }
    ;

/* ================================================================
 * ListExpr — lista de argumentos em uma chamada de função
 *
 * NO_LISTA_ARGS:
 *   esq = Expr (argumento)
 *   dir = próximo NO_LISTA_ARGS (ou NULL)
 * ================================================================ */
ListExpr
    : Expr
        { $$ = criar_no(NO_LISTA_ARGS, yylineno, $1, NULL); }
    | ListExpr ',' Expr
        { /* Encadeia novo argumento ao final da lista */
          No *novo = criar_no(NO_LISTA_ARGS, yylineno, $3, NULL);
          /* Percorre até o fim para encadear */
          No *cur = $1;
          while (cur->dir) cur = cur->dir;
          cur->dir = novo;
          $$ = $1; }
    ;

%%

/* ================================================================
 * main — ponto de entrada do compilador G-V2
 * ================================================================ */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.g>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) { perror("Erro ao abrir arquivo"); return 1; }

    int resultado = yyparse();
    fclose(yyin);

    if (resultado == 0) {
        analisar_semantico(raiz_ast);
        gerar_codigo(raiz_ast);
        liberar_ast(raiz_ast);
    }

    return resultado;
}
