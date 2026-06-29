#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/semantico.h"
#include "../include/symtab.h"

/*================================================================
 * semantico.c — Análise Semântica para G-V1 e G-V2
 *
 * Percorre a AST verificando:
 *   - Toda variável é declarada antes do uso
 *   - Os tipos são compatíveis nas operações e atribuições
 *   - Chamadas de função têm número e tipos de argumentos corretos
 *   - O comando `retorne` só aparece dentro de funções e com
 *     o tipo certo
 *   - Vetores só são indexados com expressão do tipo int
 *================================================================*/

/* --- Contexto global do analisador --- */

/* Tipo de retorno da função que está sendo analisada no momento.
 * -1 significa "não estamos dentro de uma função". */
static TipoNo funcao_ret_tipo = (TipoNo)(-1);

/* --- Protótipos internos --- */
static void   analisar_bloco      (No *no, PilhaTabela *p);
static void   analisar_lista_cmd  (No *no, PilhaTabela *p);
static void   analisar_comando    (No *no, PilhaTabela *p);
       TipoNo analisar_expr_tipo  (No *no, PilhaTabela *p);

/* --- Utilitários --- */

static void erro_sem(const char *msg, int linha) {
    printf("ERRO: %s linha %d\n", msg, linha);
    exit(1);
}

/*----------------------------------------------------------------
 * inserir_decls_sem — insere variáveis de uma NO_LISTA_DECL
 * na tabela de símbolos do escopo atual.
 *
 * Trata dois formatos:
 *   G-V1: NO_DECL_VAR com esq=NO_IDENT(nome), dir=tipo, extra=mais_nomes
 *   G-V2: NO_DECL_VAR igual + NO_DECL_VET com valor=nome, dir=tipo, tam=tamanho
 *----------------------------------------------------------------*/
/* Verifica se nome conflita com parâmetro de função em escopo envolvente.
 * Chamado antes de inserir variável local quando dentro de uma função. */
static void checar_conflito_param(PilhaTabela *p, const char *nome, int linha) {
    if ((int)funcao_ret_tipo == -1) return;  /* fora de função, sem problema */
    EntradaTabela *e = buscar(p, nome);
    if (e && (e->kind == KIND_PARAM || e->kind == KIND_PARAM_VET))
        erro_sem("variavel tem mesmo nome que parametro da funcao", linha);
}

static void inserir_decls_sem(No *lista, PilhaTabela *p) {
    for (No *ld = lista; ld != NULL; ld = ld->dir) {
        No *decl = ld->esq;
        TipoNo tipo = decl->dir->tipo;  /* NO_TIPO_INT ou NO_TIPO_CAR */

        if (decl->tipo == NO_DECL_VET) {
            /* Declaração de vetor: "v[N] : tipo" */
            checar_conflito_param(p, decl->valor, decl->linha);
            inserir_vetor(p, decl->valor, tipo, decl->tam);
        } else {
            /* Declaração escalar: "a, b, c : tipo" */
            checar_conflito_param(p, decl->esq->valor, decl->linha);
            inserir(p, decl->esq->valor, tipo);
            /* Nomes adicionais após vírgula (G-V1 style) */
            for (No *ex = decl->extra; ex != NULL; ex = ex->dir) {
                checar_conflito_param(p, ex->esq->valor, ex->esq->linha);
                inserir(p, ex->esq->valor, tipo);
            }
        }
    }
}

/*----------------------------------------------------------------
 * analisar_bloco — analisa um NO_BLOCO.
 *
 * Se o bloco tiver declarações (esq != NULL), cria novo escopo,
 * insere as variáveis e analisa os comandos. Ao final, remove
 * o escopo. Se não tiver declarações, analisa direto.
 *----------------------------------------------------------------*/
static void analisar_bloco(No *no, PilhaTabela *p) {
    int tem_decls = (no->esq != NULL);
    if (tem_decls) {
        push_escopo(p);
        inserir_decls_sem(no->esq, p);
    }
    analisar_lista_cmd(no->dir, p);
    if (tem_decls)
        pop_escopo(p);
}

/* Percorre a lista ligada de comandos */
static void analisar_lista_cmd(No *no, PilhaTabela *p) {
    for (No *lc = no; lc != NULL; lc = lc->dir)
        analisar_comando(lc->esq, p);
}

/*----------------------------------------------------------------
 * analisar_comando — valida semanticamente um comando
 *----------------------------------------------------------------*/
