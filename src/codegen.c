#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/codegen.h"
#include "../include/symtab.h"
#include "../include/semantico.h"

/* ================================================================
 * Estado global do gerador de código
 * ================================================================ */

/* Controla onde a próxima variável vai ser alocada na pilha */
static int prox_offset = -4;

/* Contador de labels únicos */
static int label_count = 0;

/* Strings coletadas para a seção .data */
#define MAX_STRINGS 256
static char *strings[MAX_STRINGS];
static int   n_strings = 0;

/* ================================================================
 * Funções auxiliares
 * ================================================================ */

static int novo_label(void) { return label_count++; }

/*Usada na fase de coleta de strings*/
/* Registra uma string literal e retorna seu índice */
static int registrar_string(const char *s) {
    for (int i = 0; i < n_strings; i++)
        if (strcmp(strings[i], s) == 0) return i;
    if (n_strings >= MAX_STRINGS) {
        fprintf(stderr, "Erro: muitas strings\n"); exit(1);
    }
    strings[n_strings] = strdup(s);
    return n_strings++;
}

/* Pré-passo: coleta todas as strings da AST */
static void coletar_strings(No *no) {
    if (!no) return;
    if (no->tipo == NO_CMD_ESCREVA_STR)
        registrar_string(no->valor);
    coletar_strings(no->esq);
    coletar_strings(no->dir);
    coletar_strings(no->extra);
}

/* Conta variáveis em uma NO_LISTA_DECL */
static int contar_vars(No *lista) {
    int n = 0;
    for (No *ld = lista; ld != NULL; ld = ld->dir) {
        No *decl = ld->esq;
        n++;                                           /* 1º nome */
        for (No *e = decl->extra; e != NULL; e = e->dir)
            n++;                                       /* nomes extras */
    }
    return n;
}

/* Extrai o valor ASCII de um CARCONST como '\n', 'a', etc. */
static int valor_carconst(const char *s) {
    /* s tem forma: 'x' ou '\c' */
    if (s[1] == '\\') {
        switch (s[2]) {
            case 'n':  return '\n';
            case 't':  return '\t';
            case 'r':  return '\r';
            case '\\': return '\\';
            case '\'': return '\'';
            case '0':  return '\0';
            default:   return s[2];
        }
    }
    return (unsigned char)s[1];
}

/* ================================================================
 * Declarações
 * ================================================================ */
static void gerar_bloco    (No *no, PilhaTabela *p);
static void gerar_lista_cmd(No *no, PilhaTabela *p);
static void gerar_comando  (No *no, PilhaTabela *p);
static void gerar_expr     (No *no, PilhaTabela *p);

/* ================================================================
 * Função recursiva que tem o objetivo de chegar nas folhas,
 * (int/car/ident), armazenam tudo em t0
 * ================================================================ */
