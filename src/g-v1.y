/* ================================================================
 * g-v1.y — Especificação do Analisador Sintático (Bison)
 *
 * O Bison lê este arquivo e gera g-v1.tab.c e g-v1.tab.h.
 * O .tab.c implementa yyparse(), que consome tokens vindos de
 * yylex() e, a cada regra gramatical reconhecida, executa a ação
 * entre chaves para construir um nó da AST.
 * ================================================================ */

/* ----------------------------------------------------------------
 * %code requires
 *
 * Código inserido no topo do g-v1.tab.h gerado. Precisamos que
 * ast.h seja incluído antes das declarações de tipo do %union,
 * pois o campo 'no' usa o tipo No* definido lá.
 * ---------------------------------------------------------------- */
%code requires { #include "../include/ast.h" }

/* ----------------------------------------------------------------
 * Código C que vai no topo do g-v1.tab.c gerado.
 * ---------------------------------------------------------------- */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ast.h"
#include "../include/semantico.h"
#include "../include/codegen.h"

/* Variáveis e funções fornecidas pelo lexer (lex.yy.c) */
extern int yylineno;    /* número da linha atual                  */
extern char *yytext;    /* texto do último token lido             */
extern int yylex();     /* função que retorna o próximo token     */
extern FILE *yyin;      /* arquivo-fonte que o lexer vai ler      */

/* Raiz da AST — acessível após yyparse() terminar */
No *raiz_ast = NULL;

/* yyerror: chamada pelo Bison quando encontra um erro sintático */
void yyerror(const char *msg) {
    printf("ERRO: %s na linha %d\n", msg, yylineno);
    exit(1);
}
%}

/* ----------------------------------------------------------------
 * %union — tipo semântico dos símbolos
 *
 * Cada token ou não-terminal pode carregar um valor junto com ele.
 * O %union define quais tipos de valor existem:
 *   no  → ponteiro para nó da AST (usado pelos não-terminais)
 *   str → string com o lexema lido (usado por tokens como IDENTIFICADOR)
 *
 * Na pilha interna do parser, cada símbolo empilhado guarda um
 * valor desse union.
 * ---------------------------------------------------------------- */
%union {
    No   *no;
    char *str;
}

/* ----------------------------------------------------------------
 * Tokens sem valor semântico
 *
 * São palavras-chave e operadores que o parser só precisa reconhecer,
 * sem carregar nenhum dado extra.
 * ---------------------------------------------------------------- */
%token PRINCIPAL
%token INT CAR
%token LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO FIMSE
%token ENQUANTO
%token OU E
%token IGUAL DIFERENTE
%token MAIORIGUAL MENORIGUAL

/* ----------------------------------------------------------------
 * Tokens com valor semântico do tipo 'str'
 *
 * O lexer deixa o texto lido em yytext. Para esses tokens, o Bison
 * salva yytext no campo 'str' do union, tornando o lexema acessível
 * nas ações como $1, $2, etc.
 * ---------------------------------------------------------------- */
%token <str> IDENTIFICADOR
%token <str> INTCONST
%token <str> CARCONST
%token <str> CADEIACARACTERES

/* ----------------------------------------------------------------
 * Tipo semântico dos não-terminais
 *
 * Cada regra gramatical produz ($$ =) um nó da AST do tipo No*.
 * O %type informa ao Bison qual campo do union usar para cada
 * não-terminal, evitando casts manuais.
 * ---------------------------------------------------------------- */
%type <no> Programa DeclPrograma
%type <no> Bloco VarSection
%type <no> ListaDeclVar DeclVar Tipo
%type <no> ListaComando Comando
%type <no> Expr OrExpr AndExpr EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr

/* ----------------------------------------------------------------
 * Precedência e associatividade dos operadores
 *
 * Declarados do MENOR para o MAIOR nível de precedência (de cima
 * para baixo). Isso resolve ambiguidades como "a + b * c" sem
 * precisar escrever regras extras na gramática.
 *
 *   %left  → associativo à esquerda:  a-b-c == (a-b)-c
 *   %right → associativo à direita:   !(!x) == !((!x))
 * ---------------------------------------------------------------- */
%left OU                        /* ||  — menor precedência          */
%left E                         /* &&                               */
%left IGUAL DIFERENTE           /* == !=                            */
%left '<' '>' MAIORIGUAL MENORIGUAL  /* < > >= <=                  */
%left '+' '-'                   /* adição e subtração               */
%left '*' '/'                   /* multiplicação e divisão          */
%right '!' UMINUS               /* unários: ! e - unário (maior)    */
                                /* UMINUS é um token fictício usado
                                   só para dar precedência ao '-'
                                   unário via %prec                 */

