#include "symbol_table.h"
#include "codegen.h"

void ecrire_code_nasm_main(FILE *fichier) {
    fprintf(fichier, "global _start\n");
    fprintf(fichier, "section .data\n");  // On ne met QUE le header ici
  }
  
  
  
  void ecrire_section_data(FILE *fichier, symbol_table *table) {
    // Variables globales
    for (symbol_table_variables *v = table->variables_global; v != NULL; v = v->next) {
        fprintf(fichier, "%s: dq 0\n", v->var.id);
    }
    
    // Variables statiques des fonctions
    for (symbol_table_fonctions *func = table->fonctions; func != NULL; func = func->next) {
        for (symbol_table_variables *var = func->func.defvars; var != NULL; var = var->next) {
            if (var->var.static_var) {
                // Préfixe avec nom de la fonction pour éviter les collisions
                fprintf(fichier, "static_%s_%s: dq 0\n", func->func.id.id, var->var.id);
            }
        }
    }
    
    // Ajout de données pour les fonctions d'E/S
    fprintf(fichier, "err_msg_getint: db \"Erreur: entrée non numérique\", 10, 0\n");
    fprintf(fichier, "err_msg_len_getint: equ $ - err_msg_getint\n");
    fprintf(fichier, "err_msg_division: db \"Erreur: division par zéro\", 10, 0\n");
    fprintf(fichier, "err_msg_len_division: equ $ - err_msg_division\n");
    fprintf(fichier, "buffer: times 64 db 0\n");  // Tampon pour la lecture
    
    fprintf(fichier, "\n");
}

  // Fonction pour parcourir l'arbre abstrait et vérifier si une fonction est "main"
  void verifier_et_ecrire_main(symbol_table table, FILE *fichier) {
    // Vérifier si la fonction "main" existe dans la table des symboles
    symbol_table_fonctions *current = table.fonctions;
    while (current != NULL) {
      if (strcmp(current->func.id.id, "main") == 0) {
        // Écrire le code NASM pour la fonction "main"
        ecrire_code_nasm_main(fichier);
        return;
      }
      current = current->next;
    }
    // Si la fonction "main" n'existe pas, afficher un message d'erreur
    fprintf(fichier, "Erreur: La fonction 'main' n'existe pas dans le programme.\n");
  }
  
 void generer_appel_fonction(Node *node, FILE *f, bool push_result, fonction *current_func) {
    if (!node || node->label != FunctionCall) {
        fprintf(stderr, "Erreur : appel de fonction invalide.\n");
        return;
    }

    char *function_name = node->u.ident;
    Node *arg = node->firstChild;
    Node *argList[10];
    int count = 0;

    while (arg != NULL && count < 10) {
        argList[count++] = arg;
        arg = arg->nextSibling;
    }

    // Fonctions prédéfinies qui utilisent la convention d'appel System V
    

    // Pour toutes les fonctions (prédef ou utilisateur), placer les arguments dans les registres
    for (int i = 0; i < count; i++) {
    int idx = count - 1 - i;
    generer_expression(argList[idx], f, current_func, NULL);
    fprintf(f, "    pop rax\n");
    switch (i) {
        case 0: fprintf(f, "    mov rdi, rax\n"); break;
        case 1: fprintf(f, "    mov rsi, rax\n"); break;
        case 2: fprintf(f, "    mov rdx, rax\n"); break;
        case 3: fprintf(f, "    mov rcx, rax\n"); break;
        case 4: fprintf(f, "    mov r8, rax\n"); break;
        case 5: fprintf(f, "    mov r9, rax\n"); break;
        default: fprintf(f, "    push rax\n"); break;
    }
}

    fprintf(f, "    call %s\n", function_name);

    if (push_result) {
        fprintf(f, "    push rax\n");
    }
}
  
  
  
  
  
  extern Node *root;
  
  Node *get_corps_main() {
      Node *declFoncts = root->firstChild->nextSibling;
      for (Node *decl = declFoncts->firstChild; decl; decl = decl->nextSibling) {
          Node *enTete = decl->firstChild;
          Node *typeNode = enTete->firstChild;
          Node *identNode = typeNode->firstChild;
          if (identNode && strcmp(identNode->u.ident, "main") == 0) {
              return enTete->nextSibling;  // C’est le Node Corps
          }
      }
      return NULL;
  }
  
  
// Convertit une expression booléenne en valeur arithmétique (0 ou 1)
void generer_bool_to_arithm(Node *expr, FILE *f, fonction *current_func) {
    labels_info labels;
    init_labels(&labels);
    
    // Générer le code booléen avec les étiquettes générées
    generer_boolean_expression(expr, f, current_func, &labels);
    
    // Code pour produire 0 ou 1 selon le résultat
    fprintf(f, "    ; Convertir booléen en valeur arithmétique\n");
    fprintf(f, "    xor rax, rax\n");          // rax = 0 (valeur par défaut)
    fprintf(f, "    jmp %s\n", labels.end_label);
    
    fprintf(f, "%s:\n", labels.true_label);
    fprintf(f, "    mov rax, 1\n");           // rax = 1 si vrai
    
    fprintf(f, "%s:\n", labels.end_label);
    fprintf(f, "    push rax\n");             // Empiler le résultat
    
    fprintf(f, "%s:\n", labels.false_label);  // Un point d'arrivée pour les expressions fausses
}
  
