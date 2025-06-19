#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "tree.h"

// Type de variables 
typedef enum {
    Int,
    Char,
    Void,
} type;

// structure pour les variables
typedef struct {
    char *id;          // nom de la variable
    bool static_var;   // est-ce une variable statique ?
    type type;         // Variable type (Int, Char, Void)
    int adresse;       // Adresse de la variable (pour la gestion mémoire)
} Variable;

// Liste chaînée pour les variables globales
typedef struct symbol_table_variables_ {
    Variable var;                  // Variable donnée
    struct symbol_table_variables_ *next;  // Prochain élément de la liste
} symbol_table_variables;

// Struct for function definitions
typedef struct {
    Variable id;                      // Fonction ID 
    symbol_table_variables *defvars;  // Local variables dans la fonction
    symbol_table_variables *args;     // Arguments de la fonction
} fonction;

// Liste chaînée pour les fonctions
typedef struct symbol_table_fonctions_ {
    fonction func;                    // Function data
    struct symbol_table_fonctions_ *next;  // prochain élément de la liste
} symbol_table_fonctions;

// structure pour la table des symboles
typedef struct {
    symbol_table_variables *variables_global;  // Liste des variables globales
    symbol_table_fonctions *fonctions;        // Liste des fonctions
} symbol_table;

// Function declarations
symbol_table_variables * create_variable_global();
symbol_table_fonctions * create_fonction();
symbol_table_variables *remplir_var_global(Node *tree);  // remplir la table des variables globales à partir de l'AST
symbol_table_fonctions *remplir_fonction(Node *tree);   // remplir la table des fonctions à partir de l'AST
symbol_table fill_symbol_table(Node *tree,int print);             // remplir la table des symboles à partir de l'AST
symbol_table_fonctions* chercher_fonction(symbol_table table, const char *nom);
void ajouter_fonctions_predefinies(symbol_table *table) ;
void printsymboletable(symbol_table table);  // Print 

#endif // SYMBOL_TABLE_H
