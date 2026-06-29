#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/codegen.h"
#include "../include/symtab.h"
#include "../include/semantico.h"

/*================================================================
 * codegen.c — Geração de Código MIPS para G-V1 e G-V2
 *
 * Alvo: SPIM/MARS (simuladores MIPS32).
 *
 * Convenção de registradores:
 *   $t0  — resultado da subexpressão atual
 *   $t1  — operando esquerdo (em operações binárias)
 *   $fp  — frame pointer (base do frame atual)
 *   $sp  — stack pointer
 *   $ra  — endereço de retorno
 *   $v0  — valor de retorno de função + código de syscall
 *   $a0  — argumento de syscall
 *
 * Layout do frame de função (G-V2):
 *
 *   $fp + (N-1)*4  ←  argN    (arg empilhado primeiro pelo chamador)
 *   ...
 *   $fp + 4        ←  arg2
 *   $fp + 0        ←  arg1    (arg empilhado por último; topo da pilha)
 *   $fp - 4        ←  $ra salvo
 *   $fp - 8        ←  $fp antigo
 *   $fp - 12       ←  var_local_1
 *   $fp - 16       ←  var_local_2
 *   ...
 *
 * O chamador empilha os args, chama "jal", e após o retorno
 * desempilha (addiu $sp, $sp, N*4). O valor de retorno fica em $v0.
 *================================================================*/

/* ================================================================
 * Estado global do gerador
 * ================================================================ */

static int prox_offset = -4;   /* próximo offset para variável local */
static int label_count = 0;    /* contador de labels únicos          */

/* Strings coletadas para a seção .data */
#define MAX_STRINGS 256
static char *strings[MAX_STRINGS];
static int   n_strings = 0;

/* Variáveis globais para a seção .data (G-V2) */
#define MAX_GLOBAIS 256
typedef struct { char *nome; TipoNo tipo; int tam; } InfoGlobal;
static InfoGlobal globais[MAX_GLOBAIS];
static int n_globais = 0;

/* ================================================================
 * Funções auxiliares
 * ================================================================ */

static int novo_label(void) { return label_count++; }

static int registrar_string(const char *s) {
    for (int i = 0; i < n_strings; i++)
        if (strcmp(strings[i], s) == 0) return i;
    if (n_strings >= MAX_STRINGS) { fprintf(stderr, "Erro: muitas strings\n"); exit(1); }
    strings[n_strings] = strdup(s);
    return n_strings++;
}

/* Pré-passo: coleta todas as strings da AST para a seção .data */
static void coletar_strings(No *no) {
    if (!no) return;
    if (no->tipo == NO_CMD_ESCREVA_STR)
        registrar_string(no->valor);
    coletar_strings(no->esq);
    coletar_strings(no->dir);
    coletar_strings(no->extra);
}