int index_variable_locale_ordre_declaration(fonction *func, const char *ident) {
    // Compter le nombre de variables locales
    int nb_vars = 0;
    for (symbol_table_variables *var = func->defvars; var != NULL; var = var->next)
        if (!var->var.static_var) nb_vars++;

    // Stocker les pointeurs dans un tableau temporaire
    symbol_table_variables *vars[nb_vars];
    int i = 0;
    for (symbol_table_variables *var = func->defvars; var != NULL; var = var->next)
        if (!var->var.static_var) vars[i++] = var;

    // Parcourir le tableau dans l'ordre inverse (ordre de déclaration)
    for (int j = nb_vars - 1, idx = 0; j >= 0; j--, idx++) {
        if (strcmp(vars[j]->var.id, ident) == 0)
            return idx;
    }
    return -1;
}
  
  // Modifie generer_expression pour supporter les expressions booléennes comme valeurs arithmétiques
void generer_expression(Node *expr, FILE *f, fonction *current_func,symbol_table *table) {
    if (!expr) return;
    
    switch (expr->label) {
        case DIGIT_ef:
            fprintf(f, "    mov rax, %d\n", expr->u.num);
            fprintf(f, "    push rax\n");
            break;

           case IDENT_ef: {
    // Vérifie si c'est un paramètre
    int index = 0;
    bool is_param = false;
    bool is_static = false;
    
    // Vérifier d'abord si c'est un paramètre
    for (symbol_table_variables *arg = current_func->args; arg != NULL; arg = arg->next) {
        if (strcmp(arg->var.id, expr->u.ident) == 0) {
            // Paramètres dans les registres selon la convention d'appel
            if (index == 0) {
                fprintf(f, "    push rdi\n");
            } else if (index == 1) {
                fprintf(f, "    push rsi\n");
            } else if (index == 2) {
                fprintf(f, "    push rdx\n");
            } else if (index == 3) {
                fprintf(f, "    push rcx\n");
            } else if (index == 4) {
                fprintf(f, "    push r8\n");
            } else if (index == 5) {
                fprintf(f, "    push r9\n");
            } else {
                // Paramètres au-delà du sixième sont sur la pile
                fprintf(f, "    ; Paramètre %d sur la pile\n", index + 1);
                fprintf(f, "    mov rax, [rbp + %d]\n", (index - 5) * 8 + 16);
                fprintf(f, "    push rax\n");
            }
            is_param = true;
            break;
        }
        index++;
    }
    
    if (!is_param) {
    // Vérifier si c'est une variable statique dans la fonction courante
    for (symbol_table_variables *var = current_func->defvars; var != NULL; var = var->next) {
        if (strcmp(var->var.id, expr->u.ident) == 0 && var->var.static_var) {
            fprintf(f, "    ; Accès à la variable statique %s de %s\n", var->var.id, current_func->id.id);
            fprintf(f, "    mov rax, [static_%s_%s]\n", current_func->id.id, var->var.id);
            fprintf(f, "    push rax\n");
            is_static = true;
            break;
        }
    }

    if (!is_static) {
        int idx = index_variable_locale_ordre_declaration(current_func, expr->u.ident);
        if (idx >= 0) {
            int offset = (idx + 1) * 8;
            fprintf(f, "    mov rax, [rbp - %d]\n", offset);
            fprintf(f, "    push rax\n");
        } else if (table) {
    // Recherche dans les variables globales
    symbol_table_variables *glob = NULL;
    for (symbol_table_variables *v = table->variables_global; v != NULL; v = v->next) {
        if (strcmp(v->var.id, expr->u.ident) == 0) {
            glob = v;
            break;
        }
    }
    if (glob) {
        fprintf(f, "    mov rax, [%s]\n", expr->u.ident);
        fprintf(f, "    push rax\n");
    }
}
    }
}
    break;
}

        case ADDSUB_ef:
         case DIVSTAR_ef: {
            Node *left = expr->firstChild;
            Node *right = left->nextSibling;

            // Générer le code pour l'opérande gauche d'abord
            generer_expression(left, f, current_func, table);
            
            // Puis pour l'opérande droit
            generer_expression(right, f, current_func, table);

            // Récupérer les valeurs dans l'ordre inverse de la pile
            fprintf(f, "    pop rbx\n");   // Second opérande
            fprintf(f, "    pop rax\n");   // Premier opérande

            char op = expr->u.byte;

            switch (op) {
                case '+':
                    fprintf(f, "    add rax, rbx\n");
                    break;
                case '-':
                    fprintf(f, "    sub rax, rbx\n");
                    break;
                case '*':
                    fprintf(f, "    imul rax, rbx\n");
                    break;
                case '/':
                    // Division avec protection contre division par zéro
                    fprintf(f, "    cmp rbx, 0\n");
                    fprintf(f, "    jne .division_%p\n", expr); // Générer une étiquette unique basée sur l'adresse du nœud
                    fprintf(f, "    ; Division par zéro détectée\n");
                    fprintf(f, "    mov rax, 1\n");
                    fprintf(f, "    mov rdi, 1\n"); // Code d'erreur
                    fprintf(f, "    mov rsi, err_msg_division\n"); // Message d'erreur (à définir dans data)
                    fprintf(f, "    mov rdx, err_msg_len_division\n");
                    fprintf(f, "    syscall\n");
                    fprintf(f, "    mov rax, 60\n");  // exit syscall
                    fprintf(f, "    mov rdi, 1\n");   // code de retour 1
                    fprintf(f, "    syscall\n");
                    fprintf(f, ".division_%p:\n", expr);
                    fprintf(f, "    cqo\n");          // Extension de signe pour rdx:rax
                    fprintf(f, "    idiv rbx\n");     // rax = rdx:rax / rbx, rdx = reste
                    break;
                case '%':
                    // Modulo avec protection
                    fprintf(f, "    cmp rbx, 0\n");
                    fprintf(f, "    jne .modulo_%p\n", expr);
                    fprintf(f, "    ; Division par zéro détectée dans modulo\n");
                    fprintf(f, "    mov rax, 1\n");
                    fprintf(f, "    mov rdi, 1\n");
                    fprintf(f, "    mov rsi, err_msg_division\n");
                    fprintf(f, "    mov rdx, err_msg_len_division\n");
                    fprintf(f, "    syscall\n");
                    fprintf(f, "    mov rax, 60\n");
                    fprintf(f, "    mov rdi, 1\n");
                    fprintf(f, "    syscall\n");
                    fprintf(f, ".modulo_%p:\n", expr);
                    fprintf(f, "    cqo\n");
                    fprintf(f, "    idiv rbx\n");
                    fprintf(f, "    mov rax, rdx\n"); // Le résultat du modulo est dans rdx
                    break;
                default:
                    fprintf(stderr, "Opérateur non supporté : %c\n", op);
                    break;
            }

            fprintf(f, "    push rax\n");  // Empiler le résultat pour utilisation ultérieure
            break;
        }
        
        // Traiter les expressions booléennes comme valeurs arithmétiques
        case AND_ef:
        case OR_ef:

        case NOT_ef:

        case EQ_ef:
        case ORDER_ef:
            generer_bool_to_arithm(expr, f, current_func);
            break;

        case FunctionCall:
            generer_appel_fonction(expr, f, false, current_func);
            break;
        
        case CHARACTER_ef:
            fprintf(f, "    mov rax, %d\n", expr->u.byte);  // Valeur ASCII du caractère
            fprintf(f, "    push rax\n");
            break;

        default:
            fprintf(stderr, "Expression non supportée : label = %d\n", expr->label);
            break;
    }
}
  
  
// Amélioration du traitement des instructions return
void generer_instr(Node *instr, FILE *f, fonction *current_func, symbol_table *table) {
    if (instr->label == ASSIGN_ef) {
    Node *gauche = instr->firstChild;
    Node *droite = gauche->nextSibling;

    // Générer le code pour l'expression droite
    if (droite->label == FunctionCall) {
        generer_appel_fonction(droite, f, false, current_func);
        // rax contient la valeur de retour
    } else {
        generer_expression(droite, f, current_func, table);
        fprintf(f, "    pop rax\n");  // Récupérer la valeur calculée
    }
    
    // Vérifier si la variable gauche est statique
    bool is_static = false;
    for (symbol_table_variables *var = current_func->defvars; var != NULL; var = var->next) {
        if (strcmp(var->var.id, gauche->u.ident) == 0 && var->var.static_var) {
            fprintf(f, "    ; Assigner à la variable statique %s de %s\n", var->var.id, current_func->id.id);
            fprintf(f, "    mov [static_%s_%s], rax\n", current_func->id.id, var->var.id);
            is_static = true;
            break;
        }
    }
    
   if (!is_static) {
    int idx = index_variable_locale_ordre_declaration(current_func, gauche->u.ident);
    if (idx >= 0) {
        int offset = (idx + 1) * 8;
        fprintf(f, "    mov [rbp - %d], rax\n", offset);
    } else if (table) {
        // Recherche dans les variables globales
        symbol_table_variables *glob = NULL;
        for (symbol_table_variables *v = table->variables_global; v != NULL; v = v->next) {
            if (strcmp(v->var.id, gauche->u.ident) == 0) {
                glob = v;
                break;
            }
        }
        if (glob) {
            fprintf(f, "    mov [%s], rax\n", gauche->u.ident);
        }
    }
}
}
  
   else if (instr->label == return_ef) {
    Node *ret_expr = instr->firstChild;
    if (ret_expr) {
        if (ret_expr->label == FunctionCall) {
            generer_appel_fonction(ret_expr, f, false, current_func);
            // rax contient déjà la valeur de retour
        } else {
            generer_expression(ret_expr, f, current_func, table);
            fprintf(f, "    pop rax\n"); // Mettre le résultat dans rax pour le retour
        }
    } else {
        fprintf(f, "    mov rax, 0\n");
    }
    // Pas de push rax ici !
}
  
    else if (instr->label == FunctionCall) {
        generer_appel_fonction(instr, f, false, current_func);
    }
  
    else if (instr->label == if_ef) {
        // Ajouter des messages de débogage pour comprendre ce qui se passe
        printf("Génération de code pour une instruction if-else\n");
        generer_if_else(instr, f, current_func, table);
    }
    else if (instr->label == while_ef) {
    labels_info labels;
    init_labels(&labels);

    fprintf(f, "while_%d:\n", labels.counter);

    Node *condition = instr->firstChild;
    generer_boolean_expression(condition, f, current_func, &labels);

    fprintf(f, "%s:\n", labels.true_label);
    Node *body = condition->nextSibling;

    if (body) {
    // Parcours de toutes les instructions du corps du while
    for (Node *instr = body; instr != NULL; instr = instr->nextSibling) {
        if (strcmp(StringFromLabel[instr->label], "Instr") == 0 || strcmp(StringFromLabel[instr->label], "SuiteInstr") == 0) {
            generer_suite_instr(instr, f, current_func, table);
        } else {
            generer_instr(instr, f, current_func, table);
        }
    }
} else {
       // printf("ERREUR: corps de boucle while manquant\n");
    }

    fprintf(f, "    jmp while_%d\n", labels.counter);
    fprintf(f, "%s:\n", labels.false_label);
}
    else {
      //  printf("      ➤ Pas une instruction reconnue, label = %d\n", instr->label);
    }
}
  
