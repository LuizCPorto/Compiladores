%{
#include "../parser/parser.tab.h"
#include "../ast/ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int _lex_iniciado = 0;

static void lex_cabecalho() {
    if (!_lex_iniciado) {
        _lex_iniciado = 1;
        printf("\n=== ANALISE LEXICA (Tokens Identificados) ===\n");
        printf("%-15s %-20s %s\n", "CATEGORIA",   "TOKEN",  "LEXEMA");
        printf("%-15s %-20s %s\n", "---------",   "-----",  "------");
    }
}

static void lex_print(const char* categoria, const char* token, const char* lexema) {
    lex_cabecalho();
    printf("%-15s %-20s %s\n", categoria, token, lexema);
}
%}

%%

"//"[^\n]*                          { /* ignora comentario de linha */ }
"/*"([^*]|"*"+[^*/])*"*"+"/"       { /* ignora comentario de bloco */ }

"int"               { lex_print("PALAVRA-CHAVE", "T_INT",    yytext); return T_INT; }
"float"             { lex_print("PALAVRA-CHAVE", "T_FLOAT",  yytext); return T_FLOAT; }
"return"            { lex_print("PALAVRA-CHAVE", "T_RETURN", yytext); return T_RETURN; }
"main"              { lex_print("PALAVRA-CHAVE", "T_MAIN",   yytext); return T_MAIN; }

[0-9]+"."[0-9]+     { lex_print("NUMERO_FLOAT",  "T_NUMERO_FLOAT", yytext); yylval.valorFloat   = atof(yytext); return T_NUMERO_FLOAT; }
[0-9]+              { lex_print("NUMERO_INT",     "T_NUMERO",       yytext); yylval.valorInteiro = atoi(yytext); return T_NUMERO; }

[a-zA-Z_][a-zA-Z0-9_]*  { lex_print("IDENTIFICADOR", "T_ID", yytext); yylval.texto = strdup(yytext); return T_ID; }

"="                 { lex_print("OPERADOR", "T_ATRIB",  yytext); return T_ATRIB; }
"+"                 { lex_print("OPERADOR", "T_MAIS",         yytext); return T_MAIS; }
"-"                 { lex_print("OPERADOR", "T_MENOS",        yytext); return T_MENOS; }
"*"                 { lex_print("OPERADOR", "T_MULT",         yytext); return T_MULT; }
"/"                 { lex_print("OPERADOR", "T_DIV",          yytext); return T_DIV; }
";"                 { lex_print("DELIMITADOR", "T_PONTOVIRGULA", yytext); return T_PONTOVIRGULA; }
","                 { lex_print("DELIMITADOR", "T_VIRGULA",      yytext); return T_VIRGULA; }
"("                 { lex_print("DELIMITADOR", "T_ABRE_PAREN",   yytext); return T_ABRE_PAREN; }
")"                 { lex_print("DELIMITADOR", "T_FECHA_PAREN",  yytext); return T_FECHA_PAREN; }
"{"                 { lex_print("DELIMITADOR", "T_ABRE_CHAVE",   yytext); return T_ABRE_CHAVE; }
"}"                 { lex_print("DELIMITADOR", "T_FECHA_CHAVE",  yytext); return T_FECHA_CHAVE; }

[ \t\n]             { /* ignora espacos, tabs e quebras de linha */ }
.                   { printf("Erro lexico: caractere '%s' nao reconhecido\n", yytext); }

%%

int yywrap() {
    if (_lex_iniciado)
        printf("----------------------------------------------\n");
    return 1;
}
