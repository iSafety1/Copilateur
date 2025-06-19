%{
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yylineno;
extern char* yytext;
void yyerror(const char *s);
Node* root;
%}

%union{
  Node *node;
  char ident[64];
  int num;
  char byte;
  char comp[3];
}

%token <ident> IDENT
%token <ident> TYPE
%token <num> NUM
%token <byte> CHARACTER
%token VOID IF ELSE WHILE RETURN
%token <byte> ADDSUB
%token <byte> DIVSTAR
%token <comp> EQ
%token <comp> ORDER
%token AND OR NOT
%token STATIC  /* Extension pour variables statiques locales */
%token ERROR
%token ASSIGN

%type <node> Prog DeclVars DeclFoncts DeclFonct EnTeteFonct Parametres  Declarateurs DeclVarsLocal
%type <node> ListTypVar Corps SuiteInstr Instr  
%type <node> Exp TB FB M E T F Arguments ListExp
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE




%%
Prog:   
    DeclVars DeclFoncts { 
        root = makeNode(Prog);
        addChild(root, $1);  
        addChild(root, $2);  
        $$ = root;
    }
;


/* DeclVars pour les variables globales - pas de STATIC permis */
DeclVars:
       DeclVars TYPE Declarateurs ';'   {$$ = $1; Node *t = makeNode(TYPE_ef); strcpy(t->u.ident, $2); addChild($$, t); addChild(t, $3);}
    |                            {$$ = makeNode(GlobVar);}
    ;

DeclVarsLocal:
    DeclVarsLocal STATIC TYPE Declarateurs ';'  {
        $$ = $1;  
        Node *s = makeNode(static_ef);
        Node *t = makeNode(TYPE_ef); 
        strcpy(t->u.ident, $3);
        addChild(s, t);
        addChild(t, $4);
        addChild($$, s);  
    }
    | DeclVarsLocal TYPE Declarateurs ';'       {
        $$ = $1;  
        Node *t = makeNode(TYPE_ef);
        strcpy(t->u.ident, $2);
        addChild(t, $3);
        addChild($$, t);  
    }
    |                                     {
        $$ = makeNode(DeclVars);  /* Crée un seul nœud DeclVars initial */
    }
    ;


Declarateurs:  
       Declarateurs ',' IDENT           {$$ = $1; Node *n = makeNode(IDENT_ef); strcpy(n->u.ident, $3); addSibling($$, n);}
    |  IDENT                            {$$ = makeNode(IDENT_ef); strcpy($$->u.ident, $1);}
    ;

DeclFoncts:
       DeclFoncts DeclFonct             {$$ = $1; addChild($$, $2);}
    |  DeclFonct                        {$$ = makeNode(DeclFoncts); addChild($$, $1);}
    ;

DeclFonct:
    EnTeteFonct Corps {
        $$ = makeNode(DeclFonct);
        addChild($$, $1);
        addChild($$, $2);
    }
;

EnTeteFonct:
       TYPE IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct); Node * type = makeNode(TYPE_ef); strcpy(type->u.ident, $1); addChild($$, type); Node *n = makeNode(IDENT_ef); strcpy(n->u.ident, $2); addChild(type, n); addChild($$, $4);}
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct); Node * type = makeNode(void_ef); addChild($$, type); Node *n = makeNode(IDENT_ef); strcpy(n->u.ident, $2); addChild(type, n); addChild($$, $4);}
    ;

Parametres:
    VOID { $$ = makeNode(Parametres); addChild($$, makeNode(void_ef)); }
    | ListTypVar { $$ = makeNode(Parametres); addChild($$, $1); }
;

ListTypVar:
       ListTypVar ',' TYPE IDENT        {$$ = $1; Node *type = makeNode(TYPE_ef); addSibling($$, type); strcpy(type->u.ident, $3); Node *n = makeNode(IDENT_ef); strcpy(n->u.ident, $4); addChild(type, n);}
    |  TYPE IDENT                       {$$ = makeNode(TYPE_ef); strcpy($$->u.ident, $1); Node *n = makeNode(IDENT_ef); strcpy(n->u.ident, $2); addChild($$, n);}
    ;



Corps: 
    '{' DeclVarsLocal SuiteInstr '}'    {
        $$ = makeNode(Corps);
        addChild($$, $2);
        addChild($$, $3);
    }
    ;


SuiteInstr:                             
       SuiteInstr Instr                 {if($1){$$ = $1; addSibling($$, $2);}else{$$=$2;}}
    |                             {$$ = NULL;}
    ;