void generer_suite_instr(Node *corps, FILE *f, fonction *current_func, symbol_table *table) {
    for (Node *child = corps->firstChild; child != NULL; child = child->nextSibling) {
        
        
        if (child->label != DeclVars) {
            if (child->label == if_ef) {
                // Traitement direct du if-else quand il est enfant direct du corps
             
                generer_if_else(child, f, current_func, table);
            }
            else if (child->label == while_ef) {
                // Traitement direct de la boucle while
               
                generer_instr(child, f, current_func, table);  // Utiliser la même implémentation
            }
            else if (child->label == ASSIGN_ef || child->label == return_ef || child->label == FunctionCall) {
                // Cas simple : une instruction directe
                generer_instr(child, f, current_func, table);
            } 
            else if (strcmp(StringFromLabel[child->label], "Instr") == 0 || strcmp(StringFromLabel[child->label], "SuiteInstr") == 0) {
                // Si c'est un conteneur d'instructions
                for (Node *instr = child->firstChild; instr; instr = instr->nextSibling) {
                   // printf("     Instruction rencontrée : %d (%s)\n", instr->label, StringFromLabel[instr->label]);
                    generer_instr(instr, f, current_func, table);
                }
            } 
            else if (child->label == IDENT_ef && child->nextSibling && child->nextSibling->label == DIGIT_ef) {
                // Cas spécial: détection de l'affectation a = 1 dans le bloc if
               
                
                // Créer une instruction d'affectation
                fprintf(f, "    mov rax, %d\n", child->nextSibling->u.num);
                fprintf(f, "    push rax\n");
                fprintf(f, "    pop rax\n");
                
                // Calculer l'offset pour cette variable
                int offset = 0;
                for (symbol_table_variables *var = current_func->defvars; var != NULL; var = var->next) {
                    if (!var->var.static_var) {
                        if (strcmp(var->var.id, child->u.ident) == 0) {
                            break;
                        }
                        offset += 8;
                    }
                }
                fprintf(f, "    mov [rsp + %d], rax\n", offset);
                
                // Sauter un nœud car on a déjà traité le chiffre
                child = child->nextSibling;
            }
            else {
                //printf(" Nœud inattendu dans le corps : %d (%s)\n", child->label, StringFromLabel[child->label]);
            }
        }
    }
}
  
  int compter_arguments(symbol_table_variables *args) {
    int count = 0;
    for (; args != NULL; args = args->next) {
        count++;
    }
    return count;
  }
  
  symbol_table_fonctions* chercher_fonction(symbol_table table, const char *nom) {
    for (symbol_table_fonctions *f = table.fonctions; f != NULL; f = f->next) {
        if (strcmp(f->func.id.id, nom) == 0) {
            return f;
        }
    }
    return NULL;
  }
  
  void generer_fonctions_utiles(Node *tree, FILE *fichier, symbol_table table) {
    Node *declFoncts = tree->firstChild->nextSibling;

    for (Node *decl = declFoncts->firstChild; decl; decl = decl->nextSibling) {
        Node *enTete = decl->firstChild;
        Node *typeNode = enTete->firstChild;
        Node *identNode = typeNode->firstChild;

        if (identNode && strcmp(identNode->u.ident, "main") != 0) {
            //printf("Génération de la fonction : %s\n", identNode->u.ident);
            fprintf(fichier, "\n%s:\n", identNode->u.ident);

            symbol_table_fonctions *func_info = chercher_fonction(table, identNode->u.ident);

            Node *corps = enTete->nextSibling;
            if (corps) {
                generer_suite_instr(corps, fichier, &(func_info->func), &table);
            } else {
                //printf("Pas de corps trouvé pour la fonction %s\n", identNode->u.ident);
            }

            // Convention : la valeur de retour doit être dans rax (déjà géré par return)
            fprintf(fichier, "    ret\n");
        }
    }
}
  
