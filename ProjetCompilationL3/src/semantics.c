#include "semantics.h"
// Fonction modifiée pour la vérification des variables globales uniques
int verifier_variables_globales_uniques(symbol_table_variables *vars) {
    int erreur = 0;
    for (symbol_table_variables *v1 = vars; v1 != NULL; v1 = v1->next) {
        for (symbol_table_variables *v2 = v1->next; v2 != NULL; v2 = v2->next) {
            if (v1->var.id && v2->var.id && strcmp(v1->var.id, v2->var.id) == 0) {
                fprintf(stderr, "Erreur: variable globale '%s' déclarée plusieurs fois\n", v1->var.id);
                erreur = -1;
            }
        }
    }
    return erreur;
}

int verifier_conflits_noms(symbol_table table) {
  for (symbol_table_fonctions *f = table.fonctions; f != NULL; f = f->next) {
      char *fname = f->func.id.id;
      // Vérifie si nom de fonction = nom variable globale
      for (symbol_table_variables *v = table.variables_global; v != NULL; v = v->next) {
          if (strcmp(fname, v->var.id) == 0) {
              fprintf(stderr, "Erreur : nom de fonction '%s' en conflit avec une variable globale\n", fname);
              return -1;
          }
      }
  }
  return 0;
}


int verifier_variables_fonction(fonction f) {
    // Vérifier les paramètres pour les duplications
    if (f.args) {
        for (symbol_table_variables *p1 = f.args; p1 != NULL; p1 = p1->next) {
            if (!p1->var.id) continue;
            
            for (symbol_table_variables *p2 = p1->next; p2 != NULL; p2 = p2->next) {
                if (!p2->var.id) continue;
                
                if (strcmp(p1->var.id, p2->var.id) == 0) {
                    fprintf(stderr, "Erreur: paramètre '%s' déclaré plusieurs fois\n", p1->var.id);
                    return -1;
                }
            }
        }
    }
    
    // Vérifier les variables locales pour les duplications
    if (verifier_variables_locales_uniques(f) == -1)
        return -1;
        
    // Vérifier les conflits entre paramètres et variables locales
    if (f.defvars && f.args) {
        for (symbol_table_variables *v = f.defvars; v != NULL; v = v->next) {
            if (!v->var.id) continue;
            
            for (symbol_table_variables *p = f.args; p != NULL; p = p->next) {
                if (!p->var.id) continue;
                
                if (strcmp(v->var.id, p->var.id) == 0) {
                    fprintf(stderr, "Erreur: variable locale '%s' en conflit avec un paramètre\n", v->var.id);
                    return -1;
                }
            }
        }
    }
    
    return 0;
}

type verifier_variable_utilisee(Node *ident_node, fonction *func, symbol_table *table) {
    if (!ident_node || ident_node->label != IDENT_ef || !func) return -1;
    
    char *nom = ident_node->u.ident;

    // Fonctions prédéfinies
    if (strcmp(nom, "getchar") == 0 || strcmp(nom, "getint") == 0 || 
        strcmp(nom, "putchar") == 0 || strcmp(nom, "putint") == 0) {
        return Int; // Ces fonctions sont toujours disponibles
    }

    // Recherche dans les variables locales
    for (symbol_table_variables *var = func->defvars; var; var = var->next) {
        if (strcmp(var->var.id, nom) == 0) {
            return var->var.type;
        }
    }
    
    // Recherche dans les paramètres
    for (symbol_table_variables *arg = func->args; arg; arg = arg->next) {
        if (strcmp(arg->var.id, nom) == 0) {
            return arg->var.type;
        }
    }
    
    // Recherche dans les variables globales
    for (symbol_table_variables *glob = table->variables_global; glob; glob = glob->next) {
        if (strcmp(glob->var.id, nom) == 0) {
            return glob->var.type;
        }
    }

    // Recherche comme nom de fonction
    for (symbol_table_fonctions *f = table->fonctions; f; f = f->next) {
        if (strcmp(f->func.id.id, nom) == 0) {
            // C'est une fonction, mais on ne devrait pas y accéder comme à une variable
            fprintf(stderr, "Erreur ligne %d : '%s' est une fonction, pas une variable\n", 
                    ident_node->lineno, nom);
            return -1;
        }
    }

    fprintf(stderr, "Erreur ligne %d : variable '%s' utilisée sans être déclarée\n", 
            ident_node->lineno, nom);
    return -1;
}


