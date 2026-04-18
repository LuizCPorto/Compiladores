%{
#include <stdio.h>
#include <stdlib.h>

/* ISSO RESOLVE O ERRO 1: Avisa ao GCC que a funcao do Lexer existe */
extern int yylex();
void yyerror(const char *s);
%}

/* ISSO RESOLVE O ERRO 2: Cria a lista oficial de tokens para o Lexer usar */
%token T_INT T_ID T_NUMERO T_ATRIB T_PONTOVIRGULA

%%

programa:
    declaracao
    ;

declaracao:
    T_INT T_ID T_ATRIB T_NUMERO T_PONTOVIRGULA {
        printf("SUCESSO: A declaracao da variavel foi compreendida pelo Parser!\n");
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error, entrada não reconhecida: %s\n", s);
}

int main() {
    yyparse();
    return 0;
}