%code requires {
    #include "../ast/ast.h"
}

%{
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "../ast/ast.h"
#include "../semantica/tabela_simbolos.h"

extern int yylex();
void yyerror(const char *s);

// tabela definida em main_ambiente.cpp
extern TabelaSimbolos tabela;

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
%token T_INT T_FLOAT T_RETURN T_MAIN
%token T_ATRIB T_PONTOVIRGULA T_VIRGULA
%token T_ABRE_PAREN T_FECHA_PAREN T_ABRE_CHAVE T_FECHA_CHAVE
%token T_MAIS T_MENOS T_MULT T_DIV

%type <ast_no> lista_comandos comando bloco funcao
%type <ast_no> declaracao atribuicao retorno
%type <ast_no> expressao termo fator
%type <texto>  tipo nome_funcao

%start programa

%%

/* -------------------------------------------------
   programa: constroi a AST completa, imprime e
   gera o codigo intermediario
   ------------------------------------------------- */
programa:
    lista_comandos {
        NoBloco* raiz = static_cast<NoBloco*>($1);

        std::cout << "\n=== ARVORE SINTATICA ABSTRATA (AST) ===\n";
        raiz->imprimir();

        std::cout << "\n=== CODIGO INTERMEDIARIO (3 enderecos) ===\n";
        raiz->gerarCodigo();

        // --- Salva expressao de cada comando antes de otimizar ---
        std::vector<std::string> antes;
        for (auto* c : raiz->comandos)
            antes.push_back(c->paraExpressao());

        std::cout << "\n=== OTIMIZACAO INDEPENDENTE DE MAQUINA ===\n";
        std::cout << "(Simplificacao algebrica | Dobramento de constantes | Eliminacao de codigo morto)\n\n";

        raiz->otimizar();

        // --- Mostra before/after por comando que mudou ---
        bool alguma = false;
        for (size_t i = 0; i < std::min(antes.size(), raiz->comandos.size()); ++i) {
            std::string dep = raiz->comandos[i]->paraExpressao();
            if (antes[i] != dep) {
                if (!alguma) { std::cout << "\nTransformacoes por instrucao:\n"; alguma = true; }
                std::cout << "  Antes:  " << antes[i] << "\n";
                std::cout << "  Depois: " << dep << "\n\n";
            }
        }
        if (!alguma) std::cout << "Nenhuma otimizacao aplicavel encontrada.\n";

        std::cout << "\n=== CODIGO INTERMEDIARIO OTIMIZADO ===\n";
        resetarContadorAST();
        raiz->gerarCodigo();

        // ---- BACK-END: GERADOR DE CODIGO ASSEMBLY x86-64 ----
        {
            std::ostringstream asm_buf;
            asm_buf << "# Gerado pelo Compilador SimpleC\n";
            asm_buf << "# Sintaxe: AT&T/GAS  |  Target: x86-64 Linux\n";
            asm_buf << "# Compilar: gcc -no-pie saida.asm -o programa\n\n";

            inicializarGeradorAssembly(asm_buf, tabela);
            raiz->gerarAssembly();

            std::string codigo_asm = asm_buf.str();

            std::cout << "\n=== CODIGO DE MAQUINA (Assembly x86-64) ===\n";
            std::cout << codigo_asm;

            std::ofstream arq("saida.asm");
            if (arq) {
                arq << codigo_asm;
                arq.close();
                std::cout << "\n[INFO] Assembly salvo em: saida.asm\n";
            }
        }

        std::cout << "\nCompilacao finalizada com sucesso!\n";
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
   comando
   ------------------------------------------------- */
comando:
    declaracao  { $$ = $1; }
    | atribuicao { $$ = $1; }
    | retorno    { $$ = $1; }
    | funcao     { $$ = $1; }
    ;

/* -------------------------------------------------
   tipo: int ou float
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
        // Captura valor inicial quando for literal simples
        if (NoNumero* n = dynamic_cast<NoNumero*>($4))
            tabela.atualizarValor($2, escopoAtual, n->valor);
        else if (NoFloat* f = dynamic_cast<NoFloat*>($4))
            tabela.atualizarValor($2, escopoAtual, (int)f->valor);
        $$ = new NoDeclaracao(std::string($1), std::string($2), $4);
        free($1); free($2);
    }
    ;

/* -------------------------------------------------
   atribuicao:
     x = 10;
     x = a + b;
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
   ------------------------------------------------- */
retorno:
    T_RETURN expressao T_PONTOVIRGULA {
        $$ = new NoRetorno($2);
    }
    ;

/* -------------------------------------------------
   nome_funcao: identificador ou main
   ------------------------------------------------- */
nome_funcao:
    T_ID    { $$ = $1; }
    | T_MAIN { $$ = strdup("main"); }
    ;

/* -------------------------------------------------
   lista_parametros / parametros
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
   inicio_funcao: salva tipo/nome e muda escopo
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
   fator: unidade atomica
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