int verifier_main(symbol_table_fonctions *fonctions) {
  for (; fonctions; fonctions = fonctions->next) {
      if (strcmp(fonctions->func.id.id, "main") == 0) {
          if (fonctions->func.id.type != Int) {
              fprintf(stderr, "Erreur : la fonction main doit être de type int\n");
              return -1;
          }
          return 0; // main existe et est correcte
      }
  }
  fprintf(stderr, "Erreur : fonction main absente\n");
  return -1;
}


// Fonction pour évaluer le type d'une expression (évaluation récursive)
// Renvoie le type de retour de la fonction appelée
type eval_type_function_call(Node *node, symbol_table *table) {
  if (!node || node->label != FunctionCall) return -1;

  char *fname = node->firstChild->u.ident;

  if (strcmp(fname, "getchar") == 0) return Char;
  if (strcmp(fname, "getint") == 0) return Int;
  if (strcmp(fname, "putchar") == 0) return Void;
  if (strcmp(fname, "putint") == 0) return Void;

  for (symbol_table_fonctions *f = table->fonctions; f; f = f->next) {
      if (strcmp(f->func.id.id, fname) == 0) {
          return f->func.id.type;
      }
  }

  return -1;
}

type eval_type(Node *node, symbol_table *table, fonction *fonction_courante) {
    if (!node) return -1;

    switch (node->label) {
        case DIGIT_ef:
            return Int;

        case CHARACTER_ef:
            return Char;

        case IDENT_ef: {
            // Variables locales
            if (fonction_courante) {
                for (symbol_table_variables *var = fonction_courante->defvars; var; var = var->next) {
                    if (strcmp(var->var.id, node->u.ident) == 0) {
                        return var->var.type;
                    }
                }
                for (symbol_table_variables *arg = fonction_courante->args; arg; arg = arg->next) {
                    if (strcmp(arg->var.id, node->u.ident) == 0) {
                        return arg->var.type;
                    }
                }
            }

            // Variables globales
            for (symbol_table_variables *var = table->variables_global; var; var = var->next) {
                if (strcmp(var->var.id, node->u.ident) == 0) {
                    return var->var.type;
                }
            }

            // Fonctions prédéfinies - important pour les expressions contenant des appels de fonction
            if (strcmp(node->u.ident, "getchar") == 0) return Char;
            if (strcmp(node->u.ident, "getint") == 0) return Int;
            if (strcmp(node->u.ident, "putchar") == 0 || strcmp(node->u.ident, "putint") == 0) return Void;

            fprintf(stderr, "Erreur ligne %d : variable '%s' non déclarée\n", node->lineno, node->u.ident);
            return -1;
        }

        case ADDSUB_ef:
        case DIVSTAR_ef: {
            type t1 = eval_type(node->firstChild, table, fonction_courante);
            type t2 = eval_type(node->firstChild->nextSibling, table, fonction_courante);
            
            if (t1 == Void || t2 == Void) {
                fprintf(stderr, "Erreur ligne %d : opération arithmétique avec expression void\n", node->lineno);
                return -1;
            }
            
            if (t1 == -1 || t2 == -1) return -1;
            return Int;
        }

        case EQ_ef:
        case ORDER_ef:
        case AND_ef:
        case OR_ef:
        case NOT_ef: {
            // Vérifiez que les opérandes ne sont pas void
            if (node->firstChild) {
                type t1 = eval_type(node->firstChild, table, fonction_courante);
                if (t1 == Void) {
                    fprintf(stderr, "Erreur ligne %d : expression booléenne avec type void\n", node->lineno);
                    return -1;
                }
            }
            
            if (node->firstChild && node->firstChild->nextSibling) {
                type t2 = eval_type(node->firstChild->nextSibling, table, fonction_courante);
                if (t2 == Void) {
                    fprintf(stderr, "Erreur ligne %d : expression booléenne avec type void\n", node->lineno);
                    return -1;
                }
            }
            
            return Int;
        }

        case FunctionCall: {
            char *fname = node->u.ident;
            
            if (strcmp(fname, "getchar") == 0) return Char;
            if (strcmp(fname, "getint") == 0) return Int;
            if (strcmp(fname, "putchar") == 0 || strcmp(fname, "putint") == 0) return Void;
            
            for (symbol_table_fonctions *f = table->fonctions; f; f = f->next) {
                if (strcmp(f->func.id.id, fname) == 0) {
                    return f->func.id.type;
                }
            }
            
            fprintf(stderr, "Erreur ligne %d : fonction '%s' non déclarée\n", node->lineno, fname);
            return -1;
        }

        default:
            return -1;
    }
}





