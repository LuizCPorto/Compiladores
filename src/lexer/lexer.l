%{
#include "../parser/parser.tab.h"
#include <stdlib.h>
#include <string.h> /* Adicionado para podermos usar o strdup e copiar o texto */
%}

%%
"INT"       { return T_INT; }

[0-9]+      { 
    /* Converte o texto para número e guarda na gaveta 'valorInteiro' da mochila */
    yylval.valorInteiro = atoi(yytext); 
    return T_NUMERO; 
}

[a-zA-Z]+   { 
    /* Copia o texto lido e guarda na gaveta 'texto' da mochila */
    yylval.texto = strdup(yytext); 
    return T_ID; 
}

"="         { return T_ATRIB; }
";"         { return T_PONTOVIRGULA; }
[ \t\n]     { /* ignora espacos */ }
.           { printf("Erro lexico\n"); }
%%

int yywrap() { return 1; }