# Compilador G-V1

Compilador completo para a linguagem **G-V1** — uma linguagem imperativa simples com variáveis `int`/`car`, estruturas de controle (`se`/`enquanto`) e operações de I/O (`leia`/`escreva`). O compilador gera código **assembly MIPS** executável nos simuladores SPIM ou MARS.

---

## Dependências

- `gcc`
- `flex`
- `bison`

---

## Como compilar

```bash
make
```

Para limpar os artefatos gerados:

```bash
make clean
```

---

## Como usar

```bash
./g-v1 <arquivo.g>
```

O assembly MIPS é emitido na **saída padrão**. Para salvar em arquivo:

```bash
./g-v1 tests/Testes-G.v1/Corretos/fatorial.g > saida.asm
```

Para executar no simulador SPIM:

```bash
spim -file saida.asm
```

---

## Como rodar os testes

### Testar um programa correto (deve compilar sem erros)

```bash
./g-v1 tests/Testes-G.v1/Corretos/fatorial.g
```

### Testar um erro sintático (deve imprimir erro de sintaxe)

```bash
./g-v1 "tests/Testes-G.v1/ErroSintatico/NotaEmConceitoPonto-e-vrigulaFaltandoLin20.g"
```

### Testar um erro semântico (deve imprimir erro semântico e linha)

```bash
./g-v1 "tests/Testes-G.v1/ErroSemantico/fatorial-VariavelNaoDeclaradaLin19.g"
```

### Rodar todos os testes de uma categoria

```bash
for f in tests/Testes-G.v1/Corretos/*.g; do
    echo "=== $f ==="; ./g-v1 "$f" > /dev/null && echo "OK"
done
```

```bash
for f in tests/Testes-G.v1/ErroSintatico/*.g; do
    echo "=== $f ==="; ./g-v1 "$f" 2>&1 | head -1
done
```

```bash
for f in tests/Testes-G.v1/ErroSemantico/*.g; do
    echo "=== $f ==="; ./g-v1 "$f" 2>&1 | head -1
done
```

---

## Estrutura de arquivos

```
.
├── Makefile
├── include/          # Cabeçalhos (interfaces entre módulos)
│   ├── ast.h
│   ├── symtab.h
│   ├── semantico.h
│   └── codegen.h
├── src/              # Código-fonte
│   ├── g-v1.l        # Especificação do analisador léxico (Flex)
│   ├── g-v1.y        # Especificação do parser (Bison)
│   ├── ast.c         # Implementação da AST
│   ├── symtab.c      # Tabela de símbolos
│   ├── semantico.c   # Analisador semântico
│   └── codegen.c     # Gerador de código MIPS
├── build/            # Arquivos gerados (não editar manualmente)
└── tests/
    └── Testes-G.v1/
        ├── Corretos/         # Programas válidos
        ├── ErroSintatico/    # Programas com erro sintático
        └── ErroSemantico/    # Programas com erro semântico
```

---

## Pipeline de compilação

```
Código-fonte (.g)
       │
       ▼
  [1] Análise Léxica     (Flex   → g-v1.l)
       │  tokens
       ▼
  [2] Análise Sintática  (Bison  → g-v1.y)   ← constrói a AST
       │  AST
       ▼
  [3] Análise Semântica  (C      → semantico.c)
       │  AST verificada
       ▼
  [4] Geração de Código  (C      → codegen.c)
       │
       ▼
  Assembly MIPS (stdout)
```

---

## Descrição de cada módulo

### `src/g-v1.l` — Analisador Léxico (Flex)

**Conceito:** O analisador léxico (ou *scanner*) lê o arquivo-fonte caractere por caractere e agrupa sequências em **tokens** — as unidades mínimas de significado da linguagem (palavras-chave, identificadores, constantes, operadores).

O Flex gera automaticamente um autômato finito a partir das expressões regulares escritas no arquivo `.l`. Cada padrão está associado a uma ação em C que retorna o token correspondente ao parser.

**O que faz neste projeto:**
- Reconhece palavras-chave: `principal`, `int`, `car`, `leia`, `escreva`, `novalinha`, `se`, `entao`, `senao`, `fimse`, `enquanto`
- Reconhece identificadores (`[a-zA-Z_][a-zA-Z0-9_]*`), constantes inteiras e de caractere (`'a'`, `'\n'`)
- Ignora espaços, tabulações, quebras de linha e comentários `/* ... */`
- Reporta caracteres inválidos e comentários não fechados com mensagem de erro

---

### `src/g-v1.y` — Parser (Bison)

**Conceito:** O analisador sintático (*parser*) consome a sequência de tokens e verifica se ela obedece à **gramática** da linguagem. Bison gera um parser LALR(1) a partir de regras gramaticais escritas em BNF. Cada regra pode ter uma **ação semântica** em C — aqui usada para construir a AST nodo a nodo.

