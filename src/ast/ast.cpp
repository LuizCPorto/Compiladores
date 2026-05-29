#include "ast.h"
#include <iostream>
#include <sstream>

static int contador_temp = 1;

void resetarContadorAST() { contador_temp = 1; }

// ============================================================
// Helpers internos para otimizacao
// ============================================================

static bool ehZero(const No* n) {
    const NoNumero* nn = dynamic_cast<const NoNumero*>(n);
    return nn && nn->valor == 0;
}
static bool ehUm(const No* n) {
    const NoNumero* nn = dynamic_cast<const NoNumero*>(n);
    return nn && nn->valor == 1;
}
static bool mesmosId(const No* a, const No* b) {
    const NoIdentificador* ia = dynamic_cast<const NoIdentificador*>(a);
    const NoIdentificador* ib = dynamic_cast<const NoIdentificador*>(b);
    return ia && ib && ia->nome == ib->nome;
}

// ===================== NoNumero =====================

NoNumero::NoNumero(int v) : valor(v) {}

void NoNumero::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Numero (" << valor << ")\n";
}
std::string NoNumero::gerarCodigo() { return std::to_string(valor); }
std::string NoNumero::paraExpressao() const { return std::to_string(valor); }

// ===================== NoFloat =====================

NoFloat::NoFloat(float v) : valor(v) {}

void NoFloat::imprimirNo(const std::string& pre, bool isLast) {
    std::ostringstream oss; oss << valor;
    std::cout << pre << astRamo(isLast) << "Float (" << oss.str() << ")\n";
}
std::string NoFloat::gerarCodigo() {
    std::ostringstream oss; oss << valor; return oss.str();
}
std::string NoFloat::paraExpressao() const {
    std::ostringstream oss; oss << valor; return oss.str();
}

// ===================== NoIdentificador =====================

NoIdentificador::NoIdentificador(const std::string& n) : nome(n) {}

void NoIdentificador::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Id (" << nome << ")\n";
}
std::string NoIdentificador::gerarCodigo() { return nome; }
std::string NoIdentificador::paraExpressao() const { return nome; }

// ===================== NoOperacaoBinaria =====================

NoOperacaoBinaria::NoOperacaoBinaria(const std::string& o, No* e, No* d)
    : op(o), esq(e), dir(d) {}

NoOperacaoBinaria::~NoOperacaoBinaria() { delete esq; delete dir; }

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

std::string NoOperacaoBinaria::paraExpressao() const {
    return "(" + (esq ? esq->paraExpressao() : "?")
         + " " + op + " "
         + (dir ? dir->paraExpressao() : "?") + ")";
}

No* NoOperacaoBinaria::otimizar() {
    // 1. Otimiza os filhos primeiro (pós-ordem)
    No* ne = esq->otimizar();
    if (ne != esq) { delete esq; esq = ne; }

    No* nd = dir->otimizar();
    if (nd != dir) { delete dir; dir = nd; }

    // 2. Dobramento de constantes: dois números -> avalia agora
    NoNumero* numEsq = dynamic_cast<NoNumero*>(esq);
    NoNumero* numDir = dynamic_cast<NoNumero*>(dir);
    if (numEsq && numDir) {
        int resultado = 0;
        bool valido = true;
        if      (op == "+") resultado = numEsq->valor + numDir->valor;
        else if (op == "-") resultado = numEsq->valor - numDir->valor;
        else if (op == "*") resultado = numEsq->valor * numDir->valor;
        else if (op == "/" && numDir->valor != 0) resultado = numEsq->valor / numDir->valor;
        else valido = false;

        if (valido) {
            std::cout << "  [DOBRAMENTO]      " << paraExpressao()
                      << "  ->  " << resultado << "\n";
            esq = nullptr; dir = nullptr;
            return new NoNumero(resultado);
        }
    }

    // 3. Simplificações algébricas
    if (op == "+") {
        if (ehZero(dir)) {                              // x + 0 = x
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x + 0 = x]\n";
            return r;
        }
        if (ehZero(esq)) {                              // 0 + x = x
            std::string antes = paraExpressao();
            No* r = dir; dir = nullptr; delete esq; esq = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [0 + x = x]\n";
            return r;
        }
    }
    else if (op == "-") {
        if (ehZero(dir)) {                              // x - 0 = x
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x - 0 = x]\n";
            return r;
        }
        if (mesmosId(esq, dir)) {                       // x - x = 0
            std::string antes = paraExpressao();
            delete esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  0  [x - x = 0]\n";
            return new NoNumero(0);
        }
    }
    else if (op == "*") {
        if (ehUm(dir)) {                                // x * 1 = x
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x * 1 = x]\n";
            return r;
        }
        if (ehUm(esq)) {                                // 1 * x = x
            std::string antes = paraExpressao();
            No* r = dir; dir = nullptr; delete esq; esq = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [1 * x = x]\n";
            return r;
        }
        if (ehZero(dir) || ehZero(esq)) {              // x * 0 = 0
            std::string antes = paraExpressao();
            delete esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  0  [x * 0 = 0]\n";
            return new NoNumero(0);
        }
    }
    else if (op == "/") {
        if (ehUm(dir)) {                                // x / 1 = x
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x / 1 = x]\n";
            return r;
        }
    }

    return this;
}

