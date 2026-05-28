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
std::string escopoAtual    = "global";
std::string funcaoTipoAtual;
std::string funcaoNomeAtual;
%}

%union {
    int   valorInteiro;
    float valorFloat;
    char* texto;
    No*   ast_no;
}

%token <texto>        T_ID
%token <valorInteiro> T_NUMERO
%token <valorFloat>   T_NUMERO_FLOAT
%token T_INT T_FLOAT T_IF T_ELSE T_WHILE T_RETURN T_VOID T_MAIN
%token T_ATRIB T_PONTOVIRGULA T_VIRGULA
%token T_ABRE_PAREN T_FECHA_PAREN T_ABRE_CHAVE T_FECHA_CHAVE
%token T_MAIS T_MENOS T_MULT T_DIV
%token T_MENOR T_MAIOR T_MENOR_IGUAL T_MAIOR_IGUAL T_IGUAL_IGUAL T_DIFERENTE

%type <ast_no> lista_comandos comando bloco funcao
%type <ast_no> declaracao atribuicao retorno
%type <ast_no> expressao termo fator
%type <texto>  tipo nome_funcao

%start programa

%%

/* -------------------------------------------------
   programa: constroi a AST completa, depois imprime
   e gera o codigo intermediario de uma so vez
   ------------------------------------------------- */
programa:
    lista_comandos {
        NoBloco* raiz = static_cast<NoBloco*>($1);

        std::cout << "\n=== ARVORE SINTATICA ABSTRATA (AST) ===\n";
        raiz->imprimir();

        std::cout << "\n=== CODIGO INTERMEDIARIO (3 enderecos) ===\n";
        raiz->gerarCodigo();

        std::cout << "\nCompilacao e Analise Semantica finalizadas com sucesso!\n";
        tabela.listarTodos();
        delete raiz;
    }
    ;

/* -------------------------------------------------
   lista_comandos: acumula comandos em um NoBloco
   ------------------------------------------------- */
lista_comandos:
    lista_comandos comando {
        static_cast<NoBloco*>($1)->adicionar($2);
        $$ = $1;
    }
    | comando {
        NoBloco* b = new NoBloco();
        b->adicionar($1);
        $$ = b;
    }
    ;

/* -------------------------------------------------
   comando: repassa o no criado pela regra filha
   ------------------------------------------------- */
comando:
    declaracao  { $$ = $1; }
    | atribuicao { $$ = $1; }
    | retorno    { $$ = $1; }
    | funcao     { $$ = $1; }
    ;

/* -------------------------------------------------
   tipo: int ou float  →  retorna string alocada
   ------------------------------------------------- */
tipo:
    T_INT   { $$ = strdup("int"); }
    | T_FLOAT { $$ = strdup("float"); }
    ;

/* -------------------------------------------------
   declaracao:
     int x;
     float y = 3.14;
     int z = a + b;
   ------------------------------------------------- */
declaracao:
    tipo T_ID T_PONTOVIRGULA {
        tabela.inserirIdentificador($2, $1, escopoAtual);
        $$ = new NoDeclaracao(std::string($1), std::string($2), nullptr);
        free($1); free($2);
    }
    | tipo T_ID T_ATRIB expressao T_PONTOVIRGULA {
        tabela.inserirIdentificador($2, $1, escopoAtual);
        $$ = new NoDeclaracao(std::string($1), std::string($2), $4);
        free($1); free($2);
    }
    ;

/* -------------------------------------------------
   atribuicao:
     x = 10;
     x = a + b;
     x = (a + b) * 2;
   ------------------------------------------------- */
atribuicao:
    T_ID T_ATRIB expressao T_PONTOVIRGULA {
        if (!tabela.existe($1, escopoAtual))
            std::cerr << "[ERRO SEMANTICO] Variavel '" << $1 << "' nao declarada!\n";
        $$ = new NoAtribuicao(std::string($1), $3);
        free($1);
    }
    ;

/* -------------------------------------------------
   retorno:
     return r;
     return a + b;
   ------------------------------------------------- */
retorno:
    T_RETURN expressao T_PONTOVIRGULA {
        $$ = new NoRetorno($2);
    }
    ;

/* -------------------------------------------------
   nome_funcao: identificador comum ou palavra "main"
   ------------------------------------------------- */
nome_funcao:
    T_ID    { $$ = $1; }
    | T_MAIN { $$ = strdup("main"); }
    ;

/* -------------------------------------------------
   lista_parametros / parametros:
     (int a, int b)   ou   ()
   ------------------------------------------------- */
lista_parametros:
    lista_parametros T_VIRGULA tipo T_ID {
        tabela.inserirIdentificador($4, $3, escopoAtual);
        free($3); free($4);
    }
    | tipo T_ID {
        tabela.inserirIdentificador($2, $1, escopoAtual);
        free($1); free($2);
    }
    ;

parametros:
    lista_parametros
    | %empty
    ;

/* -------------------------------------------------
   bloco: { lista_comandos }  ou  {}
   ------------------------------------------------- */
bloco:
    T_ABRE_CHAVE lista_comandos T_FECHA_CHAVE { $$ = $2; }
    | T_ABRE_CHAVE T_FECHA_CHAVE              { $$ = new NoBloco(); }
    ;

/* -------------------------------------------------
   inicio_funcao: salva tipo/nome antes de processar
   parametros (que precisam do escopo correto)
   ------------------------------------------------- */
inicio_funcao:
    tipo nome_funcao T_ABRE_PAREN {
        funcaoTipoAtual = $1;
        funcaoNomeAtual = $2;
        escopoAtual     = $2;
        free($1); free($2);
    }
    ;

/* -------------------------------------------------
   funcao:
     int soma(int a, int b) { ... }
     void main() { ... }
   ------------------------------------------------- */
funcao:
    inicio_funcao parametros T_FECHA_PAREN bloco {
        NoBloco* corpo = static_cast<NoBloco*>($4);
        $$ = new NoFuncao(funcaoTipoAtual, funcaoNomeAtual, corpo);
        escopoAtual = "global";
    }
    ;

/* -------------------------------------------------
   expressao: + e - (menor precedencia)
   ------------------------------------------------- */
expressao:
    expressao T_MAIS  termo { $$ = new NoOperacaoBinaria("+", $1, $3); }
    | expressao T_MENOS termo { $$ = new NoOperacaoBinaria("-", $1, $3); }
    | termo                 { $$ = $1; }
    ;

/* -------------------------------------------------
   termo: * e / (maior precedencia)
   ------------------------------------------------- */
termo:
    termo T_MULT fator { $$ = new NoOperacaoBinaria("*", $1, $3); }
    | termo T_DIV fator  { $$ = new NoOperacaoBinaria("/", $1, $3); }
    | fator              { $$ = $1; }
    ;

/* -------------------------------------------------
   fator: unidade atomica de uma expressao
   ------------------------------------------------- */
fator:
    T_NUMERO                               { $$ = new NoNumero($1); }
    | T_NUMERO_FLOAT                       { $$ = new NoFloat($1); }
    | T_ID                                 { $$ = new NoIdentificador(std::string($1)); free($1); }
    | T_ABRE_PAREN expressao T_FECHA_PAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    std::cerr << "Erro de sintaxe: " << s << "\n";
}

int main() {
    yyparse();
    return 0;
}
