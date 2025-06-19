CC = gcc
CFLAGS = -Wall -g -Iobj -Isrc
LDFLAGS = -Wall -ll
PARSER = parser
LEXER = lexer
EXEC = tpcc

# Répertoires
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Fichiers sources
SRCS = \
    $(SRC_DIR)/codegen.c \
    $(SRC_DIR)/semantics.c \
    $(SRC_DIR)/symbol_table.c \
    $(SRC_DIR)/tree.c \
    $(SRC_DIR)/$(PARSER).y \
    $(SRC_DIR)/$(LEXER).l

# Fichiers objets
OBJS = \
    $(OBJ_DIR)/codegen.o \
    $(OBJ_DIR)/semantics.o \
    $(OBJ_DIR)/symbol_table.o \
    $(OBJ_DIR)/tree.o \
    $(OBJ_DIR)/$(PARSER).tab.o \
    $(OBJ_DIR)/lex.yy.o

# Cible finale
all: $(BIN_DIR)/$(EXEC)

$(BIN_DIR)/$(EXEC): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation des fichiers .o pour chaque module
$(OBJ_DIR)/codegen.o: $(SRC_DIR)/codegen.c $(SRC_DIR)/codegen.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/semantics.o: $(SRC_DIR)/semantics.c $(SRC_DIR)/semantics.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/symbol_table.o: $(SRC_DIR)/symbol_table.c $(SRC_DIR)/symbol_table.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/tree.o: $(SRC_DIR)/tree.c $(SRC_DIR)/tree.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Règle pour parser et lexer
$(OBJ_DIR)/$(PARSER).tab.o: $(OBJ_DIR)/$(PARSER).tab.c $(OBJ_DIR)/$(PARSER).tab.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/lex.yy.o: $(OBJ_DIR)/lex.yy.c $(OBJ_DIR)/$(PARSER).tab.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Génération des fichiers .c et .h pour le parser et le lexer
$(OBJ_DIR)/$(PARSER).tab.c $(OBJ_DIR)/$(PARSER).tab.h: $(SRC_DIR)/$(PARSER).y $(SRC_DIR)/tree.h
	mkdir -p $(OBJ_DIR)
	bison -d -o $(OBJ_DIR)/$(PARSER).tab.c $(SRC_DIR)/$(PARSER).y

$(OBJ_DIR)/lex.yy.c: $(SRC_DIR)/$(LEXER).l $(OBJ_DIR)/$(PARSER).tab.h
	mkdir -p $(OBJ_DIR)
	flex -o $@ $(SRC_DIR)/$(LEXER).l

# Nettoyage des fichiers objets et binaires
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*

# Exécution des tests
test: $(BIN_DIR)/$(EXEC)
	./run_tests.sh