/* Extrai o valor ASCII de um CARCONST como '\n', 'a', etc. */
static int valor_carconst(const char *s) {
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
 * Declarações antecipadas
 * ================================================================ */
static void gerar_bloco    (No *no, PilhaTabela *p);
static void gerar_lista_cmd(No *no, PilhaTabela *p);
static void gerar_comando  (No *no, PilhaTabela *p);
static void gerar_expr     (No *no, PilhaTabela *p);
static void gerar_lvalue_endereco(No *no, PilhaTabela *p);

/* ================================================================
 * Calcula bytes necessários para uma NO_LISTA_DECL.
 * Simplificação didática: todos os elementos usam 4 bytes cada
 * (tanto int quanto car), evitando problemas de alinhamento MIPS.
 * ================================================================ */
static int calcular_espaco_decls(No *lista) {
    int bytes = 0;
    for (No *ld = lista; ld != NULL; ld = ld->dir) {
        No *decl = ld->esq;
        if (decl->tipo == NO_DECL_VET) {
            bytes += decl->tam * 4;  /* tam elementos × 4 bytes */
        } else {
            int n = 1;
            for (No *ex = decl->extra; ex != NULL; ex = ex->dir) n++;
            bytes += n * 4;
        }
    }
    return bytes;
}

/* ================================================================
 * gerar_lvalue_endereco — coloca em $t1 o ENDEREÇO do lvalue.
 * Usado em atribuições e leia para vetores.
 *
 * Para NO_IDENT_VET (a[i]):
 *   - Avalia índice em $t0
 *   - Calcula base + i * elem_size em $t1
 * Para NO_IDENT escalar: simplesmente $fp + offset.
 * ================================================================ */
static void gerar_lvalue_endereco(No *no, PilhaTabela *p) {
    EntradaTabela *e = buscar(p, no->valor);

    if (no->tipo == NO_IDENT_VET) {
        /* Avalia o índice; resultado vai para $t0 */
        gerar_expr(no->esq, p);
        int esz = (e->tipo == NO_TIPO_INT) ? 4 : 1;
        if (e->kind == KIND_PARAM_VET) {
            /* Parâmetro vetor: e->offset guarda onde está o PONTEIRO */
            printf("\tlw\t$t1, %d($fp)\n", e->offset);  /* carrega ponteiro */
        } else if (e->is_global) {
            /* Vetor global: base está no .data */
            printf("\tla\t$t1, _gv_%s\n", e->nome);
        } else {
            /* Vetor local: base é $fp + offset */
            printf("\taddiu\t$t1, $fp, %d\n", e->offset);
        }
        /* Multiplica índice pelo tamanho do elemento */
        if (esz == 4)
            printf("\tsll\t$t0, $t0, 2\n");
        /* (se esz == 1, não multiplica) */
        printf("\tadd\t$t1, $t1, $t0\n");  /* endereço final */
    } else {
        /* Escalar */
        if (e->is_global)
            printf("\tla\t$t1, _gv_%s\n", e->nome);
        else
            printf("\taddiu\t$t1, $fp, %d\n", e->offset);
    }
}

/* ================================================================
 * gerar_expr — gera código para uma expressão.
 * O resultado sempre fica em $t0.
 * ================================================================ */
static void gerar_expr(No *no, PilhaTabela *p) {
    if (!no) return;

    switch (no->tipo) {

        case NO_INT_CONST:
            printf("\tli\t$t0, %s\n", no->valor);
            break;

        case NO_CAR_CONST:
            printf("\tli\t$t0, %d\n", valor_carconst(no->valor));
            break;

        /* Leitura de variável escalar */
        case NO_IDENT: {
            EntradaTabela *e = buscar(p, no->valor);
            if (e->is_global) {
                if (e->tipo == NO_TIPO_INT)
                    printf("\tlw\t$t0, _gv_%s\n", e->nome);
                else
                    printf("\tlb\t$t0, _gv_%s\n", e->nome);
            } else {
                if (e->tipo == NO_TIPO_INT)
                    printf("\tlw\t$t0, %d($fp)\n", e->offset);
                else
                    printf("\tlb\t$t0, %d($fp)\n", e->offset);
            }
            break;
        }

        /* Leitura de elemento de vetor a[i] — G-V2 */
        case NO_IDENT_VET: {
            EntradaTabela *e = buscar(p, no->valor);
            gerar_lvalue_endereco(no, p);  /* endereço em $t1 */
            if (e->tipo == NO_TIPO_INT)
                printf("\tlw\t$t0, 0($t1)\n");
            else
                printf("\tlb\t$t0, 0($t1)\n");
            break;
        }

        /* Atribuição: lvalue = Expr */
        case NO_ATRIB: {
            gerar_expr(no->dir, p);  /* avalia rvalue em $t0 */
            /* Salva $t0 na pilha temporariamente */
            printf("\taddiu\t$sp, $sp, -4\n");
            printf("\tsw\t$t0, 0($sp)\n");
            /* Calcula endereço do lvalue em $t1 */
            gerar_lvalue_endereco(no->esq, p);
            /* Restaura valor em $t0 */
            printf("\tlw\t$t0, 0($sp)\n");
            printf("\taddiu\t$sp, $sp, 4\n");
            /* Armazena */
            EntradaTabela *e = buscar(p, no->esq->valor);
            if (e->tipo == NO_TIPO_INT)
                printf("\tsw\t$t0, 0($t1)\n");
            else
                printf("\tsb\t$t0, 0($t1)\n");
            break;
        }

        /* Chamada de função: f(arg1, ...) — G-V2 */
        case NO_CHAMADA_FUNC: {
            EntradaTabela *ef = buscar(p, no->valor);
            int n_args = ef->num_params;

            /* Empilha argumentos da DIREITA para a ESQUERDA
             * para que arg1 fique no topo quando o callee entrar */
            No *args[64];
            int narg = 0;
            for (No *la = no->esq; la != NULL; la = la->dir)
                args[narg++] = la->esq;

            /* Coleta ponteiros para parâmetros em array (ordem original) */
            InfoParam *ips[64];
            InfoParam *cur = ef->params;
            for (int i = 0; i < narg; i++) { ips[i] = cur; cur = cur->prox; }

            for (int i = narg - 1; i >= 0; i--) {
                if (ips[i]->eh_vetor) {
                    /* Passa o ENDEREÇO do primeiro elemento */
                    EntradaTabela *ea = buscar(p, args[i]->valor);
                    if (ea->kind == KIND_PARAM_VET) {
                        /* Já é um ponteiro */
                        printf("\tlw\t$t0, %d($fp)\n", ea->offset);
                    } else if (ea->is_global) {
                        printf("\tla\t$t0, _gv_%s\n", ea->nome);
                    } else {
                        printf("\taddiu\t$t0, $fp, %d\n", ea->offset);
                    }
                } else {
                    gerar_expr(args[i], p);  /* valor escalar em $t0 */
                }
                printf("\taddiu\t$sp, $sp, -4\n");
                printf("\tsw\t$t0, 0($sp)\n");
            }

            /* Salta para a função */
            printf("\tjal\t_func_%s\n", no->valor);

            /* Desempilha argumentos */
            if (n_args > 0)
                printf("\taddiu\t$sp, $sp, %d\n", n_args * 4);

            /* Retorno em $v0 → move para $t0 */
            printf("\tmove\t$t0, $v0\n");
            break;
        }

        /* Operadores binários: push esq, avalia dir, pop, opera */
        case NO_OP_SOMA: case NO_OP_SUB: case NO_OP_MUL: case NO_OP_DIV:
        case NO_OP_IGUAL: case NO_OP_DIFER:
        case NO_OP_MENOR: case NO_OP_MAIOR:
        case NO_OP_MENIG: case NO_OP_MAIIG:
        case NO_OP_OU:    case NO_OP_E: {
            gerar_expr(no->esq, p);
            printf("\taddiu\t$sp, $sp, -4\n");
            printf("\tsw\t$t0, 0($sp)\n");
            gerar_expr(no->dir, p);
            printf("\tlw\t$t1, 0($sp)\n");
            printf("\taddiu\t$sp, $sp, 4\n");
            /* $t1 = esq,  $t0 = dir */
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
 * gerar_comando — gera código para um comando
 * ================================================================ */
static void gerar_comando(No *no, PilhaTabela *p) {
    if (!no) return;

    switch (no->tipo) {

        case NO_CMD_EXPR:
            gerar_expr(no->esq, p);
            break;

        case NO_CMD_ESCREVA: {
            gerar_expr(no->esq, p);
            TipoNo t = analisar_expr_tipo(no->esq, p);
            if (t == NO_TIPO_INT)
                printf("\tmove\t$a0, $t0\n\tli\t$v0, 1\n\tsyscall\n");
            else
                printf("\tmove\t$a0, $t0\n\tli\t$v0, 11\n\tsyscall\n");
            break;
        }

        case NO_CMD_ESCREVA_STR: {
            int idx = registrar_string(no->valor);
            printf("\tla\t$a0, _str%d\n\tli\t$v0, 4\n\tsyscall\n", idx);
            break;
        }

        case NO_CMD_NOVALINHA:
            printf("\tli\t$a0, 10\n\tli\t$v0, 11\n\tsyscall\n");
            break;

        /* leia id; — G-V1 (e G-V2 para escalar) */
        case NO_CMD_LEIA: {
            EntradaTabela *e = buscar(p, no->valor);
            if (e->tipo == NO_TIPO_INT) {
                printf("\tli\t$v0, 5\n\tsyscall\n");
                if (e->is_global)
                    printf("\tsw\t$v0, _gv_%s\n", e->nome);
                else
                    printf("\tsw\t$v0, %d($fp)\n", e->offset);
            } else {
                printf("\tli\t$v0, 12\n\tsyscall\n");
                if (e->is_global)
                    printf("\tsb\t$v0, _gv_%s\n", e->nome);
                else
                    printf("\tsb\t$v0, %d($fp)\n", e->offset);
            }
            break;
        }

        /* leia a[i]; — G-V2 */
        case NO_CMD_LEIA_VET: {
            No *lv = no->esq;  /* NO_IDENT_VET */
            EntradaTabela *e = buscar(p, lv->valor);
            /* Calcula endereço do elemento em $t1 */
            gerar_lvalue_endereco(lv, p);
            if (e->tipo == NO_TIPO_INT) {
                printf("\tli\t$v0, 5\n\tsyscall\n");
                printf("\tsw\t$v0, 0($t1)\n");
            } else {
                printf("\tli\t$v0, 12\n\tsyscall\n");
                printf("\tsb\t$v0, 0($t1)\n");
            }
            break;
        }

        case NO_CMD_SE: {
            int lbl = novo_label();
            gerar_expr(no->esq, p);
            if (no->extra) {
                printf("\tbeq\t$t0, $zero, _Lelse_%d\n", lbl);
                gerar_comando(no->dir, p);
                printf("\tj\t_Lend_%d\n", lbl);
                printf("_Lelse_%d:\n", lbl);
                gerar_comando(no->extra, p);
            } else {
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

        /* retorne Expr; — G-V2
         *
         * Layout do frame (após prólogo de gerar_funcao):
         *   $fp+8  = arg1 (primeiro parâmetro)
         *   $fp+4  = $ra salvo
         *   $fp+0  = $fp antigo
         *   $fp-4  = primeira variável local
         *
         * Para retornar com segurança de qualquer profundidade
         * de bloco aninhado, usamos "move $sp, $fp" para limpar
         * todos os blocos internos antes de restaurar os registradores.
         */
        case NO_CMD_RETORNE: {
            gerar_expr(no->esq, p);
            printf("\tmove\t$v0, $t0\n");        /* coloca retorno em $v0    */
            printf("\tlw\t$ra, 4($fp)\n");        /* restaura $ra (em $fp+4)  */
            printf("\tlw\t$t1, 0($fp)\n");        /* lê $fp antigo (em $fp+0) */
            printf("\tmove\t$sp, $fp\n");          /* limpa blocos aninhados   */
            printf("\taddiu\t$sp, $sp, 8\n");     /* sobe past $fp e $ra      */
            printf("\tmove\t$fp, $t1\n");          /* restaura $fp             */
            printf("\tjr\t$ra\n");
            break;
        }

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
 * gerar_bloco — aloca variáveis, gera comandos, desaloca.
 *
 * Para G-V2, suporta NO_DECL_VET (vetores) com tamanho variável.
 * ================================================================ */
static void gerar_bloco(No *no, PilhaTabela *p) {
    int tem_decls = (no->esq != NULL);
    int espaco = 0;

    if (tem_decls) {
        espaco = calcular_espaco_decls(no->esq);
        if (espaco > 0)
            printf("\taddiu\t$sp, $sp, -%d\n", espaco);
        push_escopo(p);

        /* Atribui offsets: percorre todas as declarações */
        for (No *ld = no->esq; ld != NULL; ld = ld->dir) {
            No *decl = ld->esq;
            TipoNo tipo = decl->dir->tipo;

            if (decl->tipo == NO_DECL_VET) {
                /* Vetor local: elementos a[0]..a[N-1] com 4 bytes cada.
                 *
                 * Exemplo: prox_offset = -4, int[3] (b = 12):
                 *   a[0] em -12($fp)
                 *   a[1] em  -8($fp)
                 *   a[2] em  -4($fp)
                 *   Fórmula de acesso: addr = ($fp + offset) + i*4
                 *   onde offset = prox_offset - b + 4 = prox_offset_novo + 4
                 */
                int b = decl->tam * 4;
                inserir_vetor(p, decl->valor, tipo, decl->tam);
                buscar(p, decl->valor)->offset = prox_offset - (b - 4);
                prox_offset -= b;
            } else {
                /* Variável escalar (G-V1 e G-V2) */
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
    }

    gerar_lista_cmd(no->dir, p);

    if (tem_decls) {
        pop_escopo(p);
        if (espaco > 0)
            printf("\taddiu\t$sp, $sp, %d\n", espaco);
        prox_offset += espaco;
    }
}

/* ================================================================
 * gerar_funcao — gera o código de UMA função (NO_FUNC).
 *
 * Estrutura do NO_FUNC:
 *   valor = nome da função
 *   esq   = NO_LISTA_PARAM (lista de parâmetros) ou NULL
 *   dir   = tipo de retorno
 *   extra = NO_BLOCO (corpo da função)
 *
 * Convenção de chamada:
 *   - O chamador já empilhou os argumentos (arg1 no topo)
 *   - Ao entrar, $sp aponta para arg1
 *   - Prólogo: salva $ra e $fp, ajusta $fp = $sp
 *   - Corpo: variáveis locais em offsets negativos de $fp
 *   - Retorno via NO_CMD_RETORNE (que emite "jr $ra")
 * ================================================================ */
static void gerar_funcao(No *fn, PilhaTabela *p) {
    printf("\n_func_%s:\n", fn->valor);

    /* Prólogo: Ao entrar, $sp → arg1 (caller empilhou args right-to-left)
     *   sw $ra, -4($sp)  → salva $ra em arg1_addr-4
     *   sw $fp, -8($sp)  → salva $fp em arg1_addr-8
     *   addiu $sp, -8    → $sp = arg1_addr-8
     *   move $fp, $sp    → $fp = arg1_addr-8
     *
     * Após prólogo:
     *   $fp+8  = arg1,  $fp+12 = arg2, ...
     *   $fp+4  = $ra salvo
     *   $fp+0  = $fp antigo
     *   $fp-4  = primeira var local                */
    printf("\tsw\t$ra, -4($sp)\n");
    printf("\tsw\t$fp, -8($sp)\n");
    printf("\taddiu\t$sp, $sp, -8\n");
    printf("\tmove\t$fp, $sp\n\n");

    /* Salva offset do chamador e reinicia para esta função */
    int offset_salvo = prox_offset;
    prox_offset = -4;  /* primeira var local em -4($fp) */

    /* Cria escopo para os parâmetros formais */
    push_escopo(p);

    /* Parâmetros: param1 em +8($fp), param2 em +12($fp), etc.
     * (ficam ACIMA de $fp, onde o chamador os colocou) */
    int param_off = 8;
    for (No *lp = fn->esq; lp != NULL; lp = lp->dir) {
        No *param = lp->esq;  /* NO_PARAM ou NO_PARAM_VET */
        int eh_vet = (param->tipo == NO_PARAM_VET);
        inserir_parametro(p, param->valor, param->dir->tipo, eh_vet);
        buscar(p, param->valor)->offset = param_off;
        param_off += 4;
    }

    /* Gera código do corpo da função */
    gerar_bloco(fn->extra, p);

    /* Epílogo implícito — executado se não houver retorne explícito.
     * Retorna 0 por padrão. */
    printf("\tli\t$v0, 0\n");
    printf("\tlw\t$ra, 4($fp)\n");      /* $ra salvo em $fp+4 */
    printf("\tlw\t$t1, 0($fp)\n");      /* $fp antigo em $fp+0 */
    printf("\tmove\t$sp, $fp\n");
    printf("\taddiu\t$sp, $sp, 8\n");   /* passa por $ra e $fp salvos */
    printf("\tmove\t$fp, $t1\n");
    printf("\tjr\t$ra\n");

    pop_escopo(p);
    prox_offset = offset_salvo;  /* restaura estado do chamador */
}

/* ================================================================
 * Registra variáveis globais e define seus rótulos no .data
 * ================================================================ */
static void registrar_globais(No *lista_decl, PilhaTabela *p) {
    for (No *ld = lista_decl; ld != NULL; ld = ld->dir) {
        No *decl = ld->esq;
        TipoNo tipo = decl->dir->tipo;

        if (decl->tipo == NO_DECL_VET) {
            inserir_vetor(p, decl->valor, tipo, decl->tam);
            buscar(p, decl->valor)->is_global = 1;
            globais[n_globais++] = (InfoGlobal){ strdup(decl->valor), tipo, decl->tam };
        } else {
            inserir(p, decl->esq->valor, tipo);
            buscar(p, decl->esq->valor)->is_global = 1;
            globais[n_globais++] = (InfoGlobal){ strdup(decl->esq->valor), tipo, 0 };

            for (No *ex = decl->extra; ex != NULL; ex = ex->dir) {
                inserir(p, ex->esq->valor, tipo);
                buscar(p, ex->esq->valor)->is_global = 1;
                globais[n_globais++] = (InfoGlobal){ strdup(ex->esq->valor), tipo, 0 };
            }
        }
    }
}

/* ================================================================
 * gerar_codigo — ponto de entrada público
 *
 * Detecta G-V1 (NO_PROGRAMA) ou G-V2 (NO_PROGRAMA_V2) e age
 * de acordo.
 * ================================================================ */
void gerar_codigo(No *raiz) {
    /* Coleta strings para a seção .data */
    coletar_strings(raiz);

    /* --- Seção .data --- */
    printf("\t.data\n");
    /* Strings */
    for (int i = 0; i < n_strings; i++) {
        const char *s = strings[i];
        int len = strlen(s);
        printf("_str%d:\t.asciiz %.*s\n", i, len, s);
    }

    if (raiz->tipo == NO_PROGRAMA) {
        /* ── G-V1: sem variáveis globais nem funções ── */
        printf("\n\t.text\n\t.globl main\nmain:\n\tmove\t$fp, $sp\n\n");
        PilhaTabela p;
        init_pilha(&p);
        prox_offset = -4;
        gerar_bloco(raiz->esq, &p);
        printf("\n\tli\t$v0, 10\n\tsyscall\n");
        return;
    }

    /* ── G-V2 ── */

    /* Cria escopo global e declara variáveis globais no .data */
    PilhaTabela p;
    init_pilha(&p);
    push_escopo(&p);

    if (raiz->esq) {
        /* Registra vars globais na tabela e prepara o .data */
        registrar_globais(raiz->esq->esq, &p);
    }

    /* Emite rótulos das variáveis globais na seção .data */
    for (int i = 0; i < n_globais; i++) {
        InfoGlobal *g = &globais[i];
        if (g->tam > 0) {
            /* Vetor */
            int bytes = (g->tipo == NO_TIPO_INT) ? g->tam * 4 : g->tam;
            bytes = (bytes + 3) & ~3;
            printf("_gv_%s:\t.space %d\n", g->nome, bytes);
        } else {
            /* Escalar */
            if (g->tipo == NO_TIPO_INT)
                printf("_gv_%s:\t.word 0\n", g->nome);
            else
                printf("_gv_%s:\t.byte 0\n", g->nome);
        }
    }

    /* --- Seção .text --- */
    printf("\n\t.text\n");

    /* Registra funções na tabela de símbolos e gera o código de cada uma */
    if (raiz->dir) {
        for (No *lf = raiz->dir; lf != NULL; lf = lf->dir) {
            No *fn = lf->esq;

            /* Coleta info dos parâmetros */
            int num_params = 0;
            InfoParam *params_head = NULL, *params_tail = NULL;
            for (No *lp = fn->esq; lp != NULL; lp = lp->dir) {
                No *param = lp->esq;
                InfoParam *ip = malloc(sizeof(InfoParam));
                ip->nome     = strdup(param->valor);
                ip->tipo     = param->dir->tipo;
                ip->eh_vetor = (param->tipo == NO_PARAM_VET);
                ip->prox     = NULL;
                if (!params_head) params_head = ip;
                else              params_tail->prox = ip;
                params_tail = ip;
                num_params++;
            }
            inserir_func(&p, fn->valor, fn->dir->tipo, num_params, params_head);
        }

        /* Gera o código de cada função */
        for (No *lf = raiz->dir; lf != NULL; lf = lf->dir)
            gerar_funcao(lf->esq, &p);
    }

    /* --- Programa principal --- */
    printf("\n\t.globl main\nmain:\n");
    printf("\tmove\t$fp, $sp\n\n");
    prox_offset = -4;
    gerar_bloco(raiz->extra, &p);

    printf("\n\tli\t$v0, 10\n\tsyscall\n");
    pop_escopo(&p);
}
