/* ================================================================
 * g-v1.l — Especificação do Analisador Léxico (Flex)
 * ================================================================ */

%option yylineno
%option noyywrap

%{
#include "../include/ast.h"
#include "g-v1.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%x COMMENT

LETRA       [a-zA-Z_]
DIGITO      [0-9]
ALFANUM     [a-zA-Z0-9_]

%%

    /* --- Comentarios estilo C --- */
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             { }
<COMMENT>.              { }
<COMMENT><<EOF>>        { printf("ERRO: COMENTARIO NAO TERMINA %d\n", yylineno); exit(1); }

    /* --- Palavras reservadas --- */
"principal"             { return PRINCIPAL;  }
"int"                   { return INT;        }
"car"                   { return CAR;        }
"leia"                  { return LEIA;       }
"escreva"               { return ESCREVA;    }
"novalinha"             { return NOVALINHA;  }
"se"                    { return SE;         }
"entao"                 { return ENTAO;      }
"senao"                 { return SENAO;      }
"fimse"                 { return FIMSE;      }
"enquanto"              { return ENQUANTO;   }

    /* --- Identificador --- */
{LETRA}{ALFANUM}*       { yylval.str = strdup(yytext); return IDENTIFICADOR; }

    /* --- Constante inteira --- */
{DIGITO}+               { yylval.str = strdup(yytext); return INTCONST; }

    /* --- Constante de caractere --- */
'\\.'                   { yylval.str = strdup(yytext); return CARCONST; }
'[^'\\]'                { yylval.str = strdup(yytext); return CARCONST; }

    /* --- Cadeia de caracteres --- */
\"[^"\n]*\"             { yylval.str = strdup(yytext); return CADEIACARACTERES; }
\"[^"\n]*\n             { printf("ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA %d\n", yylineno - 1); exit(1); }

    /* --- Operadores de dois caracteres --- */
"=="                    { return IGUAL;      }
"!="                    { return DIFERENTE;  }
">="                    { return MAIORIGUAL; }
"<="                    { return MENORIGUAL; }
"||"                    { return OU;         }
"&"                     { return E;          }

    /* --- Operadores de um caractere --- */
"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"="                     { return '='; }
"!"                     { return '!'; }
"<"                     { return '<'; }
">"                     { return '>'; }
"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }
";"                     { return ';'; }
","                     { return ','; }
":"                     { return ':'; }

    /* --- Espaços em branco --- */
[ \t\r\n]+              { }

    /* --- Fim de arquivo --- */
<<EOF>>                 { return -1; }

    /* --- Caractere inválido --- */
.                       { printf("ERRO: CARACTERE INVALIDO %d\n", yylineno); exit(1); }

%%
