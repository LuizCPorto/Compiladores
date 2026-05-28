#include "ast.h"
#include <iostream>
#include <sstream>

static int contador_temp = 1;

// ===================== NoNumero =====================

NoNumero::NoNumero(int v) : valor(v) {}

void NoNumero::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Numero (" << valor << ")\n";
}

std::string NoNumero::gerarCodigo() {
    return std::to_string(valor);
}

// ===================== NoFloat =====================

NoFloat::NoFloat(float v) : valor(v) {}

void NoFloat::imprimirNo(const std::string& pre, bool isLast) {
    std::ostringstream oss;
    oss << valor;
    std::cout << pre << astRamo(isLast) << "Float (" << oss.str() << ")\n";
}

std::string NoFloat::gerarCodigo() {
    std::ostringstream oss;
    oss << valor;
    return oss.str();
}

// ===================== NoIdentificador =====================

NoIdentificador::NoIdentificador(const std::string& n) : nome(n) {}

void NoIdentificador::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Id (" << nome << ")\n";
}

std::string NoIdentificador::gerarCodigo() {
    return nome;
}

// ===================== NoOperacaoBinaria =====================

NoOperacaoBinaria::NoOperacaoBinaria(const std::string& o, No* e, No* d)
    : op(o), esq(e), dir(d) {}

NoOperacaoBinaria::~NoOperacaoBinaria() {
    delete esq;
    delete dir;
}

void NoOperacaoBinaria::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Op (" << op << ")\n";
    std::string p = pre + astIndent(isLast);
    esq->imprimirNo(p, false);
    dir->imprimirNo(p, true);
}

std::string NoOperacaoBinaria::gerarCodigo() {
    std::string le = esq ? esq->gerarCodigo() : "";
    std::string ld = dir ? dir->gerarCodigo() : "";
    std::string tmp = "t" + std::to_string(contador_temp++);
    std::cout << tmp << " = " << le << " " << op << " " << ld << "\n";
    return tmp;
}

// ===================== NoDeclaracao =====================

NoDeclaracao::NoDeclaracao(const std::string& t, const std::string& n, No* val)
    : tipo(t), nome(n), valorInicial(val) {}

NoDeclaracao::~NoDeclaracao() {
    delete valorInicial;
}

void NoDeclaracao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast)
              << "Declaracao (" << tipo << " " << nome << ")\n";
    if (valorInicial)
        valorInicial->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoDeclaracao::gerarCodigo() {
    if (valorInicial) {
        std::string val = valorInicial->gerarCodigo();
        std::cout << nome << " = " << val << "\n";
    }
    return nome;
}

// ===================== NoAtribuicao =====================

NoAtribuicao::NoAtribuicao(const std::string& n, No* expr)
    : nome(n), expressao(expr) {}

NoAtribuicao::~NoAtribuicao() {
    delete expressao;
}

void NoAtribuicao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Atribuicao (" << nome << " =)\n";
    expressao->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoAtribuicao::gerarCodigo() {
    std::string val = expressao->gerarCodigo();
    std::cout << nome << " = " << val << "\n";
    return nome;
}

// ===================== NoRetorno =====================

NoRetorno::NoRetorno(No* expr) : expressao(expr) {}

NoRetorno::~NoRetorno() {
    delete expressao;
}

void NoRetorno::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Retorno\n";
    if (expressao)
        expressao->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoRetorno::gerarCodigo() {
    std::string val = expressao ? expressao->gerarCodigo() : "";
    std::cout << "return " << val << "\n";
    return "";
}

// ===================== NoBloco =====================

void NoBloco::adicionar(No* cmd) {
    if (cmd) comandos.push_back(cmd);
}

NoBloco::~NoBloco() {
    for (No* c : comandos) delete c;
}

void NoBloco::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Bloco\n";
    std::string p = pre + astIndent(isLast);
    for (size_t i = 0; i < comandos.size(); i++)
        comandos[i]->imprimirNo(p, i == comandos.size() - 1);
}

std::string NoBloco::gerarCodigo() {
    for (No* c : comandos) c->gerarCodigo();
    return "";
}

// ===================== NoFuncao =====================

NoFuncao::NoFuncao(const std::string& t, const std::string& n, NoBloco* c)
    : tipo(t), nome(n), corpo(c) {}

NoFuncao::~NoFuncao() {
    delete corpo;
}

void NoFuncao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast)
              << "Funcao (" << tipo << " " << nome << ")\n";
    if (corpo)
        corpo->imprimirNo(pre + astIndent(isLast), true);
}

std::string NoFuncao::gerarCodigo() {
    std::cout << "\nfunc " << tipo << " " << nome << ":\n";
    if (corpo) corpo->gerarCodigo();
    std::cout << "endfunc\n";
    return "";
}
