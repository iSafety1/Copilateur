#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "symbol_table.h"

// Check the semantic validity of the program
int verifier_semantique(symbol_table table, Node *arbre);


type eval_type_function_call(Node *node, symbol_table *table);
type eval_type(Node *node, symbol_table *table, fonction *fonction_courante);
// Helper functions for semantic validation
int verifier_variables_globales_uniques(symbol_table_variables *vars);
int verifier_conflits_noms(symbol_table table);
int verifier_variables_fonction(fonction f);
int verifier_main(symbol_table_fonctions *fonctions);
int verifier_return(Node *ret_node, fonction *func, symbol_table *table);
int verifier_condition(Node *condition, symbol_table *table, fonction *fonction_courante);
type verifier_variable_utilisee(Node *ident_node, fonction *func, symbol_table *table);
int verifier_variables_locales_uniques(fonction f);
int verifier_expressions(Node *node, symbol_table *table, fonction *fonction_courante);
int verifier_static_initialisation(Node *var_node);
int verifier_static_declarations(Node *declVars);
int verifier_appel_fonctions(Node *node, symbol_table *table, fonction *fonction_courante);
void verifier_variables_utilisees(symbol_table table, Node *arbre) ;
int verifier_return_recursif(Node *node, fonction *func, symbol_table *table, int *has_return);
int verifier_appel_fonctions(Node *node, symbol_table *table, fonction *fonction_courante);

int verifier_condition(Node *condition, symbol_table *table, fonction *fonction_courante);
int verifier_static_initialisation(Node *var_node);
int verifier_static_declarations(Node *declVars);
int verifier_variables_locales_uniques(fonction f);
int verifier_expressions(Node *node, symbol_table *table, fonction *fonction_courante);
void chercher(Node *n);

#endif // SEMANTICS_H