static void analisar_comando(No *no, PilhaTabela *p) {
    if (!no) return;

    switch (no->tipo) {

        case NO_CMD_EXPR:
            analisar_expr_tipo(no->esq, p);
            break;

        /* leia id; — G-V1 style */
        case NO_CMD_LEIA:
            if (!buscar(p, no->valor))
                erro_sem("variavel nao declarada", no->linha);
            break;

        /* leia a[i]; — G-V2 style */
        case NO_CMD_LEIA_VET: {
            /* esq é NO_IDENT_VET, que será verificado em analisar_expr_tipo */
            TipoNo t = analisar_expr_tipo(no->esq, p);
            (void)t; /* resultado não é usado, só valida */
            break;
        }

        case NO_CMD_ESCREVA:
            analisar_expr_tipo(no->esq, p);
            break;

        case NO_CMD_ESCREVA_STR:
        case NO_CMD_NOVALINHA:
            break;

        case NO_CMD_SE:
            analisar_expr_tipo(no->esq, p);   /* condição */
            analisar_comando(no->dir, p);      /* ramo entao */
            if (no->extra)
                analisar_comando(no->extra, p); /* ramo senao */
            break;

        case NO_CMD_ENQUANTO:
            analisar_expr_tipo(no->esq, p);
            analisar_comando(no->dir, p);
            break;

        case NO_BLOCO:
            analisar_bloco(no, p);
            break;

        /* retorne Expr; — G-V2 */
        case NO_CMD_RETORNE: {
            if ((int)funcao_ret_tipo == -1)
                erro_sem("'retorne' fora de funcao", no->linha);
            TipoNo t = analisar_expr_tipo(no->esq, p);
            if (t != funcao_ret_tipo)
                erro_sem("tipo do retorno incompativel com a funcao", no->linha);
            break;
        }

        default:
            break;
    }
}

/*----------------------------------------------------------------
 * analisar_expr_tipo — determina o tipo de uma expressão.
 *
 * Retorna NO_TIPO_INT ou NO_TIPO_CAR.
 * Emite erro e encerra se encontrar violação de tipo.
 *----------------------------------------------------------------*/