int verifier_return(Node *ret_node, fonction *func, symbol_table *table) {
    if (!ret_node || !func) return 0;
    
    Node *exp = ret_node->firstChild;

    if (func->id.type == Void) {
        if (exp != NULL) {
            fprintf(stderr, "Erreur ligne %d : return avec valeur dans une fonction void\n", ret_node->lineno);
            return -1;
        }
        return 0;
    }
    
    // Pour les fonctions non-void
    if (exp == NULL) {
        fprintf(stderr, "Erreur ligne %d : return vide dans une fonction non-void\n", ret_node->lineno);
        return -1;
    }
    
    // Accepter explicitement les expressions arithmétiques dans les return
    if (exp->label == ADDSUB_ef || exp->label == DIVSTAR_ef) {
        // Pour les expressions arithmétiques, évaluer le type résultant
        type t_left = eval_type(exp->firstChild, table, func);
        type t_right = eval_type(exp->firstChild->nextSibling, table, func);
        
        // Si les deux opérandes sont valides (pas Void ni erreur)
        if (t_left != Void && t_right != Void && t_left != -1 && t_right != -1) {
            // Les expressions arithmétiques produisent un Int en C
            type result_type = Int;
            
            if (result_type != func->id.type) {
                if (func->id.type == Int && (t_left == Char || t_right == Char)) {
                    // Char dans Int est ok sans avertissement
                    return 0;
                }
                if (func->id.type == Char && result_type == Int) {
                    fprintf(stderr, "Avertissement ligne %d : return int dans fonction char\n", ret_node->lineno);
                    return 0;
                }
            }
            return 0;  // L'expression est valide
        }
    }
    
    // Pour les autres types d'expressions, utiliser l'évaluation standard
    type t = eval_type(exp, table, func);
    if (t == -1) return -1;  // Erreur déjà signalée dans eval_type

    if (t != func->id.type) {
        if (func->id.type == Int && t == Char) {
            // Char dans Int est ok sans avertissement
            return 0;
        }
        if (func->id.type == Char && t == Int) {
            fprintf(stderr, "Avertissement ligne %d : return int dans fonction char\n", ret_node->lineno);
            return 0;
        }
        fprintf(stderr, "Erreur ligne %d : type de retour incompatible (attendu %d, obtenu %d)\n",
                ret_node->lineno, func->id.type, t);
        return -1;
    }
    
    return 0;
}


int verifier_appel_fonctions(Node *node, symbol_table *table, fonction *fonction_courante) {
    if (!node || node->label != FunctionCall) return 0;

    char *fname = node->u.ident;
    Node *argsNode = node->firstChild;

    // Vérifier si la fonction existe dans la table des symboles
    symbol_table_fonctions *func = NULL;
    for (symbol_table_fonctions *f = table->fonctions; f; f = f->next) {
        if (strcmp(f->func.id.id, fname) == 0) {
            func = f;
            break;
        }
    }

    // Vérifier si une variable locale ombrage la fonction
    for (symbol_table_variables *var = fonction_courante->defvars; var; var = var->next) {
        if (strcmp(var->var.id, fname) == 0) {
        fprintf(stderr, "Erreur ligne %d : '%s' est une variable locale et ne peut pas être appelée comme une fonction\n", 
                node->lineno, fname);
        return -1;
    }
    }

    // Si la fonction n'existe pas, générer une erreur sémantique
    if (!func) {
        fprintf(stderr, "Erreur ligne %d : fonction '%s' non déclarée\n", node->lineno, fname);
        return -1;
    }

    // Vérifier les arguments pour les fonctions prédéfinies
    if (strcmp(fname, "putint") == 0 || strcmp(fname, "putchar") == 0) {
        if (!argsNode) {
            fprintf(stderr, "Erreur ligne %d : la fonction '%s' attend un argument\n", node->lineno, fname);
            return -1;
        }
        type t_arg = eval_type(argsNode, table, fonction_courante);
        if (t_arg == Void || t_arg == -1) {
            fprintf(stderr, "Erreur ligne %d : argument invalide pour la fonction '%s'\n", node->lineno, fname);
            return -1;
        }
        return 0; // Pas besoin de vérifier plus pour ces fonctions
    }

    if (strcmp(fname, "getint") == 0 || strcmp(fname, "getchar") == 0) {
        if (argsNode) {
            fprintf(stderr, "Erreur ligne %d : la fonction '%s' ne prend pas d'arguments\n", node->lineno, fname);
            return -1;
        }
        return 0; // Pas besoin de vérifier plus pour ces fonctions
    }

    // Vérifier le nombre d'arguments fournis
    int arg_count = 0;
    for (Node *arg = argsNode; arg; arg = arg->nextSibling) {
        arg_count++;
    }

    // Compter les paramètres attendus
    int param_count = 0;
    for (symbol_table_variables *p = func->func.args; p; p = p->next) {
        param_count++;
    }

    // Vérifier le nombre d'arguments
    if (arg_count != param_count) {
        fprintf(stderr, "Erreur ligne %d : nombre d'arguments incorrect pour la fonction '%s' (attendu %d, obtenu %d)\n", 
                node->lineno, fname, param_count, arg_count);
        return -1;
    }

    return 0;
}

