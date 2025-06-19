#include "symbol_table.h"




symbol_table_variables * create_variable_global(){
    symbol_table_variables * var = malloc(sizeof(symbol_table_variables));
    if (var == NULL){
      printf("Erreur d'allocation de mémoire\n");
      exit(1);
    }
    var->next = NULL;
    return var; 
  }
  
  // Modification de remplir_var_global pour gérer les déclarations multiples
  symbol_table_variables *remplir_var_global(Node *tree) {
      if (!tree || tree->label != Prog) {
          fprintf(stderr, "Erreur : arbre racine invalide\n");
          return NULL;
      }
  
      symbol_table_variables *head = NULL;
      symbol_table_variables *tail = NULL;
  
      // Parcourt tous les enfants de Prog
      for (Node *decl = tree->firstChild; decl != NULL; decl = decl->nextSibling) {
          // Stoppe dès qu'on entre dans DeclFoncts
          if (decl->label == DeclFoncts)
              break;
  
          // Sinon, ce sont des déclarations globales
          for (Node *var_node = decl->firstChild; var_node != NULL; var_node = var_node->nextSibling) {
              if (var_node->label != TYPE_ef) continue;
  
              // Déterminer le type commun pour cette déclaration
              type var_type;
              if (strcmp(var_node->u.ident, "int") == 0)
                  var_type = Int;
              else if (strcmp(var_node->u.ident, "char") == 0)
                  var_type = Char;
              else
                  var_type = -1;
              
              // Parcourir tous les identificateurs dans cette déclaration
              for (Node *id_node = var_node->firstChild; id_node != NULL; id_node = id_node->nextSibling) {
                  if (id_node->label == IDENT_ef) {
                      Variable var;
                      var.static_var = false;
                      var.type = var_type;
                      var.id = strdup(id_node->u.ident);
                      
                      symbol_table_variables *node = malloc(sizeof(symbol_table_variables));
                      node->var = var;
                      node->next = NULL;
                      
                      if (!head) {
                          head = node;
                          tail = node;
                      } else {
                          tail->next = node;
                          tail = node;
                      }
                      
                      //printf("Ajout variable globale : %s (type %d)\n", var.id, var.type);
                  }
              }
          }
      }
  
      return head;
  }
  
  
  
  symbol_table_fonctions * create_fonction(){
    symbol_table_fonctions * func = malloc(sizeof(symbol_table_fonctions));
    if (func == NULL){
      printf("Erreur d'allocation de mémoire\n");
      exit(1);
    }
    return func;
  }
  
  void debug_print_tree_structure(Node *node, int depth) {
    if (node == NULL) return;
  
    // Indentation pour montrer la hiérarchie
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
  
    // Imprimer le label du nœud
    printf("Depth %d - Label: ", depth);
    switch(node->label) {
        case Prog: printf("Prog"); break;
        case DeclVars: printf("DeclVars"); break;
        case GlobVar: printf("GlobVar"); break;
        case DeclFoncts: printf("DeclFoncts"); break;
        case DeclFonct: printf("DeclFonct"); break;
        case EnTeteFonct: printf("EnTeteFonct"); break;
        case TYPE_ef: 
            printf("TYPE_ef: %s", node->u.ident); 
            break;
        case IDENT_ef: 
            printf("IDENT_ef: %s", node->u.ident); 
            break;
        case void_ef: printf("void_ef"); break;
          case int_ef: printf("int_ef"); break;
          case char_ef: printf("char_ef"); break;
          case static_ef: printf("static_ef"); break;
          case FunctionCall: printf("FunctionCall %s", node->u.ident); break;
          case return_ef: printf("return_ef"); break;
          case ASSIGN_ef: printf("ASSIGN_ef"); break;
          case ADDSUB_ef: printf("ADDSUB_ef"); break;
          case DIVSTAR_ef: printf("DIVSTAR_ef"); break;
          case EQ_ef: printf("EQ_ef"); break;
          case ORDER_ef: printf("ORDER_ef"); break;
          case AND_ef: printf("AND_ef"); break;
          case OR_ef: printf("OR_ef"); break;
          case NOT_ef: printf("NOT_ef"); break;
          case DIGIT_ef: printf("DIGIT_ef: %d", node->u.num); break;
          case Corps : printf("Corps"); break;
        default: printf("Unknown label: %d", node->label);
    }
    printf("\n");
  
    // Parcourir les enfants
    if (node->firstChild) {
        debug_print_tree_structure(node->firstChild, depth + 1);
    }
  
    // Parcourir les frères
    if (node->nextSibling) {
        debug_print_tree_structure(node->nextSibling, depth);
    }
  }
  
  int verifier_static_initialisees(Node *decl) {
    if (!decl || decl->label != static_ef) return 0;
  
    Node *type_node = decl->firstChild; // type
    Node *ident_node = type_node->firstChild; // identifiant
    Node *init = ident_node->nextSibling; // potentielle initialisation
  
    if (init) {
        fprintf(stderr, "Erreur ligne %d : initialisation interdite d'une variable static locale\n", decl->lineno);
        return -1;
    }
  
    return 0;
  }
  
  
  symbol_table_fonctions *remplir_fonction(Node *tree) {
    symbol_table_fonctions *tableau_fonction = create_fonction();
    symbol_table_fonctions *current_table = tableau_fonction;
    
    
    Node *DeclFoncts = tree->firstChild->nextSibling; // Assuming DeclFoncts is the next sibling after DeclVars 
  
    //debug_print_tree_structure(tree, 0);
    if (DeclFoncts == NULL) {
      printf("No function declarations found\n");
      return NULL;
    }
  
    
    // Iterate through each function declaration
    for (Node *current = DeclFoncts->firstChild; current != NULL; current = current->nextSibling) {
        if (current->label == DeclFonct) {
            // Get the function header node
            Node *entete = current->firstChild;
            
            fonction *func = malloc(sizeof(fonction));
            //func->defvars = malloc(sizeof(symbol_table_variables));
  
            
            // Determine return type
            Node *typeNode = entete->firstChild;
            Node *test = NULL;
  
            // Gérer les types
            if (typeNode->label == TYPE_ef) {
                if (strcmp(typeNode->u.ident, "int") == 0)
                    func->id.type = Int;
                else if (strcmp(typeNode->u.ident, "char") == 0)
                    func->id.type = Char;
                else {
                    fprintf(stderr, "Erreur : type inconnu '%s'\n", typeNode->u.ident);
                    func->id.type = -1;
                }
                test = typeNode->firstChild;
              
            } else if (typeNode->label == void_ef) {
                func->id.type = Void;
                test = typeNode->firstChild;
            
            } else {
                fprintf(stderr, "Erreur : type de retour non reconnu (label %d)\n", typeNode->label);
                func->id.type = -1;
            }
  
            
            // Get function identifier
            Node *identNode = typeNode->firstChild;
            func->id.id = identNode->u.ident;
            
            // Handle parameters
            Node *parametres = entete->firstChild->nextSibling;
            symbol_table_variables *args = NULL;
            
            if (parametres->firstChild->label != void_ef) {
                // Parse parameters
                Node *param = parametres->firstChild;
                while (param != NULL) {
                    Variable arg;
                
                    if (param->label == TYPE_ef) {
                        if (strcmp(param->u.ident, "int") == 0) {
                            arg.type = Int;
                        } else if (strcmp(param->u.ident, "char") == 0) {
                            arg.type = Char;
                        } else {
                            fprintf(stderr, "Erreur : type inconnu '%s' pour le paramètre\n", param->u.ident);
                            arg.type = -1;
                        }
                        arg.id = param->firstChild->u.ident;
                        arg.static_var = false;
                      
                        // Add parameter to args list
                        symbol_table_variables *arg_node = malloc(sizeof(symbol_table_variables));
                        arg_node->var = arg;
                        arg_node->next = args;
                        args = arg_node;
                    }
                  
                    param = param->nextSibling;
                }
                    
                    // Move to next parameter
                    
                }
                func->args = args;
                
                //printf("name = %s\n retour = %d\n", func.id.id, func.id.type);
                //for (symbol_table_variables *arg = args; arg != NULL; arg = arg->next) {
                //printf("args de la fonc = %s\n", arg->var.id);
               //}
            
            
            
            
            // Handle local variables (optional)
            Node *corps = entete->nextSibling;
            
            if (corps->firstChild->label == DeclVars) {
              Node *local_vars = corps->firstChild;
              for (Node *var_node = local_vars->firstChild; var_node != NULL; var_node = var_node->nextSibling) {
                  Variable local_var_template;
                  Node *type_node = var_node;
                  local_var_template.static_var = false;
          
                  if (var_node->label == static_ef) {
                      local_var_template.static_var = true;
                      type_node = var_node->firstChild; // On saute "static" pour accéder à TYPE_ef
                  }
          
                  if (type_node->label == TYPE_ef) {
                      // Détermine le type
                      if (strcmp(type_node->u.ident, "int") == 0)
                          local_var_template.type = Int;
                      else if (strcmp(type_node->u.ident, "char") == 0)
                          local_var_template.type = Char;
                      else
                          local_var_template.type = -1;
                      
                      // Parcourir tous les identificateurs de la déclaration multiple
                      for (Node *id_node = type_node->firstChild; id_node != NULL; id_node = id_node->nextSibling) {
                          if (id_node->label == IDENT_ef) {
                              Variable local_var = local_var_template; // Copie du template
                              local_var.id = strdup(id_node->u.ident);
                              
                              // Vérification pour les variables static
                              if (local_var.static_var && id_node->nextSibling != NULL && 
                                  id_node->nextSibling->label != IDENT_ef) {
                                  fprintf(stderr, "Erreur: initialisation interdite pour la variable static '%s'\n", local_var.id);
                                  continue;
                              }
                              
                              // Ajout à la table
                              symbol_table_variables *local_var_node = malloc(sizeof(symbol_table_variables));
                              local_var_node->var = local_var;
                              local_var_node->next = func->defvars;
                              func->defvars = local_var_node;
                          }
                      }
                  }
              }
          }

            current_table->func = *func;
            // Fill current function table entry
            
            
          
            /*for (symbol_table_variables *def = defvars; def != NULL; def = def->next) {
                printf("type = %d  ", def->var.type);
                printf(" name = %s\n", def->var.id);
                printf("static = %d\n", def->var.static_var);
            }*/
          }
  
            // Create next entry if not the last function
            if (current->nextSibling != NULL) {
                current_table->next = create_fonction();
                current_table = current_table->next;
            } else {
                current_table->next = NULL;
            }
        }
        
        return tableau_fonction;
    }
  
  
  symbol_table create_symbol_table() {
    symbol_table table;
    table.variables_global = NULL;
    table.fonctions = NULL;
    return table;
  }
  
  void printsymboletable(symbol_table table) {
    printf("Table des symboles:\n");
  
    if (table.variables_global == NULL) {
        printf("Pas de variables globales\n");
    } else {
        printf("Variables globales:\n");
        for (symbol_table_variables *var = table.variables_global; var != NULL; var = var->next) {
            printf("ID: %s, Type: %d, Adresse: %d\n",
                   var->var.id ? var->var.id : "(null)",
                   var->var.type,
                   var->var.adresse);
        }
    }
  
    printf("Fonctions:\n");
    for (symbol_table_fonctions *func = table.fonctions; func != NULL; func = func->next) {
        printf("ID: %s, Type de retour: %d\n", func->func.id.id, func->func.id.type);
        for (symbol_table_variables *arg = func->func.args; arg != NULL; arg = arg->next) {
            printf("Argument ID: %s, Type: %d\n", arg->var.id, arg->var.type);
        }
  
        if (func->func.defvars == NULL) {
            printf("Pas de variables locales\n");
        } else {
            printf("Variables locales:\n");
            for (symbol_table_variables *local_var = func->func.defvars; local_var != NULL; local_var = local_var->next) {
                printf("Local Variable ID: %s, Type: %d, static = %d\n",
                       local_var->var.id, local_var->var.type, local_var->var.static_var);
            }
        }
    }
  
    printf("Fin de la table des symboles\n");
  }
  