**O que faz neste projeto:**
- Define a gramática completa de G-V1 (programa, blocos, declarações, comandos, expressões)
- Estabelece a precedência e associatividade dos operadores (`||` < `&&` < relacionais < aditivos < multiplicativos < unários)
- A função `main()` está aqui: abre o arquivo, chama `yyparse()`, depois encadeia análise semântica e geração de código

---

### `include/ast.h` + `src/ast.c` — Árvore Sintática Abstrata

**Conceito:** A **AST** (*Abstract Syntax Tree*) é a representação interna do programa após a análise sintática. Diferente da árvore de derivação, ela omite detalhes sintáticos irrelevantes (parênteses, ponto-e-vírgula) e mantém apenas a estrutura semântica do programa. Cada nó representa uma construção — operação, comando, declaração — e aponta para seus sub-nós (filhos).

**O que faz neste projeto:**
- Define um único `struct No` com três ponteiros filhos (`esq`, `dir`, `extra`), tipo (`TipoNo`), linha e valor léxico
- Há 32 tipos de nó: `NO_PROGRAMA`, `NO_BLOCO`, `NO_OP_SOMA`, `NO_IDENT`, `NO_CMD_SE`, etc.
- Funções: `criar_no()`, `criar_no3()`, `criar_no_folha()`, `liberar_ast()`, `imprimir_ast()` (debug)

---

### `include/symtab.h` + `src/symtab.c` — Tabela de Símbolos

**Conceito:** A **tabela de símbolos** associa cada identificador declarado às suas propriedades (tipo, posição em memória). Como a linguagem tem **escopos aninhados** (blocos dentro de blocos), a tabela é implementada como uma **pilha de escopos**: ao entrar em um bloco com declarações, um novo escopo é empurrado; ao sair, é desempilhado e suas variáveis são esquecidas. A busca por um identificador percorre a pilha do topo à base — isso implementa **shadowing** (variável interna esconde a externa de mesmo nome).

**O que faz neste projeto:**
- `push_escopo()` / `pop_escopo()` — gerenciam a pilha ao entrar/sair de blocos
- `inserir()` — adiciona variável ao escopo atual; erro se já declarada no mesmo escopo
- `buscar()` — pesquisa do escopo mais interno ao mais externo; retorna `EntradaTabela*` com tipo e offset

---

### `include/semantico.h` + `src/semantico.c` — Análise Semântica

**Conceito:** A análise semântica percorre a AST e verifica regras que a gramática não consegue expressar, como **uso de variável não declarada** e **compatibilidade de tipos**. Ela usa a tabela de símbolos para rastrear quais variáveis estão visíveis em cada ponto do programa.

**O que faz neste projeto:**
- Percorre a AST recursivamente visitando blocos, comandos e expressões
- Insere declarações na tabela de símbolos ao entrar em cada bloco
- Verifica: variável declarada antes do uso, tipos compatíveis em atribuições, operandos corretos para cada operador (`+`, `-`, `*`, `/` exigem `int`; operadores lógicos exigem `int`; relacionais exigem mesmo tipo nos dois lados)
- Erros são fatais: imprime `ERRO: <mensagem> linha <N>` e encerra

---

### `include/codegen.h` + `src/codegen.c` — Geração de Código MIPS

**Conceito:** A geração de código transforma a AST verificada em instruções de máquina (aqui, assembly MIPS). Cada construção da linguagem é mapeada para um padrão fixo de instruções. Expressões são avaliadas deixando o resultado em registrador; estruturas de controle usam labels e desvios condicionais.

**O que faz neste projeto:**
- **Seção `.data`:** coleta todas as string literals do programa antes de emitir código
- **Variáveis locais:** alocadas na pilha via `$fp` (frame pointer); cada variável recebe um offset único
- **Expressões:** resultado sempre em `$t0`; operações binárias empilham o lado esquerdo, avaliam o direito em `$t0`, desempilham para `$t1`, depois operam
- **I/O:** usa syscalls MIPS (`li $v0, 1` = print\_int, `li $v0, 5` = read\_int, etc.)
- **Controle:** `se`/`enquanto` geram labels únicas com contador global

---

## Exemplo de programa G-V1

```
principal
{
    fatorial, i, n: int;
}
{
    n = -1;
    enquanto (n <= 0)
        {
            escreva "Digite um numero inteiro nao negativo";
            novalinha;
            leia n;
        }
    fatorial = n;
    i = n - 1;
    enquanto (i > 1)
        {
            fatorial = fatorial * i;
            i = i - 1;
        }
    escreva "O fatorial de ";
    escreva n;
    escreva " e: ";
    escreva fatorial;
    novalinha;
}
```