// Vérification des expressions conditionnelles
int verifier_condition(Node *condition, symbol_table *table, fonction *fonction_courante) {
    if (!condition) return -1;
    
    // Vérifier le type de l'expression de condition
    type t_cond = eval_type(condition, table, fonction_courante);
    
    // Un void ne peut pas être utilisé en tant que condition
    if (t_cond == Void) {
        fprintf(stderr, "Erreur ligne %d : une expression void ne peut pas être utilisée comme condition\n", 
                condition->lineno);
        return -1;
    }
    
    // En C, toute condition non-void est acceptable
    return 0;
}

int verifier_static_initialisation(Node *var_node) {
    if (!var_node) return 0;
    
    if (var_node->label == static_ef) {
        Node *type_node = var_node->firstChild;
        if (!type_node) return 0;
        
        for (Node *id_node = type_node->firstChild; id_node; id_node = id_node->nextSibling) {
            // Vérifie si on a une initialisation après l'identificateur
            if (id_node->label == IDENT_ef && id_node->nextSibling && id_node->nextSibling->label != IDENT_ef) {
                fprintf(stderr, "Erreur ligne %d : initialisation interdite pour la variable static '%s'\n", 
                        var_node->lineno, id_node->u.ident);
                return -1;
            }
        }
    }
    
    return 0;
}

// Fonction pour vérifier les déclarations de variables statiques avec initialisation
int verifier_static_declarations(Node *declVars) {
    if (!declVars || declVars->label != DeclVars)
        return 0;
    
    for (Node *var_node = declVars->firstChild; var_node; var_node = var_node->nextSibling) {
        if (var_node->label == static_ef) {
            // C'est une déclaration static
            Node *type_node = var_node->firstChild;
            if (!type_node) continue;
            
            for (Node *id_node = type_node->firstChild; id_node; id_node = id_node->nextSibling) {
                if (id_node->label == IDENT_ef && id_node->nextSibling && 
                   (id_node->nextSibling->label != IDENT_ef && id_node->nextSibling->label != TYPE_ef)) {
                    fprintf(stderr, "Erreur ligne %d : initialisation de variable static '%s' non autorisée\n",
                            var_node->lineno, id_node->u.ident);
                    return -1;
                }
            }
        }
    }
    
    return 0;
}

int verifier_variables_locales_uniques(fonction f) {
    // Vérifier que les variables locales n'ont pas de noms en double
    for (symbol_table_variables *v1 = f.defvars; v1 != NULL; v1 = v1->next) {
        for (symbol_table_variables *v2 = v1->next; v2 != NULL; v2 = v2->next) {
            if (v1->var.id && v2->var.id && strcmp(v1->var.id, v2->var.id) == 0) {
                fprintf(stderr, "Erreur : variable locale '%s' déclarée plusieurs fois\n", v1->var.id);
                return -1;
            }
        }
    }
    return 0;
}



