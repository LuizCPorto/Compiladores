%{
#include <stdio.h>
%}

%%
[0-9]+      { printf("NUMERO: %s\n", yytext); }
[a-zA-Z]+   { printf("PALAVRA: %s\n", yytext); }
[ \t\n]     { /* ignora espaços em branco */ }
.           { printf("CARACTERE INVALIDO: %s\n", yytext); }
%%

int main() {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}