static void gerar_expr(No *no, PilhaTabela *p) {
    if (!no) return;

    switch (no->tipo) {
        /*Carrega cte inteira*/
        case NO_INT_CONST:
            printf("\tli\t$t0, %s\n", no->valor);
            break;
        /*Carrega cte caractere*/
        case NO_CAR_CONST:
            printf("\tli\t$t0, %d\n", valor_carconst(no->valor));
            break;
        /*Carrega valor da variavel*/
        case NO_IDENT: {
            EntradaTabela *e = buscar(p, no->valor);
            if (e->tipo == NO_TIPO_INT)
                printf("\tlw\t$t0, %d($fp)\n", e->offset);
            else
                printf("\tlb\t$t0, %d($fp)\n", e->offset);
            break;
        }
        /*Salva valor numa variavel*/
        case NO_ATRIB: {
            /* Precisa ter o valor do nó direito*/
            gerar_expr(no->dir, p);
            EntradaTabela *e = buscar(p, no->esq->valor);
            if (e->tipo == NO_TIPO_INT)
                printf("\tsw\t$t0, %d($fp)\n", e->offset);
            else
                printf("\tsb\t$t0, %d($fp)\n", e->offset);
            break;
        }

        /* Operadores binários: avalia esq (push), avalia dir, pop para $t1 */
        case NO_OP_SOMA:
        case NO_OP_SUB:
        case NO_OP_MUL:
        case NO_OP_DIV:
        case NO_OP_IGUAL:
        case NO_OP_DIFER:
        case NO_OP_MENOR:
        case NO_OP_MAIOR:
        case NO_OP_MENIG:
        case NO_OP_MAIIG:
        case NO_OP_OU:
        case NO_OP_E: {
            /*Avalia valor a esquerda do operador binário*/
            gerar_expr(no->esq, p);
            /* push $t0 na pilha */
            printf("\taddiu\t$sp, $sp, -4\n");
            printf("\tsw\t$t0, ($sp)\n");
            /*Avalia no direito no t0*/
            gerar_expr(no->dir, p);
            /* pop do esq para t1*/
            printf("\tlw\t$t1, ($sp)\n");
            printf("\taddiu\t$sp, $sp, 4\n");
            /* $t1 = esq,  $t0 = dir */
            /*Aqui temos os dois lados da expressao em $t1 e $t0*/
            switch (no->tipo) {
                case NO_OP_SOMA:  printf("\tadd\t$t0, $t1, $t0\n"); break;
                case NO_OP_SUB:   printf("\tsub\t$t0, $t1, $t0\n"); break;
                case NO_OP_MUL:   printf("\tmul\t$t0, $t1, $t0\n"); break;
                case NO_OP_DIV:
                    printf("\tdiv\t$t1, $t0\n");
                    printf("\tmflo\t$t0\n");
                    break;
                case NO_OP_IGUAL: printf("\tseq\t$t0, $t1, $t0\n"); break;
                case NO_OP_DIFER: printf("\tsne\t$t0, $t1, $t0\n"); break;
                case NO_OP_MENOR: printf("\tslt\t$t0, $t1, $t0\n"); break;
                case NO_OP_MAIOR: printf("\tsgt\t$t0, $t1, $t0\n"); break;
                case NO_OP_MENIG: printf("\tsle\t$t0, $t1, $t0\n"); break;
                case NO_OP_MAIIG: printf("\tsge\t$t0, $t1, $t0\n"); break;
                case NO_OP_OU:
                    printf("\tor\t$t0, $t1, $t0\n");
                    printf("\tsneu\t$t0, $t0, $zero\n");
                    break;
                case NO_OP_E:
                    printf("\tsne\t$t1, $t1, $zero\n");
                    printf("\tsne\t$t0, $t0, $zero\n");
                    printf("\tand\t$t0, $t1, $t0\n");
                    break;
                default: break;
            }
            break;
        }

        case NO_OP_NEG:
            gerar_expr(no->esq, p);
            printf("\tneg\t$t0, $t0\n");
            break;

        case NO_OP_NAO:
            gerar_expr(no->esq, p);
            printf("\tseq\t$t0, $t0, $zero\n");
            break;

        default:
            break;
    }
}

/* ================================================================
 * gerar_comando
 * ================================================================ */