%%

/* ================================================================
 * GRAMÁTICA COM AÇÕES SEMÂNTICAS
 *
 * Formato de cada regra:
 *
 *   NaoTerminal
 *       : simbolo1 simbolo2 ...   { $$ = criar_no(...); }
 *       | alternativa             { $$ = ...; }
 *       ;
 *
 * $$ → valor produzido por esta regra (nó pai)
 * $1, $2, $N → valor do N-ésimo símbolo do lado direito
 * ================================================================ */

/* ----------------------------------------------------------------
 * Programa — raiz da gramática
 *
 * Toda a árvore parte daqui. Salvamos o nó raiz em raiz_ast para
 * acessá-la no main() depois que yyparse() terminar.
 * ---------------------------------------------------------------- */
Programa
    : DeclPrograma
        { $$ = criar_no(NO_PROGRAMA, yylineno, $1, NULL);
          raiz_ast = $$; }
    ;

/* ----------------------------------------------------------------
 * DeclPrograma — "principal Bloco"
 *
 * O token PRINCIPAL não vira nó na AST; apenas $2 (o Bloco) é
 * propagado para cima. O nó PROGRAMA envolverá esse bloco.
 * ---------------------------------------------------------------- */
DeclPrograma
    : PRINCIPAL Bloco
        { $$ = $2; }
    ;

/* ----------------------------------------------------------------
 * Bloco — corpo de código entre chaves
 *
 * Duas formas:
 *   sem variáveis:  { ListaComando }
 *   com variáveis:  VarSection { ListaComando }
 *
 * O nó NO_BLOCO usa três filhos:
 *   esq   = VarSection (declarações) ou NULL
 *   dir   = ListaComando (comandos)
 *   extra = não usado aqui (NULL)
 * ---------------------------------------------------------------- */
Bloco
    : '{' ListaComando '}'
        { $$ = criar_no3(NO_BLOCO, yylineno, NULL, $2, NULL); }
    | VarSection '{' ListaComando '}'
        { $$ = criar_no3(NO_BLOCO, yylineno, $1, $3, NULL); }
    ;

/* ----------------------------------------------------------------
 * VarSection — seção de declaração de variáveis
 *
 * É um bloco entre chaves contendo apenas declarações.
 * Retorna diretamente a lista de declarações ($2).
 * ---------------------------------------------------------------- */
VarSection
    : '{' ListaDeclVar '}'
        { $$ = $2; }
    ;

/* ----------------------------------------------------------------
 * ListaDeclVar — lista encadeada de declarações
 *
 * Cada declaração tem a forma:   id1, id2, ... : tipo ;
 *
 * A lista é representada como uma cadeia de nós NO_LISTA_DECL:
 *   NO_LISTA_DECL
 *     esq  = NO_DECL_VAR  (esta declaração)
 *     dir  = próximo NO_LISTA_DECL (ou NULL se for o último)
 *
 * Dentro de NO_DECL_VAR:
 *   esq   = NO_IDENT com o primeiro nome ($1)
 *   dir   = NO_TIPO_INT ou NO_TIPO_CAR  ($4)
 *   extra = cadeia de nomes adicionais vinda de DeclVar ($2)
 * ---------------------------------------------------------------- */
ListaDeclVar
    : IDENTIFICADOR DeclVar ':' Tipo ';' ListaDeclVar
        { No *decl = criar_no(NO_DECL_VAR, yylineno,
                              criar_no_folha(NO_IDENT, yylineno, $1), $4);
          /* encadeia os nomes adicionais ($2) no extra da decl */
          decl->extra = $2;
          $$ = criar_no(NO_LISTA_DECL, yylineno, decl, $6); }
    | IDENTIFICADOR DeclVar ':' Tipo ';'
        { No *decl = criar_no(NO_DECL_VAR, yylineno,
                              criar_no_folha(NO_IDENT, yylineno, $1), $4);
          decl->extra = $2;
          $$ = criar_no(NO_LISTA_DECL, yylineno, decl, NULL); }
    ;

/* ----------------------------------------------------------------
 * DeclVar — nomes adicionais após vírgula na mesma declaração
 *
 * Ex.: "x, y, z : int" → DeclVar captura ", y, z"
 *
 * É uma lista encadeada de NO_LISTA_DECL onde cada esq é um
 * NO_IDENT com o nome extra. Quando não há vírgula, retorna NULL.
 * ---------------------------------------------------------------- */
DeclVar
    :                               /* produção vazia: sem nomes extras */
        { $$ = NULL; }
    | ',' IDENTIFICADOR DeclVar     /* mais um nome: cria nó e encadeia */
        { No *id = criar_no_folha(NO_IDENT, yylineno, $2);
          $$ = criar_no(NO_LISTA_DECL, yylineno, id, $3); }
    ;

