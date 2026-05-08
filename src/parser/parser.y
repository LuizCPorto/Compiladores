%{
#include <iostream>
#include <string>
#include <cstdlib>

// Importamos as classes que vocês criaram
#include "../ast/ast.h"
#include "../semantica/tabela_simbolos.h" 

extern int yylex();
void yyerror(const char *s);

// Instanciamos a memória do compilador globalmente
TabelaSimbolos tabela;
%}

/* O %union é a mochila. Ele define os tipos de dados que os tokens 
  podem carregar do Lexer para o Parser.
*/
%union {
    int valorInteiro;
    char* texto;
    No* ast_no;
}

/* Agora dizemos qual token carrega qual tipo de dado */
%token <texto> T_ID
%token <valorInteiro> T_NUMERO
%token T_INT T_ATRIB T_PONTOVIRGULA

/* Dizemos que a regra 'declaracao' vai retornar um objeto da AST */
%type <ast_no> declaracao

%%

programa:
    declaracao {
        std::cout << "Compilacao e Analise Semantica finalizadas com sucesso!\n";
    }
    ;

declaracao:
    T_INT T_ID T_ATRIB T_NUMERO T_PONTOVIRGULA {
        
        std::string nomeVariavel = $2; // Pega o texto do T_ID
        
        // ==========================================
        // 1. ANÁLISE SEMÂNTICA (Tabela de Símbolos)
        // ==========================================
        if (!tabela.inserir(nomeVariavel, "INT")) {
            yyerror("Erro Semantico: Variavel ja declarada anteriormente!");
            YYABORT; // Aborta a compilação imediatamente
        }

        // ==========================================
        // 2. CONSTRUÇÃO DA AST (Árvore)
        // ==========================================
        // Cria um nó de operação '=' recebendo um número
        $$ = new NoOperacao("=", new NoNumero($4), nullptr); 
        
        std::cout << "Acao Semantica: '" << nomeVariavel << "' guardada na memoria.\n";
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