// Initialiser des étiquettes uniques pour la comparaison
void init_labels(labels_info *labels) {
    static int global_counter = 0;
    labels->counter = global_counter++;
    sprintf(labels->true_label, "true_%d", labels->counter);
    sprintf(labels->false_label, "false_%d", labels->counter);
    sprintf(labels->end_label, "end_%d", labels->counter);
}

void generer_comparison(Node *expr, FILE *f, fonction *current_func, labels_info *labels) {
   // printf("Génération d'une comparaison: %s\n", expr->u.comp);
    
    // Générer le code des deux côtés de la comparaison
    Node *left = expr->firstChild;
    Node *right = left->nextSibling;
    
    generer_expression(left, f, current_func,NULL);  // Empile le résultat gauche
    generer_expression(right, f, current_func,NULL); // Empile le résultat droit
    
    fprintf(f, "    pop rbx\n");   // Second opérande
    fprintf(f, "    pop rax\n");   // Premier opérande
    fprintf(f, "    cmp rax, rbx\n"); // Compare rax et rbx
    
    // Effectuer le saut approprié en fonction de l'opérateur de comparaison
    if (expr->label == EQ_ef) {
        if (strcmp(expr->u.comp, "==") == 0) {
            fprintf(f, "    je %s\n", labels->true_label);
        } else if (strcmp(expr->u.comp, "!=") == 0) {
            fprintf(f, "    jne %s\n", labels->true_label);
        }
    } else if (expr->label == ORDER_ef) {
        if (strcmp(expr->u.comp, "<") == 0) {
            fprintf(f, "    jl %s\n", labels->true_label);
        } else if (strcmp(expr->u.comp, ">") == 0) {
            fprintf(f, "    jg %s\n", labels->true_label);
        } else if (strcmp(expr->u.comp, "<=") == 0) {
            fprintf(f, "    jle %s\n", labels->true_label);
        } else if (strcmp(expr->u.comp, ">=") == 0) {
            fprintf(f, "    jge %s\n", labels->true_label);
        }
    }
    
    // Si la condition n'est pas remplie, on va au bloc false
    fprintf(f, "    jmp %s\n", labels->false_label);
}