static void gerar_comando(No *no, PilhaTabela *p) {
    if (!no) return;

    switch (no->tipo) {
        /*Expressoes simples x = 5*/
        case NO_CMD_EXPR:
            gerar_expr(no->esq, p);
            break;
        /**/
        case NO_CMD_ESCREVA: {
            gerar_expr(no->esq, p);
            /* descobrir tipo da expressão para escolher syscall */
            TipoNo t = analisar_expr_tipo(no->esq, p);
            if (t == NO_TIPO_INT)
                printf("\tmove\t$a0, $t0\n\tli\t$v0, 1\n\tsyscall\n");
            else
                printf("\tmove\t$a0, $t0\n\tli\t$v0, 11\n\tsyscall\n");
            break;
        }
        /* Pega o índice da string e referencia*/
        case NO_CMD_ESCREVA_STR: {
            int idx = registrar_string(no->valor);
            printf("\tla\t$a0, _str%d\n\tli\t$v0, 4\n\tsyscall\n", idx);
            break;
        }
        /* Imprime nova linha */
        case NO_CMD_NOVALINHA:
            printf("\tli\t$a0, 10\n\tli\t$v0, 11\n\tsyscall\n");
            break;

        case NO_CMD_LEIA: {
            EntradaTabela *e = buscar(p, no->valor);
            if (e->tipo == NO_TIPO_INT) {
                printf("\tli\t$v0, 5\n\tsyscall\n");
                printf("\tsw\t$v0, %d($fp)\n", e->offset);
            } else {
                printf("\tli\t$v0, 12\n\tsyscall\n");
                printf("\tsb\t$v0, %d($fp)\n", e->offset);
            }
            break;
        }

        case NO_CMD_SE: {
            int lbl = novo_label();
            gerar_expr(no->esq, p);
            if (no->extra) {
                /* se ... entao ... senao ... fimse */
                printf("\tbeq\t$t0, $zero, _Lelse_%d\n", lbl);
                gerar_comando(no->dir, p);
                printf("\tj\t_Lend_%d\n", lbl);
                printf("_Lelse_%d:\n", lbl);
                gerar_comando(no->extra, p);
            } else {
                /* se ... entao ... fimse */
                printf("\tbeq\t$t0, $zero, _Lend_%d\n", lbl);
                gerar_comando(no->dir, p);
            }
            printf("_Lend_%d:\n", lbl);
            break;
        }

        case NO_CMD_ENQUANTO: {
            int lbl = novo_label();
            printf("_Lwhile_%d:\n", lbl);
            gerar_expr(no->esq, p);
            printf("\tbeq\t$t0, $zero, _Lend_%d\n", lbl);
            gerar_comando(no->dir, p);
            printf("\tj\t_Lwhile_%d\n", lbl);
            printf("_Lend_%d:\n", lbl);
            break;
        }

        case NO_BLOCO:
            gerar_bloco(no, p);
            break;

        default:
            break;
    }
}

/* ================================================================
 * gerar_lista_cmd
 * ================================================================ */
static void gerar_lista_cmd(No *no, PilhaTabela *p) {
    for (No *lc = no; lc != NULL; lc = lc->dir)
        gerar_comando(lc->esq, p);
}

/* ================================================================
 * gerar_bloco - Aloca variaveis na pilha e gera comandos internos
 * ================================================================ */
static void gerar_bloco(No *no, PilhaTabela *p) {
    int tem_decls = (no->esq != NULL);
    int n = 0;

    if (tem_decls) {
        /* Conta quantas variaveis tem e desce stack pointer para reservar espaço */
        n = contar_vars(no->esq);
        printf("\taddiu\t$sp, $sp, -%d\n", n * 4);
        /* Push em novo escopo */
        push_escopo(p);

        /* Atribui offsets a cada variável */
        /* Percorre cada declaração e registra na TS */
        for (No *ld = no->esq; ld != NULL; ld = ld->dir) {
            No *decl = ld->esq;
            TipoNo tipo = decl->dir->tipo;

            inserir(p, decl->esq->valor, tipo);
            buscar(p, decl->esq->valor)->offset = prox_offset;
            prox_offset -= 4;

            for (No *ex = decl->extra; ex != NULL; ex = ex->dir) {
                inserir(p, ex->esq->valor, tipo);
                buscar(p, ex->esq->valor)->offset = prox_offset;
                prox_offset -= 4;
            }
        }
    }

    gerar_lista_cmd(no->dir, p);

    if (tem_decls) {
        pop_escopo(p);
        printf("\taddiu\t$sp, $sp, %d\n", n * 4);
    }
}

/* ================================================================
 * Ponto de entrada - recebe a raiz da AST
 * ================================================================ */
void gerar_codigo(No *raiz) {
    /* Coleta strings para a seção .data */
    coletar_strings(raiz);

    /* Seção .data */
    printf("\t.data\n");
    for (int i = 0; i < n_strings; i++) {
        /* Remove as aspas externas do lexema "\"texto\"" */
        const char *s = strings[i];
        int len = strlen(s);
        printf("_str%d:\t.asciiz %.*s\n", i, len, s);
    }

    /* Seção .text */
    printf("\n\t.text\n");
    printf("\t.globl main\n");
    printf("main:\n");
    printf("\tmove\t$fp, $sp\n\n");

    /* Inicia tabela de simbolos */
    PilhaTabela p;
    init_pilha(&p);

    /* raiz = NO_PROGRAMA; esq = NO_BLOCO principal */
    gerar_bloco(raiz->esq, &p);

    /* Saída do programa */
    printf("\n\tli\t$v0, 10\n");
    printf("\tsyscall\n");
}
