#include "ast.h"

#include "../intermediario/codigo_intermediario.h"
#include "../semantica/tabela_simbolos.h"

#include <iostream>
#include <sstream>

static bool tiposCompativeis(const std::string& destino, const std::string& origem) {
    if (destino == origem)
        return true;

    return destino == "float" && origem == "int";
}

NoNumero::NoNumero(int v)
    : valor(v) {}

void NoNumero::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Numero (" << valor << ")\n";
}

std::string NoNumero::gerarCodigo() {
    return std::to_string(valor);
}

std::string NoNumero::obterTipo() {
    return "int";
}

bool NoNumero::analisarSemantica(
    TabelaSimbolos&,
    const std::string&,
    const std::string&,
    std::vector<std::string>&
) {
    return true;
}

NoFloat::NoFloat(float v)
    : valor(v) {}

void NoFloat::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Float (" << gerarCodigo() << ")\n";
}

std::string NoFloat::gerarCodigo() {
    std::ostringstream oss;
    oss << valor;

    return oss.str();
}

std::string NoFloat::obterTipo() {
    return "float";
}

bool NoFloat::analisarSemantica(
    TabelaSimbolos&,
    const std::string&,
    const std::string&,
    std::vector<std::string>&
) {
    return true;
}

NoIdentificador::NoIdentificador(const std::string& n, const std::string& t)
    : nome(n), tipo(t) {}

void NoIdentificador::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Id (" << nome << ")\n";
}

std::string NoIdentificador::gerarCodigo() {
    return nome;
}

std::string NoIdentificador::obterTipo() {
    return tipo.empty() ? "indefinido" : tipo;
}

bool NoIdentificador::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string& escopo,
    const std::string&,
    std::vector<std::string>& erros
) {
    if (!tabela.existe(nome, escopo)) {
        erros.push_back("Erro semantico: variavel '" + nome + "' nao declarada.");
        tipo = "indefinido";
        return false;
    }

    tipo = tabela.obterTipo(nome, escopo);
    std::cout << "OK: " << nome << " declarado.\n";
    return true;
}

NoOperacaoBinaria::NoOperacaoBinaria(const std::string& o, No* e, No* d)
    : op(o), esq(e), dir(d), tipoResultado("indefinido") {}

NoOperacaoBinaria::~NoOperacaoBinaria() {
    delete esq;
    delete dir;
}

void NoOperacaoBinaria::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Op (" << op << ")\n";

    std::string p = pre + astIndent(isLast);

    if (esq)
        esq->imprimirNo(p, false);

    if (dir)
        dir->imprimirNo(p, true);
}

std::string NoOperacaoBinaria::gerarCodigo() {
    std::string esquerda = esq ? esq->gerarCodigo() : "";
    std::string direita = dir ? dir->gerarCodigo() : "";
    std::string temp = CodigoIntermediario::novoTemp();

    CodigoIntermediario::adicionar(temp + " = " + esquerda + " " + op + " " + direita);

    return temp;
}

std::string NoOperacaoBinaria::obterTipo() {
    return tipoResultado;
}

bool NoOperacaoBinaria::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string& escopo,
    const std::string& tipoFuncao,
    std::vector<std::string>& erros
) {
    bool okEsq = esq ? esq->analisarSemantica(tabela, escopo, tipoFuncao, erros) : false;
    bool okDir = dir ? dir->analisarSemantica(tabela, escopo, tipoFuncao, erros) : false;

    std::string t1 = esq ? esq->obterTipo() : "indefinido";
    std::string t2 = dir ? dir->obterTipo() : "indefinido";

    if (t1 == "indefinido" || t2 == "indefinido") {
        tipoResultado = "indefinido";
        return false;
    }

    tipoResultado = (t1 == "float" || t2 == "float") ? "float" : "int";
    return okEsq && okDir;
}

NoDeclaracao::NoDeclaracao(const std::string& t, const std::string& n, No* val)
    : tipo(t), nome(n), valorInicial(val) {}

NoDeclaracao::~NoDeclaracao() {
    delete valorInicial;
}

void NoDeclaracao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Declaracao (" << tipo << " " << nome << ")\n";

    if (valorInicial)
        valorInicial->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoDeclaracao::gerarCodigo() {
    if (valorInicial) {
        std::string valor = valorInicial->gerarCodigo();
        CodigoIntermediario::adicionar(nome + " = " + valor);
    }

    return nome;
}

std::string NoDeclaracao::obterTipo() {
    return tipo;
}

bool NoDeclaracao::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string& escopo,
    const std::string& tipoFuncao,
    std::vector<std::string>& erros
) {
    bool ok = tabela.inserirIdentificador(nome, tipo, escopo, escopo == "global" ? "global" : "local");

    if (!ok)
        erros.push_back("Erro semantico: variavel '" + nome + "' ja declarada no escopo '" + escopo + "'.");
    else
        std::cout << "OK: " << nome << " declarado.\n";

    if (valorInicial) {
        bool exprOk = valorInicial->analisarSemantica(tabela, escopo, tipoFuncao, erros);
        std::string tipoExpr = valorInicial->obterTipo();

        if (exprOk && !tiposCompativeis(tipo, tipoExpr)) {
            erros.push_back(
                "Erro semantico: nao e possivel inicializar '" + nome +
                "' do tipo " + tipo + " com expressao do tipo " + tipoExpr + "."
            );
            ok = false;
        }
    }

    return ok;
}

NoAtribuicao::NoAtribuicao(const std::string& n, No* expr)
    : nome(n), expressao(expr), tipoDestino("indefinido") {}