// Extension de generer_expression pour prendre en compte les étiquettes
// ...existing code...

void generer_expression_bool(Node *expr, FILE *f, fonction *current_func, labels_info *labels) {
    if (!expr) return;

    switch (expr->label) {
        case EQ_ef:
        case ORDER_ef:
            generer_comparison(expr, f, current_func, labels);
            break;

        case AND_ef: {
            Node *gauche = expr->firstChild;
            Node *droite = gauche ? gauche->nextSibling : NULL;

            if (!droite) {
                generer_expression_bool(gauche, f, current_func, labels);
            } else {
                labels_info left_labels;
                init_labels(&left_labels);

                // Générer la première condition
                generer_expression_bool(gauche, f, current_func, &left_labels);

                // Si la première condition est vraie, on vérifie la seconde
                fprintf(f, "%s:\n", left_labels.true_label);
                generer_expression_bool(droite, f, current_func, labels);

                // Si la première condition est fausse, on saute directement au bloc faux final
                fprintf(f, "%s:\n", left_labels.false_label);
                fprintf(f, "    jmp %s\n", labels->false_label);
            }
            break;
        }

        case OR_ef: {
            Node *gauche = expr->firstChild;
            Node *droite = gauche ? gauche->nextSibling : NULL;

            if (!droite) {
                generer_expression_bool(gauche, f, current_func, labels);
            } else {
                labels_info left_labels;
                init_labels(&left_labels);

                generer_expression_bool(gauche, f, current_func, &left_labels);

                fprintf(f, "%s:\n", left_labels.true_label);
                fprintf(f, "    jmp %s\n", labels->true_label);

                fprintf(f, "%s:\n", left_labels.false_label);
                generer_expression_bool(droite, f, current_func, labels);
            }
            break;
        }

        case NOT_ef: {
            labels_info not_labels = *labels;
            char tmp[64];
            strcpy(tmp, not_labels.true_label);
            strcpy(not_labels.true_label, not_labels.false_label);
            strcpy(not_labels.false_label, tmp);
            generer_expression_bool(expr->firstChild, f, current_func, &not_labels);
            break;
        }

        default:
            generer_expression(expr, f, current_func,NULL);
            fprintf(f, "    pop rax\n");
            fprintf(f, "    cmp rax, 0\n");
            fprintf(f, "    jne %s\n", labels->true_label);
            fprintf(f, "    jmp %s\n", labels->false_label);
            break;
    }
}


// Générer le code pour une structure if-else
// Générer le code pour une structure if-else
void generer_if_else(Node *node, FILE *f, fonction *current_func, symbol_table *table) {
    labels_info labels;
    init_labels(&labels);
    
   // printf("Traitement IF-ELSE : début\n");
    
    // Générer le code pour la condition
    Node *condition = node->firstChild;
    if (!condition) {
       // printf("ERREUR : aucune condition trouvée dans le nœud if\n");
        return;
    }
    
   // printf("Génération de la condition\n");
    generer_expression_bool(condition, f, current_func, &labels);
    
    // Bloc true
    //printf("Génération du bloc TRUE\n");
    fprintf(f, "%s:\n", labels.true_label);
    Node *true_block = condition->nextSibling;
    if (true_block) {
        // Si le bloc true est une instruction de return
        if (true_block->label == return_ef) {
   // printf("Bloc TRUE contient un return\n");
    // Générer l'expression de retour et la mettre dans rax
    if (true_block->firstChild) {
        generer_expression(true_block->firstChild, f, current_func,NULL);
        fprintf(f, "    pop rax\n");  // Mettre la valeur de retour dans rax
    } else {
        fprintf(f, "    xor rax, rax\n");  // Retour 0 par défaut
    }
    fprintf(f, "    mov rdi, rax\n"); // Valeur de retour dans rdi pour exit
    // Sortir directement avec syscall exit sans continuer l'exécution
    fprintf(f, "    ; Return dans le bloc IF - terminer le programme immédiatement\n");
    fprintf(f, "    mov rax, 60\n");    // syscall exit
    fprintf(f, "    syscall\n");        // Ne pas continuer l'exécution
}
        else {
            // Traiter les instructions dans le bloc true
            if (true_block->label == ASSIGN_ef || true_block->label == FunctionCall) {
                generer_instr(true_block, f, current_func, table);
            } else {
                generer_suite_instr(true_block, f, current_func, table);
            }
            // Sauter à la fin pour éviter d'exécuter le bloc else
            fprintf(f, "    jmp %s\n", labels.end_label);
        }
    } else {
        //printf("ERREUR : aucun bloc then trouvé\n");
    }
    
    // Bloc false (else)
   // printf("Génération du bloc FALSE\n");
    fprintf(f, "%s:\n", labels.false_label);
    if (true_block && true_block->nextSibling) {
        // S'il y a un bloc else
        Node *else_block = true_block->nextSibling;
        // Si le bloc else est une instruction de return
        if (else_block->label == return_ef) {
           // printf("Bloc FALSE contient un return\n");
            
            // Générer l'expression de retour et la mettre dans rdi
            if (else_block->firstChild) {
                generer_expression(else_block->firstChild, f, current_func,NULL);
                fprintf(f, "    pop rdi\n");  // Mettre la valeur de retour dans rdi
            } else {
                fprintf(f, "    xor rdi, rdi\n");  // Retour 0 par défaut
            }
            
            // Sortir directement avec syscall exit
            fprintf(f, "    ; Return dans le bloc ELSE - terminer le programme immédiatement\n");
            fprintf(f, "    mov rax, 60\n");    // syscall exit
            fprintf(f, "    syscall\n");        // Ne pas continuer l'exécution
        } else {
            // Traiter les instructions dans le bloc else
            if (else_block->label == ASSIGN_ef || else_block->label == FunctionCall) {
                generer_instr(else_block, f, current_func, table);
            } else {
                generer_suite_instr(else_block, f, current_func, table);
            }
        }
    } else {
       // printf("Info : pas de bloc else trouvé\n");
    }
    
    // Marquer la fin du if-else
    fprintf(f, "%s:\n", labels.end_label);
    
   // printf("Traitement IF-ELSE : fin\n");
}

