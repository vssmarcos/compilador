CC      = gcc
CFLAGS  = -Wall -g -I include/ -I build/

SRCDIR  = src
BLDDIR  = build

# ─── Objetos G-V1 ───────────────────────────────────────────────
OBJS_V1 = $(BLDDIR)/lex.yy.o \
           $(BLDDIR)/g-v1.tab.o \
           $(BLDDIR)/ast.o \
           $(BLDDIR)/symtab.o \
           $(BLDDIR)/semantico.o \
           $(BLDDIR)/codegen.o

# ─── Objetos G-V2 ───────────────────────────────────────────────
OBJS_V2 = $(BLDDIR)/lex-v2.yy.o \
           $(BLDDIR)/g-v2.tab.o \
           $(BLDDIR)/ast.o \
           $(BLDDIR)/symtab.o \
           $(BLDDIR)/semantico.o \
           $(BLDDIR)/codegen.o

all: g-v1 g-v2

# ═══════════════════════════════════════════════════════════════
# G-V1
# ═══════════════════════════════════════════════════════════════
g-v1: $(OBJS_V1)
	$(CC) $(CFLAGS) -o g-v1 $(OBJS_V1)

$(BLDDIR)/g-v1.tab.c $(BLDDIR)/g-v1.tab.h: $(SRCDIR)/g-v1.y
	bison -d -o $(BLDDIR)/g-v1.tab.c $(SRCDIR)/g-v1.y

$(BLDDIR)/lex.yy.c: $(SRCDIR)/g-v1.l $(BLDDIR)/g-v1.tab.h
	flex -o $(BLDDIR)/lex.yy.c $(SRCDIR)/g-v1.l

$(BLDDIR)/lex.yy.o: $(BLDDIR)/lex.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/g-v1.tab.o: $(BLDDIR)/g-v1.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

# ═══════════════════════════════════════════════════════════════
# G-V2
# ═══════════════════════════════════════════════════════════════
g-v2: $(OBJS_V2)
	$(CC) $(CFLAGS) -o g-v2 $(OBJS_V2)

$(BLDDIR)/g-v2.tab.c $(BLDDIR)/g-v2.tab.h: $(SRCDIR)/g-v2.y
	bison -d -o $(BLDDIR)/g-v2.tab.c $(SRCDIR)/g-v2.y

$(BLDDIR)/lex-v2.yy.c: $(SRCDIR)/g-v2.l $(BLDDIR)/g-v2.tab.h
	flex -o $(BLDDIR)/lex-v2.yy.c $(SRCDIR)/g-v2.l

$(BLDDIR)/lex-v2.yy.o: $(BLDDIR)/lex-v2.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/g-v2.tab.o: $(BLDDIR)/g-v2.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

# ═══════════════════════════════════════════════════════════════
# Objetos compartilhados (ast, symtab, semantico, codegen)
# Compilados UMA vez, reutilizados por g-v1 e g-v2
# ═══════════════════════════════════════════════════════════════
$(BLDDIR)/ast.o: $(SRCDIR)/ast.c include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/symtab.o: $(SRCDIR)/symtab.c include/symtab.h include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/semantico.o: $(SRCDIR)/semantico.c include/semantico.h include/symtab.h include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/codegen.o: $(SRCDIR)/codegen.c include/codegen.h include/symtab.h include/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

# ─── Limpeza ────────────────────────────────────────────────────
clean:
	rm -f g-v1 g-v2 $(BLDDIR)/*.c $(BLDDIR)/*.h $(BLDDIR)/*.o

.PHONY: all clean
