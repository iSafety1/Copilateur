#ifndef TREE_H
#define TREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *StringFromLabel[] = {
    "Prog", "DeclVars", "GlobVar", "Declarateurs", "DeclFoncts", "DeclFonct",
    "FunctionCall", "EnTeteFonct", "Parametres", "ListTypVar", "Corps", 
    "SuiteInstr", "Instr", "Exp", "TB", "FB", "M", "E", "T", "F",  
    "Arguments", "ListExp", "if", "else", "while", "return", "void", 
    "int", "char", "static", "Ident", "Type", "Digit", "Character", 
    "Addsub", "Divstar", "Eq", "ORDER", "NOT", "AND", "OR", "ASSIGN"
};

// Enum to represent various node types (labels)
typedef enum {
    Prog,
    DeclVars,
    GlobVar,
    Declarateurs,
    DeclFoncts,
    DeclFonct,
    FunctionCall,
    EnTeteFonct,
    Parametres,
    ListTypVar,
    Corps,
    SuiteInstr,
    Instr,
    Exp,
    TB,
    FB,
    M,
    E,
    T,
    F,
    Arguments,
    ListExp,
    if_ef,
    else_ef,
    while_ef,
    return_ef,
    void_ef,
    int_ef,
    char_ef,
    static_ef,
    IDENT_ef,
    TYPE_ef,
    DIGIT_ef,
    CHARACTER_ef,
    ADDSUB_ef,
    DIVSTAR_ef,
    EQ_ef,
    ORDER_ef,
    NOT_ef,
    AND_ef,
    OR_ef,
    ASSIGN_ef,
    // Additional node labels can go here
} label_t;

// Struct for tree nodes
typedef struct Node {
    label_t label;             // Node's type (label)
    struct Node *firstChild;   // Pointer to the first child node
    struct Node *nextSibling;  // Pointer to the next sibling node
    int lineno;                // Line number in the source code
    union {
        char ident[64];   // For identifiers (variable names, function names)
        int num;          // For integers (numeric literals)
        char byte;        // For characters (character literals)
        char comp[3];     // For comparison operators (e.g., "==", "<")
    } u;
} Node;

// Function declarations
Node *makeNode(label_t label);                      // Create a new node
void addSibling(Node *node, Node *sibling);         // Add a sibling node
void addChild(Node *parent, Node *child);           // Add a child node
void deleteTree(Node *node);                        // Delete the entire tree
void printTree(Node *node);                         // Print the tree structure

// Helper macros to access children
#define FIRSTCHILD(node) (node)->firstChild
#define SECONDCHILD(node) (node)->firstChild->nextSibling
#define THIRDCHILD(node) (node)->firstChild->nextSibling->nextSibling

#endif // TREE_H