// Déclaration préalable pour la récursion mutuelle
void generer_boolean_expression(Node *expr, FILE *f, fonction *current_func, labels_info *labels);
// Générer le code pour les expressions booléennes
void generer_boolean_expression(Node *expr, FILE *f, fonction *current_func, labels_info *labels) {
    if (!expr) return;
    
    switch (expr->label) {
        case AND_ef: {
            // Pour AND, on doit vérifier les deux conditions
            labels_info left_labels;
            init_labels(&left_labels);
            
            // Générer la première condition
            Node *gauche = expr->firstChild;
            generer_expression_bool(gauche, f, current_func, &left_labels);
            
            // Si la première condition est vraie, on vérifie la seconde
            fprintf(f, "%s:\n", left_labels.true_label);
            // Pour la deuxième condition, utilisez directement les étiquettes finales
            Node *droite = gauche->nextSibling;
            generer_expression_bool(droite, f, current_func, labels);
            
            // Si la première condition est fausse, on saute directement au bloc faux final
            fprintf(f, "%s:\n", left_labels.false_label);
            fprintf(f, "    jmp %s\n", labels->false_label);
            break;
        }
        
        case OR_ef: {
            // Pour OR, la première condition vraie suffit
            labels_info left_labels;
            init_labels(&left_labels);
            
            // Générer la première condition
            Node *gauche = expr->firstChild;
            generer_expression_bool(gauche, f, current_func, &left_labels);
            
            // Si la première condition est vraie, on va directement au bloc vrai final
            fprintf(f, "%s:\n", left_labels.true_label);
            fprintf(f, "    jmp %s\n", labels->true_label);
            
            // Si la première condition est fausse, on vérifie la seconde
            fprintf(f, "%s:\n", left_labels.false_label);
            Node *droite = gauche->nextSibling;
            generer_expression_bool(droite, f, current_func, labels);
            break;
        }
        
        case NOT_ef: {
            // Pour NOT, on inverse les étiquettes true et false
            labels_info not_labels;
            init_labels(&not_labels);
            
            // Configurer les étiquettes inversées
            strcpy(not_labels.true_label, labels->false_label);
            strcpy(not_labels.false_label, labels->true_label);
            strcpy(not_labels.end_label, labels->end_label);
            
            // Générer l'expression à nier avec les étiquettes inversées
            generer_expression_bool(expr->firstChild, f, current_func, &not_labels);
            break;
        }
        
        case EQ_ef:
        case ORDER_ef:
            // Comparaisons
            generer_comparison(expr, f, current_func, labels);
            break;
            
        default:
            // Expression arithmétique interprétée comme booléenne
            generer_expression(expr, f, current_func, NULL);
            fprintf(f, "    pop rax\n");
            fprintf(f, "    test rax, rax\n");
            fprintf(f, "    jnz %s\n", labels->true_label);  // Si non zéro, c'est vrai
            fprintf(f, "    jmp %s\n", labels->false_label);
            break;
    }
}

