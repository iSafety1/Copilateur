#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantics.h"
#include "symbol_table.h"



typedef struct {
    int counter;               // Compteur pour créer des étiquettes uniques
    char true_label[64];       // Étiquette pour la condition vraie
    char false_label[64];      // Étiquette pour la condition fausse
    char end_label[64];        // Étiquette pour marquer la fin
} labels_info;


void ecrire_code_nasm_main(FILE *fichier);
void ecrire_section_data(FILE *fichier, symbol_table *table);
void verifier_et_ecrire_main(symbol_table table, FILE *fichier);
void generer_appel_fonction(Node *node, FILE *f, bool push_result, fonction *current_func);
Node *get_corps_main();
void generer_bool_to_arithm(Node *expr, FILE *f, fonction *current_func);
void generer_boolean_expression(Node *expr, FILE *f, fonction *current_func, labels_info *labels);
void generer_expression(Node *expr, FILE *f, fonction *current_func,symbol_table *table);
void generer_instr(Node *instr, FILE *f, fonction *current_func, symbol_table *table);
void generer_suite_instr(Node *corps, FILE *f, fonction *current_func, symbol_table *table);
int compter_arguments(symbol_table_variables *args);
symbol_table_fonctions* chercher_fonction(symbol_table table, const char *nom);
void generer_fonctions_utiles(Node *tree, FILE *fichier, symbol_table table);
void init_labels(labels_info *labels);
void generer_comparison(Node *expr, FILE *f, fonction *current_func, labels_info *labels);
void generer_expression_bool(Node *expr, FILE *f, fonction *current_func, labels_info *labels);
void generer_if_else(Node *node, FILE *f, fonction *current_func, symbol_table *table);
void ecrire_fonctions_io(FILE *fichier);
int compter_vars_locales(fonction *func);
void genere_code_nasm(symbol_table table, FILE *fichier);

#endif // CODEGEN_H