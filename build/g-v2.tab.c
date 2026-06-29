/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 23 "src/g-v2.y"

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

#line 138 "build/g-v2.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "g-v2.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PRINCIPAL = 3,                  /* PRINCIPAL  */
  YYSYMBOL_INT = 4,                        /* INT  */
  YYSYMBOL_CAR = 5,                        /* CAR  */
  YYSYMBOL_LEIA = 6,                       /* LEIA  */
  YYSYMBOL_ESCREVA = 7,                    /* ESCREVA  */
  YYSYMBOL_NOVALINHA = 8,                  /* NOVALINHA  */
  YYSYMBOL_SE = 9,                         /* SE  */
  YYSYMBOL_ENTAO = 10,                     /* ENTAO  */
  YYSYMBOL_SENAO = 11,                     /* SENAO  */
  YYSYMBOL_FIMSE = 12,                     /* FIMSE  */
  YYSYMBOL_ENQUANTO = 13,                  /* ENQUANTO  */
  YYSYMBOL_OU = 14,                        /* OU  */
  YYSYMBOL_E = 15,                         /* E  */
  YYSYMBOL_IGUAL = 16,                     /* IGUAL  */
  YYSYMBOL_DIFERENTE = 17,                 /* DIFERENTE  */
  YYSYMBOL_MAIORIGUAL = 18,                /* MAIORIGUAL  */
  YYSYMBOL_MENORIGUAL = 19,                /* MENORIGUAL  */
  YYSYMBOL_GLOBAL = 20,                    /* GLOBAL  */
  YYSYMBOL_FUNCAO = 21,                    /* FUNCAO  */
  YYSYMBOL_RETORNE = 22,                   /* RETORNE  */
  YYSYMBOL_IDENTIFICADOR = 23,             /* IDENTIFICADOR  */
  YYSYMBOL_INTCONST = 24,                  /* INTCONST  */
  YYSYMBOL_CARCONST = 25,                  /* CARCONST  */
  YYSYMBOL_CADEIACARACTERES = 26,          /* CADEIACARACTERES  */
  YYSYMBOL_27_ = 27,                       /* '<'  */
  YYSYMBOL_28_ = 28,                       /* '>'  */
  YYSYMBOL_29_ = 29,                       /* '+'  */
  YYSYMBOL_30_ = 30,                       /* '-'  */
  YYSYMBOL_31_ = 31,                       /* '*'  */
  YYSYMBOL_32_ = 32,                       /* '/'  */
  YYSYMBOL_33_ = 33,                       /* '!'  */
  YYSYMBOL_UMINUS = 34,                    /* UMINUS  */
  YYSYMBOL_35_ = 35,                       /* '['  */
  YYSYMBOL_36_ = 36,                       /* ']'  */
  YYSYMBOL_37_ = 37,                       /* '('  */
  YYSYMBOL_38_ = 38,                       /* ')'  */
  YYSYMBOL_39_ = 39,                       /* ':'  */
  YYSYMBOL_40_ = 40,                       /* ','  */
  YYSYMBOL_41_ = 41,                       /* '{'  */
  YYSYMBOL_42_ = 42,                       /* '}'  */
  YYSYMBOL_43_ = 43,                       /* ';'  */
  YYSYMBOL_44_ = 44,                       /* '='  */
  YYSYMBOL_YYACCEPT = 45,                  /* $accept  */
  YYSYMBOL_Programa = 46,                  /* Programa  */
  YYSYMBOL_DeclVarGlobais = 47,            /* DeclVarGlobais  */
  YYSYMBOL_DeclFunc = 48,                  /* DeclFunc  */
  YYSYMBOL_Funcao = 49,                    /* Funcao  */
  YYSYMBOL_ListaFuncoes = 50,              /* ListaFuncoes  */
  YYSYMBOL_ListaParametros = 51,           /* ListaParametros  */
  YYSYMBOL_ListaParametrosTail = 52,       /* ListaParametrosTail  */
  YYSYMBOL_DeclPrograma = 53,              /* DeclPrograma  */
  YYSYMBOL_Bloco = 54,                     /* Bloco  */
  YYSYMBOL_VarSection = 55,                /* VarSection  */
  YYSYMBOL_ListaDeclVar = 56,              /* ListaDeclVar  */
  YYSYMBOL_ListVar = 57,                   /* ListVar  */
  YYSYMBOL_Tipo = 58,                      /* Tipo  */
  YYSYMBOL_ListaComando = 59,              /* ListaComando  */
  YYSYMBOL_Comando = 60,                   /* Comando  */
  YYSYMBOL_LValueExpr = 61,                /* LValueExpr  */
  YYSYMBOL_Expr = 62,                      /* Expr  */
  YYSYMBOL_OrExpr = 63,                    /* OrExpr  */
  YYSYMBOL_AndExpr = 64,                   /* AndExpr  */
  YYSYMBOL_EqExpr = 65,                    /* EqExpr  */
  YYSYMBOL_DesigExpr = 66,                 /* DesigExpr  */
  YYSYMBOL_AddExpr = 67,                   /* AddExpr  */
  YYSYMBOL_MulExpr = 68,                   /* MulExpr  */
  YYSYMBOL_UnExpr = 69,                    /* UnExpr  */
  YYSYMBOL_PrimExpr = 70,                  /* PrimExpr  */
  YYSYMBOL_ListExpr = 71                   /* ListExpr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   170

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  27
/* YYNRULES -- Number of rules.  */
#define YYNRULES  74
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  158

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   282


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    33,     2,     2,     2,     2,     2,     2,
      37,    38,    31,    29,    40,    30,     2,    32,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    39,    43,
      27,    44,    28,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    35,     2,    36,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    41,     2,    42,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    34
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   152,   152,   163,   166,   178,   191,   203,   211,   214,
     225,   228,   233,   238,   243,   248,   258,   269,   271,   279,
     292,   294,   306,   310,   314,   318,   327,   328,   335,   337,
     342,   345,   349,   353,   359,   362,   365,   368,   371,   374,
     377,   388,   391,   402,   404,   409,   411,   416,   418,   423,
     425,   427,   432,   434,   436,   438,   440,   445,   447,   449,
     454,   456,   458,   463,   465,   467,   480,   485,   490,   495,
     497,   499,   501,   513,   515
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "PRINCIPAL", "INT",
  "CAR", "LEIA", "ESCREVA", "NOVALINHA", "SE", "ENTAO", "SENAO", "FIMSE",
  "ENQUANTO", "OU", "E", "IGUAL", "DIFERENTE", "MAIORIGUAL", "MENORIGUAL",
  "GLOBAL", "FUNCAO", "RETORNE", "IDENTIFICADOR", "INTCONST", "CARCONST",
  "CADEIACARACTERES", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'!'",
  "UMINUS", "'['", "']'", "'('", "')'", "':'", "','", "'{'", "'}'", "';'",
  "'='", "$accept", "Programa", "DeclVarGlobais", "DeclFunc", "Funcao",
  "ListaFuncoes", "ListaParametros", "ListaParametrosTail", "DeclPrograma",
  "Bloco", "VarSection", "ListaDeclVar", "ListVar", "Tipo", "ListaComando",
  "Comando", "LValueExpr", "Expr", "OrExpr", "AndExpr", "EqExpr",
  "DesigExpr", "AddExpr", "MulExpr", "UnExpr", "PrimExpr", "ListExpr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-136)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-43)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       0,    20,    52,    55,    48,  -136,  -136,    45,    99,   -21,
      71,    70,    94,   -28,  -136,    95,    48,  -136,    30,    81,
      94,    24,  -136,    79,    85,  -136,  -136,  -136,    80,   101,
      94,    86,   102,    49,    83,    90,    91,    64,    16,  -136,
    -136,     1,     1,    64,  -136,  -136,    88,    24,    87,    89,
     119,   120,    12,    50,    33,    53,  -136,  -136,    24,    96,
      48,     6,   100,  -136,  -136,  -136,   104,    97,    98,   103,
    -136,    64,    64,   105,    64,   -15,    21,  -136,  -136,   106,
    -136,  -136,    64,  -136,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    92,    48,  -136,   107,
      30,   108,    64,  -136,  -136,  -136,   111,   112,  -136,   109,
    -136,  -136,    26,    64,  -136,  -136,   120,    12,    50,    50,
      33,    33,    33,    33,    53,    53,  -136,  -136,  -136,  -136,
     113,   114,    30,   115,   127,    24,   116,  -136,    64,   117,
      30,   101,   -28,  -136,    24,  -136,  -136,  -136,   118,  -136,
    -136,    84,   101,    24,  -136,  -136,   130,  -136
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,     0,     6,     0,     3,     1,     0,     0,    24,
       0,     0,     0,     0,     2,     0,     0,    19,     0,     0,
       9,     0,    16,     0,     0,    22,    26,    27,     0,    11,
       9,     0,     0,     0,     0,     0,     0,     0,    69,    70,
      71,     0,     0,     0,    30,    40,     0,    28,     0,     0,
      44,    46,    48,    51,    56,    59,    62,    65,     0,    25,
      21,     0,     0,    10,     8,     5,    42,     0,     0,     0,
      36,     0,     0,     0,     0,     0,    69,    63,    64,     0,
      17,    29,     0,    31,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
       0,     0,     0,    33,    35,    34,     0,     0,    32,     0,
      67,    73,     0,     0,    72,    43,    45,    47,    49,    50,
      54,    55,    52,    53,    57,    58,    60,    61,    18,    23,
       0,    12,     0,     0,     0,     0,    68,    66,     0,     0,
       0,     0,     0,    41,     0,    39,    74,    68,    13,    14,
       7,     0,     0,     0,    37,    15,     0,    38
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -136,  -136,  -136,  -136,   143,   126,  -136,  -135,  -136,   -13,
     156,   110,   -14,   -96,   -31,  -132,   129,   -32,  -136,    78,
      82,    17,     2,    18,    19,    74,  -136
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     3,     8,    30,    31,    62,    63,    14,    45,
      23,    10,    11,    28,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,   112
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      22,    69,    25,   145,   131,    73,   149,     4,    38,    39,
      40,    79,   151,    21,    15,    41,    81,   155,    42,    16,
       1,   156,    43,   110,    76,    39,    40,    96,    86,    87,
      32,    33,    34,    35,    26,    27,   142,    36,    43,   106,
     107,    99,   109,   111,   148,   100,    37,    38,    39,    40,
     115,    74,     6,    75,    41,     4,   113,    42,    75,     4,
     -42,    43,    92,    93,   137,    21,   138,    44,    88,    89,
     133,     9,    38,    39,    40,    68,     7,    90,    91,    41,
      12,   139,    42,   129,    94,    95,    43,    38,    39,    40,
     120,   121,   122,   123,    41,   153,   154,    42,    76,    39,
      40,    43,    13,   118,   119,    41,   146,    17,    42,    18,
     124,   125,    43,   126,   127,    77,    78,    19,    29,    24,
      58,    59,    65,    60,    61,    66,    70,    71,    72,   150,
      80,    82,    83,    84,   128,    85,    97,   144,   101,   102,
     103,   104,   157,   130,   114,   136,   105,   132,   108,   134,
     135,   143,   140,   147,   141,    20,    64,     5,   152,     0,
     -41,    67,   116,     0,     0,     0,     0,   117,     0,     0,
      98
};