TipoNo analisar_expr_tipo(No *no, PilhaTabela *p) {
    if (!no) return NO_TIPO_INT;

    TipoNo t1, t2;

    switch (no->tipo) {

        case NO_INT_CONST:
            return NO_TIPO_INT;

        case NO_CAR_CONST:
            return NO_TIPO_CAR;

        /* Identificador simples: busca tipo na tabela */
        case NO_IDENT: {
            EntradaTabela *e = buscar(p, no->valor);
            if (!e)
                erro_sem("variavel nao declarada", no->linha);
            /* Vetores não podem ser usados sem índice como rvalue escalar
             * (exceto em atribuição vetor = vetor, tratada em NO_ATRIB) */
            return e->tipo;
        }

        /* Acesso indexado a[i] — G-V2 */
        case NO_IDENT_VET: {
            EntradaTabela *e = buscar(p, no->valor);
            if (!e)
                erro_sem("variavel nao declarada", no->linha);
            if (e->kind != KIND_VET && e->kind != KIND_PARAM_VET)
                erro_sem("variavel nao e vetor", no->linha);
            /* O índice deve ser int */
            TipoNo tidx = analisar_expr_tipo(no->esq, p);
            if (tidx != NO_TIPO_INT)
                erro_sem("indice de vetor deve ser int", no->esq->linha);
            return e->tipo; /* tipo do elemento */
        }

        /* Atribuição: id = Expr ou id[i] = Expr */
        case NO_ATRIB: {
            TipoNo tipo_dest;
            if (no->esq->tipo == NO_IDENT_VET) {
                /* Atribuição a elemento de vetor: a[i] = expr */
                tipo_dest = analisar_expr_tipo(no->esq, p);
            } else {
                /* Atribuição a variável escalar: a = expr */
                EntradaTabela *e = buscar(p, no->esq->valor);
                if (!e)
                    erro_sem("variavel nao declarada", no->esq->linha);
                tipo_dest = e->tipo;
            }
            TipoNo tipo_val = analisar_expr_tipo(no->dir, p);
            if (tipo_dest != tipo_val)
                erro_sem("tipos incompativeis na atribuicao", no->linha);
            return tipo_dest;
        }

        /* Chamada de função: f(arg1, arg2, ...) — G-V2 */
        case NO_CHAMADA_FUNC: {
            EntradaTabela *ef = buscar(p, no->valor);
            if (!ef)
                erro_sem("funcao nao declarada", no->linha);
            if (ef->kind != KIND_FUNC)
                erro_sem("identificador nao e funcao", no->linha);

            /* Conta argumentos passados */
            int n_args = 0;
            for (No *la = no->esq; la != NULL; la = la->dir) n_args++;

            if (n_args != ef->num_params)
                erro_sem("numero de argumentos incorreto", no->linha);

            /* Verifica tipo de cada argumento vs parâmetro */
            InfoParam *ip = ef->params;
            for (No *la = no->esq; la != NULL; la = la->dir, ip = ip->prox) {
                No *arg = la->esq;
                if (ip->eh_vetor) {
                    /* Parâmetro vetor: argumento deve ser identificador de vetor */
                    if (arg->tipo != NO_IDENT && arg->tipo != NO_IDENT_VET) {
                        /* Pode ser NO_IDENT que aponta para um vetor */
                    }
                    EntradaTabela *ea = NULL;
                    if (arg->tipo == NO_IDENT || arg->tipo == NO_IDENT_VET)
                        ea = buscar(p, arg->valor);
                    if (!ea || (ea->kind != KIND_VET && ea->kind != KIND_PARAM_VET))
                        erro_sem("argumento deve ser vetor", arg->linha);
                    if (ea->tipo != ip->tipo)
                        erro_sem("tipo do argumento vetor incompativel", arg->linha);
                } else {
                    /* Parâmetro escalar */
                    TipoNo ta = analisar_expr_tipo(arg, p);
                    if (ta != ip->tipo)
                        erro_sem("tipo do argumento incompativel", arg->linha);
                }
            }
            return ef->tipo; /* tipo de retorno */
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

        case NO_OP_NAO:
            t1 = analisar_expr_tipo(no->esq, p);
            if (t1 != NO_TIPO_INT)
                erro_sem("operador logico requer int", no->linha);
            return NO_TIPO_INT;

        default:
            return NO_TIPO_INT;
    }
}

/*----------------------------------------------------------------
 * analisar_funcoes — processa a seção de funções G-V2.
 *
 * Para cada função:
 *   1. Registra o nome da função no escopo externo (visível a todos)
 *   2. Cria um escopo novo para parâmetros + corpo
 *   3. Analisa o corpo, com funcao_ret_tipo setado
 *   4. Fecha o escopo da função
 *
 * A função NO_DECL_FUNC é uma lista ligada de NO_FUNC via dir.
 *----------------------------------------------------------------*/
static void analisar_funcoes(No *lista_func, PilhaTabela *p) {
    for (No *lf = lista_func; lf != NULL; lf = lf->dir) {
        No *fn = lf->esq;   /* NO_FUNC */

        /* --- Coleta informações dos parâmetros --- */
        int num_params = 0;
        InfoParam *params_head = NULL;
        InfoParam *params_tail = NULL;

        for (No *lp = fn->esq; lp != NULL; lp = lp->dir) {
            No *param = lp->esq;  /* NO_PARAM ou NO_PARAM_VET */
            InfoParam *ip = malloc(sizeof(InfoParam));
            if (!ip) { fprintf(stderr, "Erro: sem memoria\n"); exit(1); }
            ip->nome     = strdup(param->valor);
            ip->tipo     = param->dir->tipo;
            ip->eh_vetor = (param->tipo == NO_PARAM_VET);
            ip->prox     = NULL;
            if (!params_head) params_head = ip;
            else              params_tail->prox = ip;
            params_tail = ip;
            num_params++;
        }

        TipoNo tipo_ret = fn->dir->tipo;  /* NO_TIPO_INT ou NO_TIPO_CAR */

        /* Registra a função no escopo atual (fora da função) */
        inserir_func(p, fn->valor, tipo_ret, num_params, params_head);

        /* --- Analisa o corpo da função --- */
        push_escopo(p);

        /* Insere parâmetros no escopo da função */
        InfoParam *ip = params_head;
        for (No *lp = fn->esq; lp != NULL; lp = lp->dir, ip = ip->prox)
            inserir_parametro(p, ip->nome, ip->tipo, ip->eh_vetor);

        /* Seta tipo de retorno esperado */
        TipoNo ret_anterior = funcao_ret_tipo;
        funcao_ret_tipo = tipo_ret;

        /* Analisa o bloco da função */
        analisar_bloco(fn->extra, p);

        /* Restaura contexto */
        funcao_ret_tipo = ret_anterior;
        pop_escopo(p);
    }
}

/*----------------------------------------------------------------
 * analisar_semantico — ponto de entrada público
 *
 * Detecta se é programa G-V1 (NO_PROGRAMA) ou G-V2 (NO_PROGRAMA_V2)
 * e direciona para a análise correta.
 *----------------------------------------------------------------*/
void analisar_semantico(No *raiz) {
    PilhaTabela p;
    init_pilha(&p);

    if (raiz->tipo == NO_PROGRAMA) {
        /* ── G-V1: raiz->esq é o NO_BLOCO do principal ── */
        analisar_bloco(raiz->esq, &p);
        return;
    }

    /* ── G-V2 ── */

    /* Escopo global: variáveis globais e nomes de funções ficam aqui */
    push_escopo(&p);

    /* 1. Declara variáveis globais (seção "global [ ... ]") */
    if (raiz->esq)
        inserir_decls_sem(raiz->esq->esq, &p);

    /* 2. Declara e analisa funções (seção "funcao [ ... ]") */
    if (raiz->dir)
        analisar_funcoes(raiz->dir, &p);

    /* 3. Analisa o bloco principal */
    analisar_bloco(raiz->extra, &p);

    pop_escopo(&p);
}