int verifier_expressions(Node *node, symbol_table *table, fonction *fonction_courante) {
    if (!node) return 0;

    if (node->label == ASSIGN_ef) {
        Node *lhs = node->firstChild;
        Node *rhs = lhs ? lhs->nextSibling : NULL;
        
        if (!lhs || !rhs) {
            fprintf(stderr, "Erreur ligne %d : assignation malformée\n", node->lineno);
            return -1;
        }
        
        if (lhs->label != IDENT_ef) {
            fprintf(stderr, "Erreur ligne %d : membre gauche d'assignation doit être un identifiant\n", node->lineno);
            return -1;
        }
        
        type tL = verifier_variable_utilisee(lhs, fonction_courante, table);
        if (tL == -1) return -1;
        
        // Vérifier le type d'expression du côté droit
        if (rhs->label == FunctionCall) {
            // Vérifier d'abord que la fonction existe
            if (verifier_appel_fonctions(rhs, table, fonction_courante) == -1)
                return -1;
            
            // Puis vérifier que son type de retour est compatible
            char *fname = rhs->u.ident;
            
            // Fonctions spéciales - ERREUR si on essaie d'assigner le résultat
            if (strcmp(fname, "putchar") == 0 || strcmp(fname, "putint") == 0) {
                fprintf(stderr, "Erreur ligne %d : fonction '%s' a un type de retour void\n", node->lineno, fname);
                return -1;
            }
            
            // Recherche la fonction dans la table
            for (symbol_table_fonctions *f = table->fonctions; f; f = f->next) {
                if (strcmp(f->func.id.id, fname) == 0) {
                    if (f->func.id.type == Void) {
                        fprintf(stderr, "Erreur ligne %d : assignation du résultat d'une fonction void\n", node->lineno);
                        return -1;
                    }
                    
                    // Vérifier la compatibilité des types
                    if (f->func.id.type != tL) {
                        if (tL == Char && f->func.id.type == Int) {
                            fprintf(stderr, "Avertissement ligne %d : assignation int -> char\n", node->lineno);
                        } else if (!(tL == Int && f->func.id.type == Char)) {
                            fprintf(stderr, "Erreur ligne %d : types incompatibles dans l'assignation\n", node->lineno);
                            return -1;
                        }
                    }
                    break;
                }
            }
        } else {
            // Expression standard
            type tR = eval_type(rhs, table, fonction_courante);
            if (tR == -1) return -1;
            
            if (tL != tR) {
                if (tL == Char && tR == Int) {
                    fprintf(stderr, "Avertissement ligne %d : assignation int -> char\n", node->lineno);
                } else if (!(tL == Int && tR == Char)) {
                    fprintf(stderr, "Erreur ligne %d : types incompatibles dans l'assignation\n", node->lineno);
                    return -1;
                }
            }
        }
    } 
    // Vérification des expressions conditionnelles
    else if (node->label == if_ef || node->label == while_ef) {
        Node *condition = node->firstChild;
        
        // Vérifier que la condition n'est pas void
        if (condition) {
            if (verifier_condition(condition, table, fonction_courante) == -1)
                return -1;
        }
    }
    else if (node->label == FunctionCall) {
        // Vérifier l'appel de fonction
        if (verifier_appel_fonctions(node, table, fonction_courante) == -1)
            return -1;
    } else if (node->label == IDENT_ef) {
        // Vérifier l'utilisation d'une variable
        if (verifier_variable_utilisee(node, fonction_courante, table) == -1)
            return -1;
    } else if (node->label == return_ef) {
        // Cette partie est gérée par verifier_return, qui est appelée séparément
    }

    // Parcours récursif des enfants
    for (Node *child = node->firstChild; child; child = child->nextSibling) {
        if (verifier_expressions(child, table, fonction_courante) == -1)
            return -1;
    }

    return 0;
}




symbol_table_variables *var_a_tester = NULL;
bool utilisee = false;

void chercher(Node *n) {
    if (!n || !var_a_tester) return;
  
    if (n->label == IDENT_ef && n->u.ident && var_a_tester->var.id && 
        strcmp(n->u.ident, var_a_tester->var.id) == 0)
        utilisee = true;
  
    for (Node *child = n->firstChild; child; child = child->nextSibling)
        chercher(child);
  }

