%{
#include "../parser/parser.tab.h"
#include "../ast/ast.h"
#include <stdlib.h>
#include <string.h>
%}

%%

"//"[^\n]*                          { /* ignora comentario de linha */ }
"/*"([^*]|"*"+[^*/])*"*"+"/"       { /* ignora comentario de bloco */ }

"int"               { return T_INT; }
"float"             { return T_FLOAT; }
"if"                { return T_IF; }
"else"              { return T_ELSE; }
"while"             { return T_WHILE; }
"return"            { return T_RETURN; }
"void"              { return T_VOID; }
"main"              { return T_MAIN; }

[0-9]+"."[0-9]+     { yylval.valorFloat = atof(yytext); return T_NUMERO_FLOAT; }
[0-9]+              { yylval.valorInteiro = atoi(yytext); return T_NUMERO; }

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.texto = strdup(yytext); return T_ID; }

"<="                { return T_MENOR_IGUAL; }
">="                { return T_MAIOR_IGUAL; }
"=="                { return T_IGUAL_IGUAL; }
"!="                { return T_DIFERENTE; }
"<"                 { return T_MENOR; }
">"                 { return T_MAIOR; }
"="                 { return T_ATRIB; }
"+"                 { return T_MAIS; }
"-"                 { return T_MENOS; }
"*"                 { return T_MULT; }
"/"                 { return T_DIV; }
";"                 { return T_PONTOVIRGULA; }
","                 { return T_VIRGULA; }
"("                 { return T_ABRE_PAREN; }
")"                 { return T_FECHA_PAREN; }
"{"                 { return T_ABRE_CHAVE; }
"}"                 { return T_FECHA_CHAVE; }

[ \t\n]             { /* ignora espacos, tabs e quebras de linha */ }
.                   { printf("Erro lexico: caractere '%s' nao reconhecido\n", yytext); }

%%

int yywrap() { return 1; }