NoAtribuicao::~NoAtribuicao() {
    delete expressao;
}

void NoAtribuicao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Atribuicao (" << nome << " =)\n";

    if (expressao)
        expressao->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoAtribuicao::gerarCodigo() {
    std::string valor = expressao ? expressao->gerarCodigo() : "";
    CodigoIntermediario::adicionar(nome + " = " + valor);

    return nome;
}

std::string NoAtribuicao::obterTipo() {
    return "void";
}

bool NoAtribuicao::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string& escopo,
    const std::string& tipoFuncao,
    std::vector<std::string>& erros
) {
    bool ok = true;

    if (!tabela.existe(nome, escopo)) {
        erros.push_back("Erro semantico: variavel '" + nome + "' nao declarada.");
        ok = false;
    } else {
        tipoDestino = tabela.obterTipo(nome, escopo);
    }

    bool exprOk = expressao ? expressao->analisarSemantica(tabela, escopo, tipoFuncao, erros) : false;
    std::string tipoExpr = expressao ? expressao->obterTipo() : "indefinido";

    if (ok && exprOk && !tiposCompativeis(tipoDestino, tipoExpr)) {
        erros.push_back(
            "Erro semantico: variavel '" + nome + "' do tipo " + tipoDestino +
            " nao pode receber expressao do tipo " + tipoExpr + "."
        );
        ok = false;
    }

    if (ok)
        std::cout << "OK: " << nome << " recebe valor " << tipoExpr << ".\n";

    return ok && exprOk;
}

NoRetorno::NoRetorno(No* expr)
    : expressao(expr) {}

NoRetorno::~NoRetorno() {
    delete expressao;
}

void NoRetorno::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Retorno\n";

    if (expressao)
        expressao->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoRetorno::gerarCodigo() {
    std::string valor = expressao ? expressao->gerarCodigo() : "";
    CodigoIntermediario::adicionar("return " + valor);

    return "";
}

std::string NoRetorno::obterTipo() {
    return expressao ? expressao->obterTipo() : "void";
}

bool NoRetorno::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string& escopo,
    const std::string& tipoFuncao,
    std::vector<std::string>& erros
) {
    bool ok = expressao ? expressao->analisarSemantica(tabela, escopo, tipoFuncao, erros) : true;
    std::string tipoRetorno = expressao ? expressao->obterTipo() : "void";

    if (!tiposCompativeis(tipoFuncao, tipoRetorno)) {
        erros.push_back(
            "Erro semantico: funcao do tipo " + tipoFuncao +
            " nao pode retornar " + tipoRetorno + "."
        );
        return false;
    }

    return ok;
}

void NoBloco::adicionar(No* cmd) {
    if (cmd)
        comandos.push_back(cmd);
}

NoBloco::~NoBloco() {
    for (No* c : comandos)
        delete c;
}

void NoBloco::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Bloco\n";

    std::string p = pre + astIndent(isLast);

    for (size_t i = 0; i < comandos.size(); i++)
        comandos[i]->imprimirNo(p, i == comandos.size() - 1);
}

std::string NoBloco::gerarCodigo() {
    for (No* c : comandos)
        c->gerarCodigo();

    return "";
}

std::string NoBloco::obterTipo() {
    return "void";
}

bool NoBloco::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string& escopo,
    const std::string& tipoFuncao,
    std::vector<std::string>& erros
) {
    bool ok = true;

    for (No* c : comandos) {
        if (!c->analisarSemantica(tabela, escopo, tipoFuncao, erros))
            ok = false;
    }

    return ok;
}

NoFuncao::NoFuncao(
    const std::string& t,
    const std::string& n,
    const std::vector<ParametroFuncao>& p,
    NoBloco* c
)
    : tipo(t), nome(n), parametros(p), corpo(c) {}

NoFuncao::NoFuncao(const std::string& t, const std::string& n, NoBloco* c)
    : tipo(t), nome(n), corpo(c) {}

NoFuncao::~NoFuncao() {
    delete corpo;
}

void NoFuncao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Funcao (" << tipo << " " << nome << ")\n";

    std::string p = pre + astIndent(isLast);

    for (size_t i = 0; i < parametros.size(); i++) {
        std::cout << p
                  << astRamo(corpo == nullptr && i == parametros.size() - 1)
                  << "Parametro ("
                  << parametros[i].tipo
                  << " "
                  << parametros[i].nome
                  << ")\n";
    }

    if (corpo)
        corpo->imprimirNo(p, true);
}

std::string NoFuncao::gerarCodigo() {
    CodigoIntermediario::adicionar("func " + tipo + " " + nome);

    if (corpo)
        corpo->gerarCodigo();

    CodigoIntermediario::adicionar("endfunc");

    return "";
}

std::string NoFuncao::obterTipo() {
    return tipo;
}

bool NoFuncao::analisarSemantica(
    TabelaSimbolos& tabela,
    const std::string&,
    const std::string&,
    std::vector<std::string>& erros
) {
    bool ok = tabela.inserirIdentificador(nome, tipo, "global", "funcao");

    if (!ok) {
        erros.push_back("Erro semantico: funcao '" + nome + "' ja declarada.");
    }

    for (const ParametroFuncao& p : parametros) {
        if (!tabela.inserirIdentificador(p.nome, p.tipo, nome, "parametro")) {
            erros.push_back(
                "Erro semantico: parametro '" + p.nome +
                "' ja declarado na funcao '" + nome + "'."
            );
            ok = false;
        }
    }

    if (corpo && !corpo->analisarSemantica(tabela, nome, tipo, erros))
        ok = false;

    return ok;
}