void ecrire_fonctions_io(FILE *fichier) {
    // Implémentation de putchar
    fprintf(fichier, "putchar:\n");
    fprintf(fichier, "    ; Enregistrer le contexte\n");
    fprintf(fichier, "    push rbp\n");
    fprintf(fichier, "    mov rbp, rsp\n");
    
    fprintf(fichier, "    ; Réserver de l'espace et préparer le caractère\n");
    fprintf(fichier, "    sub rsp, 1          ; Réserver 1 octet pour le caractère\n");
    fprintf(fichier, "    mov BYTE [rsp], dil ; Stocker le caractère (dil = 8 bits bas de rdi)\n");
    
    fprintf(fichier, "    ; Afficher le caractère\n");
    fprintf(fichier, "    mov rax, 1          ; syscall write\n");
    fprintf(fichier, "    mov rdi, 1          ; descripteur stdout\n");
    fprintf(fichier, "    mov rsi, rsp        ; adresse du caractère\n");
    fprintf(fichier, "    mov rdx, 1          ; longueur = 1 octet\n");
    fprintf(fichier, "    syscall\n");
    
    fprintf(fichier, "    ; Restaurer le contexte et retourner\n");
    fprintf(fichier, "    add rsp, 1          ; Libérer l'espace\n");
    fprintf(fichier, "    pop rbp\n");
    fprintf(fichier, "    ret\n\n");
    
    // Implémentation de getchar
    fprintf(fichier, "getchar:\n");
    fprintf(fichier, "    ; Enregistrer le contexte\n");
    fprintf(fichier, "    push rbp\n");
    fprintf(fichier, "    mov rbp, rsp\n");
    
    fprintf(fichier, "    ; Lire un caractère\n");
    fprintf(fichier, "    sub rsp, 1          ; Réserver 1 octet pour le caractère\n");
    fprintf(fichier, "    mov rax, 0          ; syscall read\n");
    fprintf(fichier, "    mov rdi, 0          ; descripteur stdin\n");
    fprintf(fichier, "    mov rsi, rsp        ; adresse où stocker le caractère\n");
    fprintf(fichier, "    mov rdx, 1          ; longueur = 1 octet\n");
    fprintf(fichier, "    syscall\n");
    
    fprintf(fichier, "    ; Charger le résultat et retourner\n");
    fprintf(fichier, "    movzx rax, BYTE [rsp] ; Charger le caractère dans AL avec extension de zéros\n");
    fprintf(fichier, "    add rsp, 1          ; Restaurer l'espace\n");
    fprintf(fichier, "    pop rbp\n");
    fprintf(fichier, "    ret\n\n");
    
    // Implémentation de putint
    fprintf(fichier, "putint:\n");
    fprintf(fichier, "    ; Enregistrer le contexte\n");
    fprintf(fichier, "    push rbp\n");
    fprintf(fichier, "    mov rbp, rsp\n");
    fprintf(fichier, "    push rbx            ; Sauvegarder rbx (utilisé)\n");
    fprintf(fichier, "    push r12            ; Sauvegarder r12 (utilisé)\n");
    
    fprintf(fichier, "    ; Préparer la conversion\n");
    fprintf(fichier, "    mov rax, rdi        ; Nombre à convertir\n");
    fprintf(fichier, "    mov r12, 10         ; Base décimale\n");
    fprintf(fichier, "    mov rbx, rsp        ; Position de départ dans la pile\n");
    fprintf(fichier, "    sub rsp, 32         ; Réserver espace pour les chiffres (jusqu'à 32 chiffres)\n");

    fprintf(fichier, "    ; Cas spécial pour zéro\n");
    fprintf(fichier, "    test rax, rax\n");
    fprintf(fichier, "    jnz .not_zero\n");
    fprintf(fichier, "    mov BYTE [rbx-1], '0'\n");
    fprintf(fichier, "    sub rbx, 1\n");
    fprintf(fichier, "    jmp .print_digits\n");

    fprintf(fichier, ".not_zero:\n");
    fprintf(fichier, "    ; Extraire les chiffres un par un\n");
    fprintf(fichier, ".extract_digit:\n");
    fprintf(fichier, "    xor rdx, rdx        ; Préparer la division\n");
    fprintf(fichier, "    div r12             ; Diviser par 10, reste dans rdx\n");
    fprintf(fichier, "    add rdx, '0'        ; Convertir en ASCII\n");
    fprintf(fichier, "    sub rbx, 1          ; Déplacer la position\n");
    fprintf(fichier, "    mov [rbx], dl       ; Stocker le chiffre\n");
    fprintf(fichier, "    test rax, rax       ; Vérifier si terminé\n");
    fprintf(fichier, "    jnz .extract_digit  ; Continuer si pas fini\n");
    
    fprintf(fichier, ".print_digits:\n");
    fprintf(fichier, "    ; Calculer la longueur de la chaîne\n");
    fprintf(fichier, "    mov rdx, rsp        ; Position initiale\n");
    fprintf(fichier, "    add rdx, 32         ; Revenir à la position de départ\n");
    fprintf(fichier, "    sub rdx, rbx        ; Calculer la longueur\n");
    
    fprintf(fichier, "    ; Afficher le nombre\n");
    fprintf(fichier, "    mov rax, 1          ; syscall write\n");
    fprintf(fichier, "    mov rdi, 1          ; descripteur stdout\n");
    fprintf(fichier, "    mov rsi, rbx        ; adresse de début des chiffres\n");
    fprintf(fichier, "    ; rdx contient déjà la longueur\n");
    fprintf(fichier, "    syscall\n");
    
    fprintf(fichier, "    ; Restaurer le contexte et retourner\n");
    fprintf(fichier, "    add rsp, 32         ; Libérer l'espace réservé\n");
    fprintf(fichier, "    pop r12             ; Restaurer r12\n");
    fprintf(fichier, "    pop rbx             ; Restaurer rbx\n");
    fprintf(fichier, "    pop rbp\n");
    fprintf(fichier, "    ret\n\n");
    
    // Implémentation de getint
    fprintf(fichier, "getint:\n");
    fprintf(fichier, "    ; Enregistrer le contexte\n");
    fprintf(fichier, "    push rbp\n");
    fprintf(fichier, "    mov rbp, rsp\n");
    fprintf(fichier, "    push rbx            ; Sauvegarder rbx (utilisé)\n");
    fprintf(fichier, "    push r12            ; Sauvegarder r12 (utilisé pour savoir si on a lu au moins un chiffre)\n");
    
    fprintf(fichier, "    ; Initialiser les registres\n");
    fprintf(fichier, "    xor rbx, rbx        ; Initialiser le résultat à 0\n");
    fprintf(fichier, "    xor r12, r12        ; Aucun chiffre lu pour le moment\n");
    
    fprintf(fichier, ".read_loop:\n");
    fprintf(fichier, "    ; Appeler getchar\n");
    fprintf(fichier, "    call getchar\n");
    
    fprintf(fichier, "    ; Vérifier si c'est un chiffre (ASCII '0' = 48, '9' = 57)\n");
    fprintf(fichier, "    cmp al, '0'\n");
    fprintf(fichier, "    jl .not_digit\n");
    fprintf(fichier, "    cmp al, '9'\n");
    fprintf(fichier, "    jg .not_digit\n");
    
    fprintf(fichier, "    ; C'est un chiffre, l'ajouter au résultat\n");
    fprintf(fichier, "    inc r12             ; Incrémenter le compteur de chiffres\n");
    fprintf(fichier, "    sub al, '0'         ; Convertir ASCII en valeur numérique\n");
    fprintf(fichier, "    movzx rax, al       ; Étendre al à rax (sans signe)\n");
    fprintf(fichier, "    imul rbx, 10        ; Multiplier résultat par 10\n");
    fprintf(fichier, "    add rbx, rax        ; Ajouter le nouveau chiffre\n");
    fprintf(fichier, "    jmp .read_loop      ; Continuer à lire\n");
    
    fprintf(fichier, ".not_digit:\n");
    fprintf(fichier, "    ; Vérifier si on a déjà lu au moins un chiffre\n");
    fprintf(fichier, "    test r12, r12\n");
    fprintf(fichier, "    jz .first_not_digit ; Si premier caractère n'est pas un chiffre\n");
    
    fprintf(fichier, "    ; Vérifier si c'est un caractère de fin de ligne ou d'espace\n");
    fprintf(fichier, "    cmp al, 10          ; LF (Retour à la ligne)\n");
    fprintf(fichier, "    je .end_read\n");
    fprintf(fichier, "    cmp al, 13          ; CR\n");
    fprintf(fichier, "    je .end_read\n");
    fprintf(fichier, "    cmp al, ' '         ; Espace\n");
    fprintf(fichier, "    je .end_read\n");
    fprintf(fichier, "    cmp al, 9           ; Tab (ASCII 9)\n");
    fprintf(fichier, "    je .end_read\n");
    
    fprintf(fichier, ".first_not_digit:\n");
    fprintf(fichier, "    ; Si premier caractère n'est pas un chiffre, terminer avec erreur\n");
    fprintf(fichier, "    ; Afficher message d'erreur\n");
    fprintf(fichier, "    mov rax, 1          ; syscall write\n");
    fprintf(fichier, "    mov rdi, 2          ; stderr\n");
    fprintf(fichier, "    mov rsi, err_msg_getint\n");
    fprintf(fichier, "    mov rdx, err_msg_len_getint\n");
    fprintf(fichier, "    syscall\n");
    
    fprintf(fichier, "    ; Terminer le programme avec code de retour 5\n");
    fprintf(fichier, "    mov rax, 60         ; syscall exit\n");
    fprintf(fichier, "    mov rdi, 5          ; code de retour = 5 (erreur E/S)\n");
    fprintf(fichier, "    syscall\n");
    
    fprintf(fichier, ".end_read:\n");
    fprintf(fichier, "    ; Retourner le résultat dans rax\n");
    fprintf(fichier, "    mov rax, rbx\n");
    fprintf(fichier, "    pop r12             ; Restaurer r12\n");
    fprintf(fichier, "    pop rbx             ; Restaurer rbx\n");
    fprintf(fichier, "    pop rbp\n");
    fprintf(fichier, "    ret\n\n");
}

