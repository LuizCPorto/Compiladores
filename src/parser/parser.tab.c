#include "parser.tab.h"

#include "../codigo_final/gerador_codigo_final.h"
#include "../intermediario/codigo_intermediario.h"
#include "../otimizacao/otimizador.h"
#include "../semantica/tabela_simbolos.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

extern int yylex();

YYSTYPE yylval;

static std::string nomeToken(int token) {
    switch (token) {
    case YYEOF: return "fim do arquivo";
    case T_ID: return "identificador";
    case T_NUMERO: return "numero inteiro";
    case T_NUMERO_FLOAT: return "numero float";
    case T_INT: return "int";
    case T_FLOAT: return "float";
    case T_VOID: return "void";
    case T_MAIN: return "main";
    case T_RETURN: return "return";
    case T_ATRIB: return "=";
    case T_PONTOVIRGULA: return ";";
    case T_VIRGULA: return ",";
    case T_ABRE_PAREN: return "(";
    case T_FECHA_PAREN: return ")";
    case T_ABRE_CHAVE: return "{";
    case T_FECHA_CHAVE: return "}";
    case T_MAIS: return "+";
    case T_MENOS: return "-";
    case T_MULT: return "*";
    case T_DIV: return "/";
    default: return "token desconhecido";
    }
}

class ParserManual {
private:
    int atual;
    YYSTYPE valorAtual;

    void avancar() {
        atual = yylex();
        valorAtual = yylval;
    }

    bool aceitar(int token) {
        if (atual == token) {
            avancar();
            return true;
        }

        return false;
    }

    void esperar(int token) {
        if (!aceitar(token)) {
            std::cerr << "Erro de sintaxe: esperado '"
                      << nomeToken(token)
                      << "', encontrado '"
                      << nomeToken(atual)
                      << "'.\n";
            std::exit(1);
        }
    }

    bool ehTipo(int token) const {
        return token == T_INT || token == T_FLOAT || token == T_VOID;
    }

    std::string consumirTipo() {
        if (atual == T_INT) {
            avancar();
            return "int";
        }

        if (atual == T_FLOAT) {
            avancar();
            return "float";
        }

        if (atual == T_VOID) {
            avancar();
            return "void";
        }

        std::cerr << "Erro de sintaxe: tipo esperado.\n";
        std::exit(1);
    }

    std::string consumirNome() {
        if (atual == T_ID) {
            std::string nome = valorAtual.texto;
            free(valorAtual.texto);
            avancar();
            return nome;
        }

        if (atual == T_MAIN) {
            avancar();
            return "main";
        }

        std::cerr << "Erro de sintaxe: identificador esperado.\n";
        std::exit(1);
    }

    No* comandoComTipo() {
        std::string tipo = consumirTipo();
        std::string nome = consumirNome();

        if (aceitar(T_ABRE_PAREN)) {
            std::vector<ParametroFuncao> parametros = parametrosFuncao();
            esperar(T_FECHA_PAREN);
            NoBloco* corpo = bloco();
            return new NoFuncao(tipo, nome, parametros, corpo);
        }

        if (tipo == "void") {
            std::cerr << "Erro de sintaxe: variavel nao pode ter tipo void.\n";
            std::exit(1);
        }

        No* inicial = nullptr;

        if (aceitar(T_ATRIB))
            inicial = expressao();

        esperar(T_PONTOVIRGULA);
        return new NoDeclaracao(tipo, nome, inicial);
    }

    std::vector<ParametroFuncao> parametrosFuncao() {
        std::vector<ParametroFuncao> parametros;

        if (atual == T_FECHA_PAREN)
            return parametros;

        while (true) {
            std::string tipo = consumirTipo();

            if (tipo == "void") {
                std::cerr << "Erro de sintaxe: parametro nao pode ter tipo void.\n";
                std::exit(1);
            }

            std::string nome = consumirNome();
            parametros.push_back({tipo, nome});

            if (!aceitar(T_VIRGULA))
                break;
        }

        return parametros;
    }