// ===================== NoDeclaracao =====================

NoDeclaracao::NoDeclaracao(const std::string& t, const std::string& n, No* val)
    : tipo(t), nome(n), valorInicial(val) {}
NoDeclaracao::~NoDeclaracao() { delete valorInicial; }

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
std::string NoDeclaracao::paraExpressao() const {
    std::string s = tipo + " " + nome;
    if (valorInicial) s += " = " + valorInicial->paraExpressao();
    return s;
}
No* NoDeclaracao::otimizar() {
    if (valorInicial) {
        No* otim = valorInicial->otimizar();
        if (otim != valorInicial) { delete valorInicial; valorInicial = otim; }
    }
    return this;
}

// ===================== NoAtribuicao =====================

NoAtribuicao::NoAtribuicao(const std::string& n, No* expr)
    : nome(n), expressao(expr) {}
NoAtribuicao::~NoAtribuicao() { delete expressao; }

void NoAtribuicao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Atribuicao (" << nome << " =)\n";
    expressao->imprimirNo(pre + astIndent(isLast), true);
}
std::string NoAtribuicao::gerarCodigo() {
    std::string val = expressao->gerarCodigo();
    std::cout << nome << " = " << val << "\n";
    return nome;
}
std::string NoAtribuicao::paraExpressao() const {
    return nome + " = " + expressao->paraExpressao();
}
No* NoAtribuicao::otimizar() {
    if (expressao) {
        No* otim = expressao->otimizar();
        if (otim != expressao) { delete expressao; expressao = otim; }
    }
    return this;
}

// ===================== NoRetorno =====================

NoRetorno::NoRetorno(No* expr) : expressao(expr) {}
NoRetorno::~NoRetorno() { delete expressao; }

void NoRetorno::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Retorno\n";
    if (expressao) expressao->imprimirNo(pre + astIndent(isLast), true);
}
std::string NoRetorno::gerarCodigo() {
    std::string val = expressao ? expressao->gerarCodigo() : "";
    std::cout << "return " << val << "\n";
    return "";
}
std::string NoRetorno::paraExpressao() const {
    return "return " + (expressao ? expressao->paraExpressao() : "");
}
No* NoRetorno::otimizar() {
    if (expressao) {
        No* otim = expressao->otimizar();
        if (otim != expressao) { delete expressao; expressao = otim; }
    }
    return this;
}

// ===================== NoBloco =====================

void NoBloco::adicionar(No* cmd) { if (cmd) comandos.push_back(cmd); }
NoBloco::~NoBloco() { for (No* c : comandos) delete c; }

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
std::string NoBloco::paraExpressao() const {
    std::string s;
    for (auto* c : comandos) s += c->paraExpressao() + "; ";
    return s;
}

No* NoBloco::otimizar() {
    std::vector<No*> novos;
    bool morto = false;

    for (No* cmd : comandos) {
        if (morto) {
            std::cout << "  [CODIGO MORTO]     " << cmd->paraExpressao()
                      << "  [instrucao apos return]\n";
            delete cmd;
            continue;
        }
        No* otim = cmd->otimizar();
        if (otim != cmd) { delete cmd; }
        novos.push_back(otim);
        if (dynamic_cast<NoRetorno*>(otim)) morto = true;
    }

    comandos = novos;
    return this;
}

// ===================== NoFuncao =====================

NoFuncao::NoFuncao(const std::string& t, const std::string& n, NoBloco* c)
    : tipo(t), nome(n), corpo(c) {}
NoFuncao::~NoFuncao() { delete corpo; }

void NoFuncao::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast)
              << "Funcao (" << tipo << " " << nome << ")\n";
    if (corpo) corpo->imprimirNo(pre + astIndent(isLast), true);
}
std::string NoFuncao::gerarCodigo() {
    std::cout << "\nfunc " << tipo << " " << nome << ":\n";
    if (corpo) corpo->gerarCodigo();
    std::cout << "endfunc\n";
    return "";
}
std::string NoFuncao::paraExpressao() const {
    return tipo + " " + nome + "(...)";
}
No* NoFuncao::otimizar() {
    if (corpo) corpo->otimizar();
    return this;
}