/* ----------------------------------------------------------------
 * Tipo — int ou car
 *
 * Cada tipo vira um nó folha sem filhos. O tipo do nó já diz
 * qual é (NO_TIPO_INT ou NO_TIPO_CAR).
 * ---------------------------------------------------------------- */
Tipo
    : INT  { $$ = criar_no(NO_TIPO_INT, yylineno, NULL, NULL); }
    | CAR  { $$ = criar_no(NO_TIPO_CAR, yylineno, NULL, NULL); }
    ;

/* ----------------------------------------------------------------
 * ListaComando — lista encadeada de comandos
 *
 * Similar a ListaDeclVar: cada nó NO_LISTA_CMD tem
 *   esq = este Comando
 *   dir = próximo NO_LISTA_CMD (ou NULL)
 * ---------------------------------------------------------------- */
ListaComando
    : Comando
        { $$ = criar_no(NO_LISTA_CMD, yylineno, $1, NULL); }
    | Comando ListaComando
        { $$ = criar_no(NO_LISTA_CMD, yylineno, $1, $2); }
    ;

/* ----------------------------------------------------------------
 * Comando — cada construção imperativa da linguagem
 * ---------------------------------------------------------------- */
Comando
    /* Comando vazio: só ponto e vírgula, não gera nó */
    : ';'
        { $$ = NULL; }

    /* Expressão usada como comando (ex.: atribuição "x = 5;") */
    | Expr ';'
        { $$ = criar_no(NO_CMD_EXPR, yylineno, $1, NULL); }

    /* leia x; — lê um valor do teclado para o identificador $2 */
    | LEIA IDENTIFICADOR ';'
        { $$ = criar_no_folha(NO_CMD_LEIA, yylineno, $2); }

    /* escreva Expr; — imprime o valor de uma expressão */
    | ESCREVA Expr ';'
        { $$ = criar_no(NO_CMD_ESCREVA, yylineno, $2, NULL); }

    /* escreva "string"; — imprime uma cadeia literal */
    | ESCREVA CADEIACARACTERES ';'
        { $$ = criar_no_folha(NO_CMD_ESCREVA_STR, yylineno, $2); }

    /* novalinha; — imprime uma quebra de linha */
    | NOVALINHA ';'
        { $$ = criar_no(NO_CMD_NOVALINHA, yylineno, NULL, NULL); }

    /* se (cond) entao cmd fimse
     * NO_CMD_SE:  esq = condição ($3)
     *             dir = comando do "então" ($6)
     *            extra = NULL (sem senão)                        */
    | SE '(' Expr ')' ENTAO Comando FIMSE
        { $$ = criar_no3(NO_CMD_SE, yylineno, $3, $6, NULL); }

    /* se (cond) entao cmd senao cmd fimse
     * NO_CMD_SE:  esq = condição ($3)
     *             dir = comando do "então" ($6)
     *            extra = comando do "senão" ($8)                 */
    | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE
        { $$ = criar_no3(NO_CMD_SE, yylineno, $3, $6, $8); }

    /* enquanto (cond) cmd
     * NO_CMD_ENQUANTO:  esq = condição ($3)
     *                   dir = corpo do laço ($5)                 */
    | ENQUANTO '(' Expr ')' Comando
        { $$ = criar_no(NO_CMD_ENQUANTO, yylineno, $3, $5); }

    /* Um bloco inteiro pode aparecer onde um comando é esperado */
    | Bloco
        { $$ = $1; }
    ;

/* ================================================================
 * EXPRESSÕES
 *
 * A gramática de expressões é estratificada em níveis de
 * precedência: cada nível só "enxerga" os níveis abaixo dele.
 * Isso garante que, por exemplo, '*' sempre agrupe antes de '+'.
 *
 * Hierarquia (do menor para o maior nível de precedência):
 *   Expr → OrExpr → AndExpr → EqExpr → DesigExpr
 *        → AddExpr → MulExpr → UnExpr → PrimExpr
 * ================================================================ */

/* ----------------------------------------------------------------
 * Expr — nível mais alto; inclui atribuição
 *
 * A atribuição "x = expr" é tratada aqui porque tem a menor
 * precedência entre os operadores. O lado esquerdo é sempre um
 * identificador (lvalue).
 * ---------------------------------------------------------------- */
Expr
    : OrExpr
        { $$ = $1; }
    | IDENTIFICADOR '=' Expr          /* x = expr  (associativo à direita) */
        { $$ = criar_no(NO_ATRIB, yylineno,
                        criar_no_folha(NO_IDENT, yylineno, $1), $3); }
    ;