    NoBloco* bloco() {
        esperar(T_ABRE_CHAVE);
        NoBloco* b = new NoBloco();

        while (atual != YYEOF && atual != T_FECHA_CHAVE)
            b->adicionar(comando());

        esperar(T_FECHA_CHAVE);
        return b;
    }

    No* comando() {
        if (ehTipo(atual))
            return comandoComTipo();

        if (atual == T_ID) {
            std::string nome = consumirNome();
            esperar(T_ATRIB);
            No* expr = expressao();
            esperar(T_PONTOVIRGULA);
            return new NoAtribuicao(nome, expr);
        }

        if (atual == T_RETURN) {
            avancar();
            No* expr = expressao();
            esperar(T_PONTOVIRGULA);
            return new NoRetorno(expr);
        }

        std::cerr << "Erro de sintaxe: comando inesperado perto de '"
                  << nomeToken(atual)
                  << "'.\n";
        std::exit(1);
    }

    No* expressao() {
        No* no = termo();

        while (atual == T_MAIS || atual == T_MENOS) {
            std::string op = atual == T_MAIS ? "+" : "-";
            avancar();
            no = new NoOperacaoBinaria(op, no, termo());
        }

        return no;
    }

    No* termo() {
        No* no = fator();

        while (atual == T_MULT || atual == T_DIV) {
            std::string op = atual == T_MULT ? "*" : "/";
            avancar();
            no = new NoOperacaoBinaria(op, no, fator());
        }

        return no;
    }

    No* fator() {
        if (atual == T_NUMERO) {
            int valor = valorAtual.valorInteiro;
            avancar();
            return new NoNumero(valor);
        }

        if (atual == T_NUMERO_FLOAT) {
            float valor = valorAtual.valorFloat;
            avancar();
            return new NoFloat(valor);
        }

        if (atual == T_ID) {
            std::string nome = consumirNome();
            return new NoIdentificador(nome);
        }

        if (aceitar(T_ABRE_PAREN)) {
            No* no = expressao();
            esperar(T_FECHA_PAREN);
            return no;
        }

        std::cerr << "Erro de sintaxe: fator inesperado perto de '"
                  << nomeToken(atual)
                  << "'.\n";
        std::exit(1);
    }

public:
    ParserManual()
        : atual(YYEOF) {}

    NoBloco* programa() {
        avancar();
        NoBloco* raiz = new NoBloco();

        while (atual != YYEOF)
            raiz->adicionar(comando());

        return raiz;
    }
};

int yyparse(void) {
    ParserManual parser;
    NoBloco* raiz = parser.programa();

    std::cout << "\n=== ARVORE SINTATICA ABSTRATA (AST) ===\n";
    raiz->imprimir();

    TabelaSimbolos tabela;
    std::vector<std::string> erros;

    std::cout << "\n=== ANALISE SEMANTICA ===\n";
    raiz->analisarSemantica(tabela, "global", "void", erros);

    if (!erros.empty()) {
        for (const std::string& erro : erros)
            std::cerr << erro << "\n";

        tabela.listarTodos();
        delete raiz;
        return 1;
    }

    std::cout << "Analise semantica concluida sem erros.\n";
    tabela.listarTodos();

    CodigoIntermediario::limpar();
    raiz->gerarCodigo();
    CodigoIntermediario::imprimir("CODIGO INTERMEDIARIO ANTES DA OTIMIZACAO");

    std::vector<std::string> otimizado = Otimizador::otimizar(CodigoIntermediario::obterInstrucoes());
    CodigoIntermediario::definirInstrucoes(otimizado);
    CodigoIntermediario::imprimir("CODIGO INTERMEDIARIO DEPOIS DA OTIMIZACAO");

    std::vector<std::string> codigoFinal = GeradorCodigoFinal::gerar(otimizado);
    GeradorCodigoFinal::imprimir(codigoFinal);
    GeradorCodigoFinal::salvar("saida.asm", codigoFinal);

    std::cout << "\nCompilacao finalizada com sucesso. Codigo final salvo em saida.asm.\n";

    delete raiz;
    return 0;
}

int main() {
    return yyparse();
}
