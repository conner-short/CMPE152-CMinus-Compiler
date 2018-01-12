OBJDIR = .obj
CFLAGS = -g -Wall
CXXFLAGS = -g -Wall

LIBS = -ly -lfl
OBJ = $(addprefix $(OBJDIR)/,lex.yy.o y.tab.o scope.o support.o asm_gen.o)
BIN = lab4

all: $(BIN)
.PHONY: all

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LIBS)

$(OBJ): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -Iinclude/ $< -o $@

$(OBJDIR)/scope.o: include/symbol.h include/support.h
$(OBJDIR)/asm_gen.o: include/nt.h include/symbol.h
$(OBJDIR)/support.o: include/nt.h include/symbol.h

include/y.tab.h: src/y.tab.c

src/lex.yy.c: src/cminus.l
	lex $<
	mv lex.yy.c src/

src/y.tab.c: src/cminus.y
	yacc --debug -d $<
	mv y.tab.c src/
	mv y.tab.h include/

src/cminus.l: include/y.tab.h include/symbol.h include/nt.h
src/cminus.y: include/asm_gen.h include/nt.h include/symbol.h include/support.h

.PHONY: clean
clean:
	-rm $(BIN) $(OBJ) src/lex.yy.c src/y.tab.c include/y.tab.h
	-rmdir $(OBJDIR)
