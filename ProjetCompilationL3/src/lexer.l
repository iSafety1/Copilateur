%{
#include "codegen.h"
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"

int lineno = 1;
int linecharno = 1;
%}

%option noyywrap
%option yylineno
%option noinput
%option nounput
%x COMM 

/* Définitions des règles lexicales */


%%
"/*"        { BEGIN COMM;};
"//".*      ; 

"if"        {strcpy(yylval.ident, yytext); linecharno = linecharno + yyleng; return IF;}
"else"      {strcpy(yylval.ident, yytext); linecharno = linecharno + yyleng; return ELSE;}
"while"     {strcpy(yylval.ident, yytext); linecharno = linecharno + yyleng; return WHILE;}
"return"    {linecharno = linecharno + yyleng; return RETURN;}
"void"      {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return VOID;}
"int"       {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return TYPE;}
"char"      { strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return TYPE;}
"static"    { return STATIC; }
\'(\\.|[^\\'])\'         {  yylval.byte = yytext[1]; return CHARACTER; }
"+"         { strcpy(yylval.comp,yytext);return ADDSUB; }
"-"         { strcpy(yylval.comp,yytext);return ADDSUB; }
"*"         { strcpy(yylval.comp,yytext);return DIVSTAR; }
"/"         { strcpy(yylval.comp,yytext);return DIVSTAR; }
"%"         { strcpy(yylval.comp,yytext);return DIVSTAR; }
"=="        { strcpy(yylval.comp,yytext);return EQ; }
"!="        {strcpy(yylval.comp,yytext); return EQ; }
"<"         { strcpy(yylval.comp,yytext);return ORDER; }
"<="        { strcpy(yylval.comp,yytext);return ORDER; }
">"         {strcpy(yylval.comp,yytext); return ORDER; }
">="        {strcpy(yylval.comp,yytext); return ORDER; }
"!"         {strcpy(yylval.comp,yytext); return '!'; }
"&&"        { strcpy(yylval.comp,yytext);return AND; }
"||"        {strcpy(yylval.comp,yytext); return OR; }
"="         {strcpy(yylval.comp,yytext);return '='; }
";"         { return ';'; }
","         { return ','; }
"("         { return '('; }
")"         { return ')'; }
"{"         { return '{'; }
"}"         { return '}'; }





[0-9]+  	{yylval.num = atoi(yytext); linecharno = linecharno + yyleng; return NUM;}
[a-zA-Z_][a-zA-Z0-9_]*            {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return IDENT;}

<COMM>"*/"  { BEGIN(INITIAL); }  // Fin du commentaire 
<COMM>.     ;                    // Ignore tous dans les commentaires
<COMM>\n    { lineno++; } // Compte les nouvelles lignes dans les commentaires 

[ \t]+                       { /* Ignore les espaces */ }


\n                        { lineno++; linecharno = 1; }

.                       { printf("Caractère non reconnu : %s\n", yytext); return 1; }

%%


