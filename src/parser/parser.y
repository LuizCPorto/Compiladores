%code requires {
    #include "../ast/ast.h"
}

%{
#include <iostream>
#include <string>
#include <cstdlib>
#include "../ast/ast.h"
#include "../semantica/tabela_simbolos.h" 

extern int yylex();
void yyerror(const char *s);

TabelaSimbolos tabela;
%}

%union {
    int valorInteiro;
    char* texto;
    No* ast_no;
}

%token <texto> T_ID
%token <valorInteiro> T_NUMERO
%token T_INT T_ATRIB T_PONTOVIRGULA

%type <ast_no> declaracao

%%

programa:
    declaracao {
        std::cout << "Compilacao e Analise Semantica finalizadas com sucesso!\n";
    }
    ;

declaracao:
    T_INT T_ID T_ATRIB T_NUMERO T_PONTOVIRGULA {
        std::string nomeVariavel = $2;
        
        tabela.inserirIdentificador(nomeVariavel, "INT");

        $$ = new NoOperacao("=", new NoOperacao("+", new NoNumero(10), new NoNumero(5)), nullptr); 
        
        std::cout << "\n--- CODIGO INTERMEDIARIO GERADO ---\n";
        $$->gerarCodigo();
        std::cout << "-----------------------------------\n";
        delete $$;
        $$= nullptr;  
        free($2);
        $2 = nullptr;
    }
    ;

%%

void yyerror(const char *s) {
    std::cerr << s << "\n";
}

int main() {
    yyparse();
    return 0;
}