/* ----------------------------------------------------------------
 * OrExpr — operador || (ou lógico)
 * ---------------------------------------------------------------- */
OrExpr
    : OrExpr OU AndExpr
        { $$ = criar_no(NO_OP_OU, yylineno, $1, $3); }
    | AndExpr
        { $$ = $1; }
    ;

/* ----------------------------------------------------------------
 * AndExpr — operador && (e lógico)
 * ---------------------------------------------------------------- */
AndExpr
    : AndExpr E EqExpr
        { $$ = criar_no(NO_OP_E, yylineno, $1, $3); }
    | EqExpr
        { $$ = $1; }
    ;

/* ----------------------------------------------------------------
 * EqExpr — operadores == e != (igualdade)
 * ---------------------------------------------------------------- */
EqExpr
    : EqExpr IGUAL DesigExpr
        { $$ = criar_no(NO_OP_IGUAL, yylineno, $1, $3); }
    | EqExpr DIFERENTE DesigExpr
        { $$ = criar_no(NO_OP_DIFER, yylineno, $1, $3); }
    | DesigExpr
        { $$ = $1; }
    ;

/* ----------------------------------------------------------------
 * DesigExpr — operadores relacionais < > >= <=
 * ---------------------------------------------------------------- */
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

/* ----------------------------------------------------------------
 * AddExpr — operadores + e - (adição e subtração)
 * ---------------------------------------------------------------- */
AddExpr
    : AddExpr '+' MulExpr
        { $$ = criar_no(NO_OP_SOMA, yylineno, $1, $3); }
    | AddExpr '-' MulExpr
        { $$ = criar_no(NO_OP_SUB, yylineno, $1, $3); }
    | MulExpr
        { $$ = $1; }
    ;

/* ----------------------------------------------------------------
 * MulExpr — operadores * e / (multiplicação e divisão)
 * ---------------------------------------------------------------- */
MulExpr
    : MulExpr '*' UnExpr
        { $$ = criar_no(NO_OP_MUL, yylineno, $1, $3); }
    | MulExpr '/' UnExpr
        { $$ = criar_no(NO_OP_DIV, yylineno, $1, $3); }
    | UnExpr
        { $$ = $1; }
    ;

/* ----------------------------------------------------------------
 * UnExpr — operadores unários: negação aritmética e lógica
 *
 *   %prec UMINUS → diz ao Bison para usar a precedência de UMINUS
 *   (declarada no topo) ao resolver conflitos com o '-' binário.
 *   Sem isso, "-3+2" poderia ser mal interpretado.
 * ---------------------------------------------------------------- */
UnExpr
    : '-' PrimExpr %prec UMINUS       /* menos unário: -x             */
        { $$ = criar_no(NO_OP_NEG, yylineno, $2, NULL); }
    | '!' PrimExpr                    /* negação lógica: !x           */
        { $$ = criar_no(NO_OP_NAO, yylineno, $2, NULL); }
    | PrimExpr
        { $$ = $1; }
    ;

/* ----------------------------------------------------------------
 * PrimExpr — expressões primárias (folhas ou expressão entre parênteses)
 *
 * São os "átomos" da gramática de expressões: um identificador,
 * uma constante inteira, uma constante de caractere, ou uma
 * subexpressão entre parênteses que reinicia a hierarquia.
 * ---------------------------------------------------------------- */
PrimExpr
    : IDENTIFICADOR
        { $$ = criar_no_folha(NO_IDENT, yylineno, $1); }
    | INTCONST
        { $$ = criar_no_folha(NO_INT_CONST, yylineno, $1); }
    | CARCONST
        { $$ = criar_no_folha(NO_CAR_CONST, yylineno, $1); }
    | '(' Expr ')'                    /* parênteses: apenas propaga $2 */
        { $$ = $2; }
    ;

%%

/* ================================================================
 * main — ponto de entrada do compilador
 *
 * 1. Abre o arquivo-fonte passado como argumento
 * 2. Chama yyparse(), que por sua vez chama yylex() repetidamente
 * 3. Se a análise foi bem-sucedida (retorno 0), imprime a AST
 *    e libera a memória
 * ================================================================ */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.g>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");           /* abre o fonte para o lexer ler  */
    if (!yyin) { perror("Erro ao abrir arquivo"); return 1; }

    int resultado = yyparse();            /* dispara a análise léxica+sintática */
    fclose(yyin);

    if (resultado == 0) {
        analisar_semantico(raiz_ast);     /* verifica escopos e tipos           */
        gerar_codigo(raiz_ast);           /* emite assembly MIPS                */
        liberar_ast(raiz_ast);            /* libera toda a memória alocada      */
    }

    return resultado;
}