void ajouter_fonctions_predefinies(symbol_table *table) {
      // Fonction getchar
      symbol_table_fonctions *getchar_func = malloc(sizeof(symbol_table_fonctions));
      getchar_func->func.id.id = strdup("getchar");
      getchar_func->func.id.type = Char;
      getchar_func->func.defvars = NULL;
      getchar_func->func.args = NULL;
      getchar_func->next = table->fonctions;
      table->fonctions = getchar_func;
      
      // Fonction getint
      symbol_table_fonctions *getint_func = malloc(sizeof(symbol_table_fonctions));
      getint_func->func.id.id = strdup("getint");
      getint_func->func.id.type = Int;
      getint_func->func.defvars = NULL;
      getint_func->func.args = NULL;
      getint_func->next = table->fonctions;
      table->fonctions = getint_func;
      
      // Fonction putchar
      symbol_table_fonctions *putchar_func = malloc(sizeof(symbol_table_fonctions));
      putchar_func->func.id.id = strdup("putchar");
      putchar_func->func.id.type = Void;
      putchar_func->func.defvars = NULL;
      
      // Paramètre de putchar (un caractère)
      symbol_table_variables *putchar_arg = malloc(sizeof(symbol_table_variables));
      putchar_arg->var.id = strdup("c");
      putchar_arg->var.type = Char;
      putchar_arg->var.static_var = false;
      putchar_arg->next = NULL;
      putchar_func->func.args = putchar_arg;
      
      putchar_func->next = table->fonctions;
      table->fonctions = putchar_func;
      
      // Fonction putint
      symbol_table_fonctions *putint_func = malloc(sizeof(symbol_table_fonctions));
      putint_func->func.id.id = strdup("putint");
      putint_func->func.id.type = Void;
      putint_func->func.defvars = NULL;
      
      // Paramètre de putint (un entier)
      symbol_table_variables *putint_arg = malloc(sizeof(symbol_table_variables));
      putint_arg->var.id = strdup("i");
      putint_arg->var.type = Int;
      putint_arg->var.static_var = false;
      putint_arg->next = NULL;
      putint_func->func.args = putint_arg;
      
      putint_func->next = table->fonctions;
      table->fonctions = putint_func;
  }
  
  symbol_table fill_symbol_table(Node *tree, int print) {
    symbol_table table = create_symbol_table();
    table.variables_global = remplir_var_global(tree);

    table.fonctions = remplir_fonction(tree);

    ajouter_fonctions_predefinies(&table);
    if (print == 1) {
        printsymboletable(table);
    }
    return table;
  }