Instr:
    IDENT '(' Arguments ')' ';' {
        $$ = makeNode(FunctionCall);
        strcpy($$->u.ident, $1);
        addChild($$, $3);
    }
    | IDENT '=' Exp ';' {
        $$ = makeNode(ASSIGN_ef);
        strcpy($$->u.ident, "=");
        Node* identNode = makeNode(IDENT_ef);
        strcpy(identNode->u.ident, $1);
        addChild($$, identNode);
        addChild($$, $3);
    }
    | IF '(' Exp ')' Instr %prec LOWER_THAN_ELSE {
        $$ = makeNode(if_ef);
        addChild($$, $3);
        addChild($$, $5);
    }
    | IF '(' Exp ')' Instr ELSE Instr {
        $$ = makeNode(if_ef);
        addChild($$, $3);
        addChild($$, $5);
        Node* elseNode = makeNode(else_ef);
        addSibling($5, elseNode);
        addChild(elseNode, $7);
    }
    | WHILE '(' Exp ')' Instr {
        $$ = makeNode(while_ef);
        addChild($$, $3);
        addChild($$, $5);
    }
    | RETURN Exp ';' {
        $$ = makeNode(return_ef);
        addChild($$, $2);
    }
    | RETURN ';' {
        $$ = makeNode(return_ef);
    }
    | '{' SuiteInstr '}' {
        $$ = $2;
    }
    | ';' {
        $$ = NULL;
    }
;

Exp:
    Exp OR TB {
        $$ = makeNode(OR_ef);
        addChild($$, $1);
        addChild($$, $3);
    }
    | TB { $$ = $1; }
;

TB:
    TB AND FB {
        $$ = makeNode(AND_ef);
        addChild($$, $1);
        addChild($$, $3);
    }
    | FB { $$ = $1; }
;

FB:
      FB EQ M {
        $$ = makeNode(EQ_ef);
        strcpy($$->u.comp, $2);  // Utilisation de strcpy au lieu de strcmp
        addChild($$, $1);
        addChild($$, $3);
    }
    | M { $$ = $1; }
;
;

M   :  M ORDER E                            {$$ = makeNode(ORDER_ef); strcpy($$->u.comp, $2); addChild($$, $1); addChild($$, $3);}
    |  E                                    {$$ = $1;}
    ;

E   :  E ADDSUB T                           {$$ = makeNode(ADDSUB_ef); $$->u.byte = $2; addChild($$, $1); addChild($$, $3);}
    |  T                                    {$$ = $1;}
    ;    
T   :  T DIVSTAR F                          {$$ = makeNode(DIVSTAR_ef); $$->u.byte = $2; addChild($$, $1); addChild($$, $3);}
    |  F                                    {$$ = $1;}
    ;

F   :  ADDSUB F                             {$$ = makeNode(ADDSUB_ef); $$->u.byte = $1; addChild($$, $2);}
    |  '!' F                                {$$ = makeNode(NOT_ef); addChild($$, $2);}
    |  '(' Exp ')'                          {$$ = $2;}
    |  NUM                                  {$$ = makeNode(DIGIT_ef); $$->u.num = $1;}
    |  CHARACTER                            {$$ = makeNode(CHARACTER_ef); $$->u.byte = $1;}
    |  IDENT                                {$$ = makeNode(IDENT_ef); strcpy($$->u.ident, $1);}
    | IDENT '(' Arguments ')' {
        $$ = makeNode(FunctionCall);
        $$->lineno = yylineno;  // Utilisation de yylineno pour le numéro de ligne
        strcpy($$->u.ident, $1);
        addChild($$, $3);
    }

Arguments:
    ListExp { $$ = $1; }
    |  { $$ = NULL; }
;

ListExp:
       ListExp ',' Exp      {$$ = $1; addSibling($$, $3);}      
    |  Exp                  {$$ = $1;}
    ;   

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erreur syntaxique ligne %d: %s\n", yylineno, s);
}

void print_help(){

    printf("=================================\n");
    printf("===========HELP GUIDE============\n");
    printf("=================================\n");
    printf("Command Line: ./tpcas [OPTION]\nWith OPTION equal to -h/--help or -t/--tree\n");
    printf("OPTION 1 : -h/--help , Print usage of the program.\n");
    printf("OPTION 2 : -t/--tree , Print the abstract tree on the standard output\n");
    printf("OPTION 3 : -s/--symboletable , Print the symbol table on the standard output\n");
    printf("(You can use both options)\n");
}



int main(int argc, char **argv) {
    int show_tree = 0;
    int print=0;
    FILE *fichier = fopen("_anonymous.asm", "w");
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tree") == 0) {
            show_tree = 1;
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            fclose(fichier);
            return 0;
        }
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symboletable") == 0) {
            print = 1;
        }
    }

    if (yyparse() == 0) {
        if (show_tree) {
            printTree(root);
        }
        
        symbol_table table = fill_symbol_table(root,print);
        int sem_result = verifier_semantique(table, root);
        
        if (sem_result != 0) {
            fprintf(stderr, "Erreur sémantique détectée\n");
            fclose(fichier);
            exit(2);  // Code d'erreur sémantique
        }
        
        genere_code_nasm(table, fichier);
        fclose(fichier);
        return 0;  // Réussite
    }
    
    // Erreur syntaxique
    fclose(fichier);
    return 1;  // Code d'erreur syntaxique
}