int compter_vars_locales(fonction *func) {
    int count = 0;
    for (symbol_table_variables *var = func->defvars; var != NULL; var = var->next) {
        if (!var->var.static_var) count++;
    }
    return count;
}
// Amélioration de la gestion des variables locales dans la fonction main
void genere_code_nasm(symbol_table table, FILE *fichier) {
    // 1. Écrire le header et les données globales
    ecrire_code_nasm_main(fichier);
    ecrire_section_data(fichier, &table);
  
    // 2. Section .text avec les fonctions
    fprintf(fichier, "section .text\n");
    
    // Ajout des fonctions d'E/S
    ecrire_fonctions_io(fichier);
    
    generer_fonctions_utiles(root, fichier, table);  // génère toutes les fonctions sauf main
  
    // 3. Point d'entrée _start
    fprintf(fichier, "_start:\n");
  
    Node *corps = get_corps_main();
    if (corps != NULL) {
        // On cherche la fonction main dans la table des symboles
        symbol_table_fonctions *main_func = chercher_fonction(table, "main");
  
        if (main_func != NULL) {
            // CORRECTION: Initialisation correcte du frame pointer
            fprintf(fichier, "    ; Initialisation du frame pointer\n");
            fprintf(fichier, "    push rbp\n");         // Sauvegarder l'ancien frame pointer
            fprintf(fichier, "    mov rbp, rsp\n");     // rbp = rsp (nouveau frame pointer)
            
            // Compter les variables locales pour réserver l'espace nécessaire
            int nb_vars_locales = compter_vars_locales(&main_func->func);
            
            if (nb_vars_locales > 0) {
    fprintf(fichier, "    ; Réservation d'espace pour %d variables locales\n", nb_vars_locales);
    fprintf(fichier, "    sub rsp, %d\n", nb_vars_locales * 8);

    // Initialisation à 0 de toutes les variables locales
    
    for (int j = 0; j < nb_vars_locales; j++) {
        int offset = (j + 1) * 8;
        fprintf(fichier, "    mov QWORD [rbp - %d], 0\n", offset);
    }
}
            
            // Générer le code du corps de main
            generer_suite_instr(corps, fichier, &(main_func->func), &table);
            
            // CORRECTION: S'assurer que la valeur de retour est bien récupérée
            fprintf(fichier, "    ; Récupérer la valeur de retour dans rdi pour la sortie\n");
            //fprintf(fichier, "    pop rax\n");
            fprintf(fichier, "    mov rdi, rax\n");
            
            // Restaurer la pile et le frame pointer
            fprintf(fichier, "    ; Restaurer le frame pointer\n");
            fprintf(fichier, "    mov rsp, rbp\n");     // rsp = rbp (désallouer les variables)
            fprintf(fichier, "    pop rbp\n");          // Restaurer l'ancien rbp
        } else {
            fprintf(stderr, "Erreur : fonction 'main' introuvable dans la table des symboles\n");
        }
    } else {
        fprintf(stderr, "Erreur : corps de la fonction main introuvable\n");
    }
  
    // 4. Terminer le programme
    fprintf(fichier, "    ; Terminer le programme en utilisant la valeur de retour\n");
    fprintf(fichier, "    mov rax, 60\n");        // syscall exit
    fprintf(fichier, "    syscall\n");            // rdi contient déjà le code de sortie
}