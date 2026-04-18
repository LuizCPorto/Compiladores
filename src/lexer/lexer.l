%{
#include "../parser/parser.tab.h" 
#include <stdlib.h>
%}

%%
"INT"       { return T_INT; }
[0-9]+      { return T_NUMERO; }
[a-zA-Z]+   { return T_ID; }
"="         { return T_ATRIB; }
";"         { return T_PONTOVIRGULA; }
[ \t\n]     { /* ignora espacos */ }
.           { printf("Erro lexico\n"); }
%%

int yywrap() { return 1; }