void verifier_variables_utilisees(symbol_table table, Node *arbre) {
  for (symbol_table_fonctions *f = table.fonctions; f; f = f->next) {
      for (symbol_table_variables *var = f->func.defvars; var; var = var->next) {
          utilisee = false;
          var_a_tester = var;

          Node *declFoncts = arbre->firstChild->nextSibling;
          for (Node *fonct_node = declFoncts->firstChild; fonct_node; fonct_node = fonct_node->nextSibling) {
              if (fonct_node->label == DeclFonct &&
                  strcmp(fonct_node->firstChild->firstChild->nextSibling->u.ident, f->func.id.id) == 0) {
                  Node *corps = fonct_node->firstChild->nextSibling;
                  chercher(corps);
              }
          }

          if (!utilisee) {
              //fprintf(stderr, "Avertissement : variable '%s' déclarée mais jamais utilisée\n", var->var.id);
          }
      }
  }
}

int verifier_return_recursif(Node *node, fonction *func, symbol_table *table, int *has_return) {
    if (!node) return 0;
    
    // Si c'est un return, vérifier sa cohérence avec le type de la fonction
    if (node->label == return_ef) {
        *has_return = 1;
        return verifier_return(node, func, table);
    }
    
    // Vérification récursive pour tous les enfants
    for (Node *child = node->firstChild; child; child = child->nextSibling) {
        if (verifier_return_recursif(child, func, table, has_return) == -1)
            return -1;
    }
    
    return 0;
}



int verifier_semantique(symbol_table table, Node *arbre) {
    int erreur = 0;

    // 1. Variables globales uniques
    if (verifier_variables_globales_uniques(table.variables_global) == -1)
        erreur = -1;

    // 2. Conflits de noms (fonction/variable globale)
    if (verifier_conflits_noms(table) == -1)
        erreur = -1;

    // 3. Paramètres uniques et conflits locaux/paramètres
    for (symbol_table_fonctions *f = table.fonctions; f; f = f->next) {
        if (verifier_variables_fonction(f->func) == -1)
            erreur = -1;
    }

    // 4. Fonction main présente et bien typée
    if (verifier_main(table.fonctions) == -1)
        erreur = -1;

    // 5. Vérification des expressions et des retours dans chaque fonction
    Node *declFoncts = arbre->firstChild->nextSibling;
    
    for (Node *fonct_node = declFoncts->firstChild; fonct_node; fonct_node = fonct_node->nextSibling) {
        if (fonct_node->label == DeclFonct) {
            Node *enTete = fonct_node->firstChild;
            Node *typeNode = enTete->firstChild;
            Node *identNode = typeNode->firstChild;
            
            char *nom_fonction = identNode->u.ident;
            
            symbol_table_fonctions *func = NULL;
            for (symbol_table_fonctions *f = table.fonctions; f; f = f->next) {
                if (strcmp(f->func.id.id, nom_fonction) == 0) {
                    func = f;
                    break;
                }
            }
            
            if (func) {
                Node *corps = enTete->nextSibling;
                
                // Vérifier les déclarations static avec initialisation
                if (corps->firstChild && corps->firstChild->label == DeclVars) {
                    if (verifier_static_declarations(corps->firstChild) == -1)
                        erreur = -1;
                }
                
                // Vérifier toutes les expressions dans le corps de la fonction
                if (verifier_expressions(corps, &table, &func->func) == -1)
                    erreur = -1;
                    
                // Vérifier tous les return en parcourant récursivement l'arbre
                int has_return = 0;
                
                // Vérifier récursivement tous les return dans toutes les instructions
                for (Node *instr = corps->firstChild; instr; instr = instr->nextSibling) {
                    // Sauter les déclarations de variables locales
                    if (instr->label == DeclVars) continue;
                    
                    // Vérifier les return dans cette instruction
                    if (verifier_return_recursif(instr, &func->func, &table, &has_return) == -1)
                        erreur = -1;
                }
                
                // Vérifier qu'une fonction main non void a un return
                if (func->func.id.type != Void && !has_return && strcmp(nom_fonction, "main") == 0) {
                    fprintf(stderr, "Erreur : la fonction %s de type non void n'a pas d'instruction return\n", 
                            func->func.id.id);
                    erreur = -1;
                }
            }
        }
    }

    // 6. Variables inutilisées (avertissements seulement)
    verifier_variables_utilisees(table, arbre);

    return erreur;
}