static const yytype_int16 yycheck[] =
{
      13,    33,    16,   135,   100,    37,   141,    35,    23,    24,
      25,    43,   144,    41,    35,    30,    47,   152,    33,    40,
      20,   153,    37,    38,    23,    24,    25,    58,    16,    17,
       6,     7,     8,     9,     4,     5,   132,    13,    37,    71,
      72,    35,    74,    75,   140,    39,    22,    23,    24,    25,
      82,    35,     0,    37,    30,    35,    35,    33,    37,    35,
      44,    37,    29,    30,    38,    41,    40,    43,    18,    19,
     102,    23,    23,    24,    25,    26,    21,    27,    28,    30,
      35,   113,    33,    97,    31,    32,    37,    23,    24,    25,
      88,    89,    90,    91,    30,    11,    12,    33,    23,    24,
      25,    37,     3,    86,    87,    30,   138,    36,    33,    39,
      92,    93,    37,    94,    95,    41,    42,    23,    37,    24,
      41,    36,    36,    43,    23,    23,    43,    37,    37,   142,
      42,    44,    43,    14,    42,    15,    40,    10,    38,    35,
      43,    43,    12,    36,    38,    36,    43,    39,    43,    38,
      38,    36,    39,    36,    40,    12,    30,     1,    40,    -1,
      44,    32,    84,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      60
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    20,    46,    47,    35,    55,     0,    21,    48,    23,
      56,    57,    35,     3,    53,    35,    40,    36,    39,    23,
      49,    41,    54,    55,    24,    57,     4,     5,    58,    37,
      49,    50,     6,     7,     8,     9,    13,    22,    23,    24,
      25,    30,    33,    37,    43,    54,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    41,    36,
      43,    23,    51,    52,    50,    36,    23,    61,    26,    62,
      43,    37,    37,    62,    35,    37,    23,    70,    70,    62,
      42,    59,    44,    43,    14,    15,    16,    17,    18,    19,
      27,    28,    29,    30,    31,    32,    59,    40,    56,    35,
      39,    38,    35,    43,    43,    43,    62,    62,    43,    62,
      38,    62,    71,    35,    38,    62,    64,    65,    66,    66,
      67,    67,    67,    67,    68,    68,    69,    69,    42,    57,
      36,    58,    39,    62,    38,    38,    36,    38,    40,    62,
      39,    40,    58,    36,    10,    60,    62,    36,    58,    52,
      54,    60,    40,    11,    12,    52,    60,    12
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    45,    46,    47,    47,    48,    48,    49,    50,    50,
      51,    51,    52,    52,    52,    52,    53,    54,    54,    55,
      56,    56,    57,    57,    57,    57,    58,    58,    59,    59,
      60,    60,    60,    60,    60,    60,    60,    60,    60,    60,
      60,    61,    61,    62,    62,    63,    63,    64,    64,    65,
      65,    65,    66,    66,    66,    66,    66,    67,    67,    67,
      68,    68,    68,    69,    69,    69,    70,    70,    70,    70,
      70,    70,    70,    71,    71
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     2,     0,     5,     0,     7,     2,     0,
       1,     0,     3,     5,     5,     7,     2,     3,     4,     3,
       5,     4,     3,     6,     1,     4,     1,     1,     1,     2,
       1,     2,     3,     3,     3,     3,     2,     7,     9,     5,
       1,     4,     1,     3,     1,     3,     1,     3,     1,     3,
       3,     1,     3,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     1,     2,     2,     1,     4,     3,     4,     1,
       1,     1,     3,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Programa: DeclVarGlobais DeclFunc DeclPrograma  */
#line 153 "src/g-v2.y"
        { raiz_ast = criar_no3(NO_PROGRAMA_V2, yylineno, (yyvsp[-2].no), (yyvsp[-1].no), (yyvsp[0].no)); }
#line 1298 "build/g-v2.tab.c"
    break;

  case 3: /* DeclVarGlobais: GLOBAL VarSection  */
#line 164 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_DECL_VAR_GLOBAIS, yylineno, (yyvsp[0].no), NULL); }
#line 1304 "build/g-v2.tab.c"
    break;

  case 4: /* DeclVarGlobais: %empty  */
#line 166 "src/g-v2.y"
        { (yyval.no) = NULL; }
#line 1310 "build/g-v2.tab.c"
    break;

  case 5: /* DeclFunc: FUNCAO '[' Funcao ListaFuncoes ']'  */
#line 179 "src/g-v2.y"
        { /* $3 = primeira função, $4 = restante */
          /* Encadeia: cria cabeça da lista */
          No *lista = criar_no(NO_DECL_FUNC, yylineno, (yyvsp[-2].no), NULL);
          /* Encadeia o restante */
          No *cur = lista;
          for (No *lf = (yyvsp[-1].no); lf != NULL; lf = lf->dir) {
              No *nova = criar_no(NO_DECL_FUNC, yylineno, lf->esq, NULL);
              cur->dir = nova;
              cur = nova;
          }
          (yyval.no) = lista; }
#line 1326 "build/g-v2.tab.c"
    break;

  case 6: /* DeclFunc: %empty  */
#line 191 "src/g-v2.y"
        { (yyval.no) = NULL; }
#line 1332 "build/g-v2.tab.c"
    break;

  case 7: /* Funcao: IDENTIFICADOR '(' ListaParametros ')' ':' Tipo Bloco  */
#line 204 "src/g-v2.y"
        { No *fn = criar_no3(NO_FUNC, yylineno, (yyvsp[-4].no), (yyvsp[-1].no), (yyvsp[0].no));
          fn->valor = (yyvsp[-6].str);
          (yyval.no) = fn; }
#line 1340 "build/g-v2.tab.c"
    break;

  case 8: /* ListaFuncoes: Funcao ListaFuncoes  */
#line 212 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_DECL_FUNC, yylineno, (yyvsp[-1].no), (yyvsp[0].no)); }
#line 1346 "build/g-v2.tab.c"
    break;

  case 9: /* ListaFuncoes: %empty  */
#line 214 "src/g-v2.y"
        { (yyval.no) = NULL; }
#line 1352 "build/g-v2.tab.c"
    break;

  case 10: /* ListaParametros: ListaParametrosTail  */
#line 226 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1358 "build/g-v2.tab.c"
    break;

  case 11: /* ListaParametros: %empty  */
#line 228 "src/g-v2.y"
        { (yyval.no) = NULL; }
#line 1364 "build/g-v2.tab.c"
    break;

  case 12: /* ListaParametrosTail: IDENTIFICADOR ':' Tipo  */
#line 234 "src/g-v2.y"
        { No *param = criar_no(NO_PARAM, yylineno, NULL, (yyvsp[0].no));
          param->valor = (yyvsp[-2].str);
          (yyval.no) = criar_no(NO_LISTA_PARAM, yylineno, param, NULL); }
#line 1372 "build/g-v2.tab.c"
    break;

  case 13: /* ListaParametrosTail: IDENTIFICADOR '[' ']' ':' Tipo  */
#line 239 "src/g-v2.y"
        { No *param = criar_no(NO_PARAM_VET, yylineno, NULL, (yyvsp[0].no));
          param->valor = (yyvsp[-4].str);
          (yyval.no) = criar_no(NO_LISTA_PARAM, yylineno, param, NULL); }
#line 1380 "build/g-v2.tab.c"
    break;

  case 14: /* ListaParametrosTail: IDENTIFICADOR ':' Tipo ',' ListaParametrosTail  */
#line 244 "src/g-v2.y"
        { No *param = criar_no(NO_PARAM, yylineno, NULL, (yyvsp[-2].no));
          param->valor = (yyvsp[-4].str);
          (yyval.no) = criar_no(NO_LISTA_PARAM, yylineno, param, (yyvsp[0].no)); }
#line 1388 "build/g-v2.tab.c"
    break;

  case 15: /* ListaParametrosTail: IDENTIFICADOR '[' ']' ':' Tipo ',' ListaParametrosTail  */
#line 249 "src/g-v2.y"
        { No *param = criar_no(NO_PARAM_VET, yylineno, NULL, (yyvsp[-2].no));
          param->valor = (yyvsp[-6].str);
          (yyval.no) = criar_no(NO_LISTA_PARAM, yylineno, param, (yyvsp[0].no)); }
#line 1396 "build/g-v2.tab.c"
    break;

  case 16: /* DeclPrograma: PRINCIPAL Bloco  */
#line 259 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1402 "build/g-v2.tab.c"
    break;

  case 17: /* Bloco: '{' ListaComando '}'  */
#line 270 "src/g-v2.y"
        { (yyval.no) = criar_no3(NO_BLOCO, yylineno, NULL, (yyvsp[-1].no), NULL); }
#line 1408 "build/g-v2.tab.c"
    break;

  case 18: /* Bloco: VarSection '{' ListaComando '}'  */
#line 272 "src/g-v2.y"
        { (yyval.no) = criar_no3(NO_BLOCO, yylineno, (yyvsp[-3].no), (yyvsp[-1].no), NULL); }
#line 1414 "build/g-v2.tab.c"
    break;

  case 19: /* VarSection: '[' ListaDeclVar ']'  */
#line 280 "src/g-v2.y"
        { (yyval.no) = (yyvsp[-1].no); }
#line 1420 "build/g-v2.tab.c"
    break;

  case 20: /* ListaDeclVar: ListVar ':' Tipo ';' ListaDeclVar  */
#line 293 "src/g-v2.y"
        { (yyval.no) = distribuir_tipo_lista((yyvsp[-4].no), (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1426 "build/g-v2.tab.c"
    break;

  case 21: /* ListaDeclVar: ListVar ':' Tipo ';'  */
#line 295 "src/g-v2.y"
        { (yyval.no) = distribuir_tipo_lista((yyvsp[-3].no), (yyvsp[-1].no), NULL); }
#line 1432 "build/g-v2.tab.c"
    break;

  case 22: /* ListVar: IDENTIFICADOR ',' ListVar  */
#line 307 "src/g-v2.y"
        { No *id = criar_no_folha(NO_IDENT, yylineno, (yyvsp[-2].str));
          (yyval.no) = criar_no(NO_LISTA_DECL, yylineno, id, (yyvsp[0].no)); }
#line 1439 "build/g-v2.tab.c"
    break;

  case 23: /* ListVar: IDENTIFICADOR '[' INTCONST ']' ',' ListVar  */
#line 311 "src/g-v2.y"
        { No *vet = criar_no_folha_tam(NO_DECL_VET, yylineno, (yyvsp[-5].str), atoi((yyvsp[-3].str)));
          (yyval.no) = criar_no(NO_LISTA_DECL, yylineno, vet, (yyvsp[0].no)); }
#line 1446 "build/g-v2.tab.c"
    break;

  case 24: /* ListVar: IDENTIFICADOR  */
#line 315 "src/g-v2.y"
        { No *id = criar_no_folha(NO_IDENT, yylineno, (yyvsp[0].str));
          (yyval.no) = criar_no(NO_LISTA_DECL, yylineno, id, NULL); }
#line 1453 "build/g-v2.tab.c"
    break;

  case 25: /* ListVar: IDENTIFICADOR '[' INTCONST ']'  */
#line 319 "src/g-v2.y"
        { No *vet = criar_no_folha_tam(NO_DECL_VET, yylineno, (yyvsp[-3].str), atoi((yyvsp[-1].str)));
          (yyval.no) = criar_no(NO_LISTA_DECL, yylineno, vet, NULL); }
#line 1460 "build/g-v2.tab.c"
    break;

  case 26: /* Tipo: INT  */
#line 327 "src/g-v2.y"
           { (yyval.no) = criar_no(NO_TIPO_INT, yylineno, NULL, NULL); }
#line 1466 "build/g-v2.tab.c"
    break;

  case 27: /* Tipo: CAR  */
#line 328 "src/g-v2.y"
           { (yyval.no) = criar_no(NO_TIPO_CAR, yylineno, NULL, NULL); }
#line 1472 "build/g-v2.tab.c"
    break;

  case 28: /* ListaComando: Comando  */
#line 336 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_LISTA_CMD, yylineno, (yyvsp[0].no), NULL); }
#line 1478 "build/g-v2.tab.c"
    break;

  case 29: /* ListaComando: Comando ListaComando  */
#line 338 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_LISTA_CMD, yylineno, (yyvsp[-1].no), (yyvsp[0].no)); }
#line 1484 "build/g-v2.tab.c"
    break;

  case 30: /* Comando: ';'  */
#line 343 "src/g-v2.y"
        { (yyval.no) = NULL; }
#line 1490 "build/g-v2.tab.c"
    break;

  case 31: /* Comando: Expr ';'  */
#line 346 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_CMD_EXPR, yylineno, (yyvsp[-1].no), NULL); }
#line 1496 "build/g-v2.tab.c"
    break;

  case 32: /* Comando: RETORNE Expr ';'  */
#line 350 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_CMD_RETORNE, yylineno, (yyvsp[-1].no), NULL); }
#line 1502 "build/g-v2.tab.c"
    break;

  case 33: /* Comando: LEIA LValueExpr ';'  */
#line 354 "src/g-v2.y"
        { if ((yyvsp[-1].no)->tipo == NO_IDENT_VET)
              (yyval.no) = criar_no(NO_CMD_LEIA_VET, yylineno, (yyvsp[-1].no), NULL);
          else
              (yyval.no) = criar_no_folha(NO_CMD_LEIA, yylineno, (yyvsp[-1].no)->valor); }
#line 1511 "build/g-v2.tab.c"
    break;

  case 34: /* Comando: ESCREVA Expr ';'  */
#line 360 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_CMD_ESCREVA, yylineno, (yyvsp[-1].no), NULL); }
#line 1517 "build/g-v2.tab.c"
    break;

  case 35: /* Comando: ESCREVA CADEIACARACTERES ';'  */
#line 363 "src/g-v2.y"
        { (yyval.no) = criar_no_folha(NO_CMD_ESCREVA_STR, yylineno, (yyvsp[-1].str)); }
#line 1523 "build/g-v2.tab.c"
    break;

  case 36: /* Comando: NOVALINHA ';'  */
#line 366 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_CMD_NOVALINHA, yylineno, NULL, NULL); }
#line 1529 "build/g-v2.tab.c"
    break;

  case 37: /* Comando: SE '(' Expr ')' ENTAO Comando FIMSE  */
#line 369 "src/g-v2.y"
        { (yyval.no) = criar_no3(NO_CMD_SE, yylineno, (yyvsp[-4].no), (yyvsp[-1].no), NULL); }
#line 1535 "build/g-v2.tab.c"
    break;

  case 38: /* Comando: SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE  */
#line 372 "src/g-v2.y"
        { (yyval.no) = criar_no3(NO_CMD_SE, yylineno, (yyvsp[-6].no), (yyvsp[-3].no), (yyvsp[-1].no)); }
#line 1541 "build/g-v2.tab.c"
    break;

  case 39: /* Comando: ENQUANTO '(' Expr ')' Comando  */
#line 375 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_CMD_ENQUANTO, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1547 "build/g-v2.tab.c"
    break;

  case 40: /* Comando: Bloco  */
#line 378 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1553 "build/g-v2.tab.c"
    break;

  case 41: /* LValueExpr: IDENTIFICADOR '[' Expr ']'  */
#line 389 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_IDENT_VET, yylineno, (yyvsp[-1].no), NULL);
          (yyval.no)->valor = (yyvsp[-3].str); }
#line 1560 "build/g-v2.tab.c"
    break;

  case 42: /* LValueExpr: IDENTIFICADOR  */
#line 392 "src/g-v2.y"
        { (yyval.no) = criar_no_folha(NO_IDENT, yylineno, (yyvsp[0].str)); }
#line 1566 "build/g-v2.tab.c"
    break;

  case 43: /* Expr: LValueExpr '=' Expr  */
#line 403 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_ATRIB, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1572 "build/g-v2.tab.c"
    break;

  case 44: /* Expr: OrExpr  */
#line 405 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1578 "build/g-v2.tab.c"
    break;

  case 45: /* OrExpr: OrExpr OU AndExpr  */
#line 410 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_OU, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1584 "build/g-v2.tab.c"
    break;

  case 46: /* OrExpr: AndExpr  */
#line 412 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1590 "build/g-v2.tab.c"
    break;

  case 47: /* AndExpr: AndExpr E EqExpr  */
#line 417 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_E, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1596 "build/g-v2.tab.c"
    break;

  case 48: /* AndExpr: EqExpr  */
#line 419 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1602 "build/g-v2.tab.c"
    break;

  case 49: /* EqExpr: EqExpr IGUAL DesigExpr  */
#line 424 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_IGUAL, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1608 "build/g-v2.tab.c"
    break;

  case 50: /* EqExpr: EqExpr DIFERENTE DesigExpr  */
#line 426 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_DIFER, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1614 "build/g-v2.tab.c"
    break;

  case 51: /* EqExpr: DesigExpr  */
#line 428 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1620 "build/g-v2.tab.c"
    break;

  case 52: /* DesigExpr: DesigExpr '<' AddExpr  */
#line 433 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_MENOR, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1626 "build/g-v2.tab.c"
    break;

  case 53: /* DesigExpr: DesigExpr '>' AddExpr  */
#line 435 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_MAIOR, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1632 "build/g-v2.tab.c"
    break;

  case 54: /* DesigExpr: DesigExpr MAIORIGUAL AddExpr  */
#line 437 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_MAIIG, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1638 "build/g-v2.tab.c"
    break;

  case 55: /* DesigExpr: DesigExpr MENORIGUAL AddExpr  */
#line 439 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_MENIG, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1644 "build/g-v2.tab.c"
    break;

  case 56: /* DesigExpr: AddExpr  */
#line 441 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1650 "build/g-v2.tab.c"
    break;

  case 57: /* AddExpr: AddExpr '+' MulExpr  */
#line 446 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_SOMA, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1656 "build/g-v2.tab.c"
    break;

  case 58: /* AddExpr: AddExpr '-' MulExpr  */
#line 448 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_SUB, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1662 "build/g-v2.tab.c"
    break;

  case 59: /* AddExpr: MulExpr  */
#line 450 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1668 "build/g-v2.tab.c"
    break;

  case 60: /* MulExpr: MulExpr '*' UnExpr  */
#line 455 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_MUL, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1674 "build/g-v2.tab.c"
    break;

  case 61: /* MulExpr: MulExpr '/' UnExpr  */
#line 457 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_DIV, yylineno, (yyvsp[-2].no), (yyvsp[0].no)); }
#line 1680 "build/g-v2.tab.c"
    break;

  case 62: /* MulExpr: UnExpr  */
#line 459 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1686 "build/g-v2.tab.c"
    break;

  case 63: /* UnExpr: '-' PrimExpr  */
#line 464 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_NEG, yylineno, (yyvsp[0].no), NULL); }
#line 1692 "build/g-v2.tab.c"
    break;

  case 64: /* UnExpr: '!' PrimExpr  */
#line 466 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_OP_NAO, yylineno, (yyvsp[0].no), NULL); }
#line 1698 "build/g-v2.tab.c"
    break;

  case 65: /* UnExpr: PrimExpr  */
#line 468 "src/g-v2.y"
        { (yyval.no) = (yyvsp[0].no); }
#line 1704 "build/g-v2.tab.c"
    break;

  case 66: /* PrimExpr: IDENTIFICADOR '(' ListExpr ')'  */
#line 481 "src/g-v2.y"
        { No *call = criar_no(NO_CHAMADA_FUNC, yylineno, (yyvsp[-1].no), NULL);
          call->valor = (yyvsp[-3].str);
          (yyval.no) = call; }
#line 1712 "build/g-v2.tab.c"
    break;

  case 67: /* PrimExpr: IDENTIFICADOR '(' ')'  */
#line 486 "src/g-v2.y"
        { No *call = criar_no(NO_CHAMADA_FUNC, yylineno, NULL, NULL);
          call->valor = (yyvsp[-2].str);
          (yyval.no) = call; }
#line 1720 "build/g-v2.tab.c"
    break;

  case 68: /* PrimExpr: IDENTIFICADOR '[' Expr ']'  */
#line 491 "src/g-v2.y"
        { No *vet = criar_no(NO_IDENT_VET, yylineno, (yyvsp[-1].no), NULL);
          vet->valor = (yyvsp[-3].str);
          (yyval.no) = vet; }
#line 1728 "build/g-v2.tab.c"
    break;

  case 69: /* PrimExpr: IDENTIFICADOR  */
#line 496 "src/g-v2.y"
        { (yyval.no) = criar_no_folha(NO_IDENT, yylineno, (yyvsp[0].str)); }
#line 1734 "build/g-v2.tab.c"
    break;

  case 70: /* PrimExpr: INTCONST  */
#line 498 "src/g-v2.y"
        { (yyval.no) = criar_no_folha(NO_INT_CONST, yylineno, (yyvsp[0].str)); }
#line 1740 "build/g-v2.tab.c"
    break;

  case 71: /* PrimExpr: CARCONST  */
#line 500 "src/g-v2.y"
        { (yyval.no) = criar_no_folha(NO_CAR_CONST, yylineno, (yyvsp[0].str)); }
#line 1746 "build/g-v2.tab.c"
    break;

  case 72: /* PrimExpr: '(' Expr ')'  */
#line 502 "src/g-v2.y"
        { (yyval.no) = (yyvsp[-1].no); }
#line 1752 "build/g-v2.tab.c"
    break;

  case 73: /* ListExpr: Expr  */
#line 514 "src/g-v2.y"
        { (yyval.no) = criar_no(NO_LISTA_ARGS, yylineno, (yyvsp[0].no), NULL); }
#line 1758 "build/g-v2.tab.c"
    break;

  case 74: /* ListExpr: ListExpr ',' Expr  */
#line 516 "src/g-v2.y"
        { /* Encadeia novo argumento ao final da lista */
          No *novo = criar_no(NO_LISTA_ARGS, yylineno, (yyvsp[0].no), NULL);
          /* Percorre até o fim para encadear */
          No *cur = (yyvsp[-2].no);
          while (cur->dir) cur = cur->dir;
          cur->dir = novo;
          (yyval.no) = (yyvsp[-2].no); }
#line 1770 "build/g-v2.tab.c"
    break;


#line 1774 "build/g-v2.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 525 "src/g-v2.y"


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
