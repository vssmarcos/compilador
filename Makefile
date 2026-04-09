CC      = gcc
CFLAGS  = -Wall -g -I include/ -I build/

SRCDIR  = src
BLDDIR  = build

OBJS    = $(BLDDIR)/lex.yy.o \
          $(BLDDIR)/g-v1.tab.o \
          $(BLDDIR)/ast.o \
          $(BLDDIR)/symtab.o \
          $(BLDDIR)/semantico.o \
          $(BLDDIR)/codegen.o

all: g-v1

g-v1: $(OBJS)
	$(CC) $(CFLAGS) -o g-v1 $(OBJS)

# --- Geração do lexer e parser ---

$(BLDDIR)/g-v1.tab.c $(BLDDIR)/g-v1.tab.h: $(SRCDIR)/g-v1.y
	bison -d -o $(BLDDIR)/g-v1.tab.c $(SRCDIR)/g-v1.y

$(BLDDIR)/lex.yy.c: $(SRCDIR)/g-v1.l $(BLDDIR)/g-v1.tab.h
	flex -o $(BLDDIR)/lex.yy.c $(SRCDIR)/g-v1.l

# --- Compilação dos objetos ---

$(BLDDIR)/lex.yy.o: $(BLDDIR)/lex.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/g-v1.tab.o: $(BLDDIR)/g-v1.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/ast.o: $(SRCDIR)/ast.c include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/symtab.o: $(SRCDIR)/symtab.c include/symtab.h include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/semantico.o: $(SRCDIR)/semantico.c include/semantico.h include/symtab.h include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/codegen.o: $(SRCDIR)/codegen.c include/codegen.h include/symtab.h include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Limpeza ---

clean:
	rm -f g-v1 $(BLDDIR)/*.c $(BLDDIR)/*.h $(BLDDIR)/*.o

.PHONY: all clean
