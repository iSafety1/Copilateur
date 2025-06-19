#include "tree.h"
extern int lineno;  
// Static array to convert label enum values to strings for printing


Node *makeNode(label_t label) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
      printf("Run out of memory\n");
      exit(1);
    }
    node->label = label;
    node-> firstChild = node->nextSibling = NULL;
    node->lineno=lineno;
    return node;
  }
  
  void addSibling(Node *node, Node *sibling) {
    Node *curr = node;
    while (curr->nextSibling != NULL) {
      curr = curr->nextSibling;
    }
    curr->nextSibling = sibling;
  }
  
  void addChild(Node *parent, Node *child) {
    if (parent->firstChild == NULL) {
      parent->firstChild = child;
    }
    else {
      addSibling(parent->firstChild, child);
    }
  }
  
  void deleteTree(Node *node) {
    if (node->firstChild) {
      deleteTree(node->firstChild);
    }
    if (node->nextSibling) {
      deleteTree(node->nextSibling);
    }
    free(node);
  }
  
  void printTree(Node *node) {
    static bool rightmost[128]; // tells if node is rightmost sibling
    static int depth = 0;       // depth of current node
    for (int i = 1; i < depth; i++) { // 2502 = vertical line
      printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
      printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    }
    
  
    switch (node->label) {
      case IDENT_ef:
        printf("%s", node->u.ident);
        break;
      case TYPE_ef:
        printf("%s", node->u.ident);
        break;
      case DIGIT_ef:
        printf("%d", node->u.num);
        break;
      case CHARACTER_ef:
        printf("%c", node->u.byte);
        break;
      case ADDSUB_ef:
        printf("%s", node->u.comp);
        break;
      case DIVSTAR_ef:
        printf("%s", node->u.comp);
        break;
      case EQ_ef:
        printf("%s", node->u.comp);
        break;
      case ORDER_ef:
        printf("%s", node->u.comp);
        break;
      case NOT_ef:
        printf("%s", node->u.comp);
        break;
      case AND_ef:
        printf("%s", node->u.comp);
        break;
      case OR_ef:
        printf("%s", node->u.comp);
        break;
      case ASSIGN_ef:
        printf("%s", node->u.comp);
        break;
      default:
          printf("%s", StringFromLabel[node->label]);
        break;
    }
  
    printf("\n");
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
      rightmost[depth] = (child->nextSibling) ? false : true;
      printTree(child);
    }
    depth--;
  }
  
  int est_main_arbre(Node *node){
    int a;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
      if (strcmp(child->u.ident, "main") == 0) {
        return 0;
      }
      a=est_main_arbre(child);
      if (a==0){
        return 0;
      }
  
    }
    return 1;
  }
  
  