#include "ast.h"
#include "../semantica/tabela_simbolos.h"
#include <iostream>
#include <sstream>
#include <ostream>
#include <map>
#include <set>
#include <algorithm>

static int contador_temp = 1;

void resetarContadorAST() { contador_temp = 1; }

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

NoNumero::NoNumero(int v) : valor(v) {}

void NoNumero::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Numero (" << valor << ")\n";
}
std::string NoNumero::gerarCodigo() { return std::to_string(valor); }
std::string NoNumero::paraExpressao() const { return std::to_string(valor); }

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

NoIdentificador::NoIdentificador(const std::string& n) : nome(n) {}

void NoIdentificador::imprimirNo(const std::string& pre, bool isLast) {
    std::cout << pre << astRamo(isLast) << "Id (" << nome << ")\n";
}
std::string NoIdentificador::gerarCodigo() { return nome; }
std::string NoIdentificador::paraExpressao() const { return nome; }

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
    No* ne = esq->otimizar();
    if (ne != esq) { delete esq; esq = ne; }

    No* nd = dir->otimizar();
    if (nd != dir) { delete dir; dir = nd; }

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

    if (op == "+") {
        if (ehZero(dir)) {
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x + 0 = x]\n";
            return r;
        }
        if (ehZero(esq)) {
            std::string antes = paraExpressao();
            No* r = dir; dir = nullptr; delete esq; esq = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [0 + x = x]\n";
            return r;
        }
    }
    else if (op == "-") {
        if (ehZero(dir)) {
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x - 0 = x]\n";
            return r;
        }
        if (mesmosId(esq, dir)) {
            std::string antes = paraExpressao();
            delete esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  0  [x - x = 0]\n";
            return new NoNumero(0);
        }
    }
    else if (op == "*") {
        if (ehUm(dir)) {
            std::string antes = paraExpressao();
            No* r = esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [x * 1 = x]\n";
            return r;
        }
        if (ehUm(esq)) {
            std::string antes = paraExpressao();
            No* r = dir; dir = nullptr; delete esq; esq = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  " << r->paraExpressao()
                      << "  [1 * x = x]\n";
            return r;
        }
        if (ehZero(dir) || ehZero(esq)) {
            std::string antes = paraExpressao();
            delete esq; esq = nullptr; delete dir; dir = nullptr;
            std::cout << "  [SIMPL. ALGEBRICA] " << antes
                      << "  ->  0  [x * 0 = 0]\n";
            return new NoNumero(0);
        }
    }
    else if (op == "/") {
        if (ehUm(dir)) {
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

static std::ostream*   asm_saida     = nullptr;
static TabelaSimbolos* asm_tabela    = nullptr;
static std::string     asm_escopo    = "global";
static std::map<std::string, int> asm_offsets;
static std::string     asm_ret_label;

void inicializarGeradorAssembly(std::ostream& saida, TabelaSimbolos& tabela) {
    asm_saida     = &saida;
    asm_tabela    = &tabela;
    asm_escopo    = "global";
    asm_offsets.clear();
    asm_ret_label = "";
}

static std::string refVar(const std::string& nome) {
    auto it = asm_offsets.find(nome);
    if (it != asm_offsets.end())
        return std::to_string(it->second) + "(%rbp)";
    return "0(%rbp)";
}

static void construirOffsets(const std::string& funcNome) {
    asm_offsets.clear();
    std::vector<EntradaSimbolo> vars = asm_tabela->obterPorEscopo(funcNome);
    std::sort(vars.begin(), vars.end(),
              [](const EntradaSimbolo& a, const EntradaSimbolo& b){
                  return a.endereco < b.endereco;
              });
    int offset = -4;
    for (const auto& v : vars) {
        asm_offsets[v.nome] = offset;
        offset -= 4;
    }
}

void NoNumero::gerarAssembly() {
    *asm_saida << "\tmovl\t$" << valor << ", %eax\n";
}

void NoFloat::gerarAssembly() {
    *asm_saida << "\tmovl\t$" << static_cast<int>(valor)
               << ", %eax\t\t# float " << valor << " truncado para int\n";
}

void NoIdentificador::gerarAssembly() {
    *asm_saida << "\tmovl\t" << refVar(nome) << ", %eax\t\t# " << nome << "\n";
}

void NoOperacaoBinaria::gerarAssembly() {
    if (esq) esq->gerarAssembly();
    *asm_saida << "\tpushq\t%rax\t\t\t# salva operando esquerdo\n";

    if (dir) dir->gerarAssembly();

    *asm_saida << "\tpopq\t%rcx\t\t\t# restaura operando esquerdo\n";

    if (op == "+") {
        *asm_saida << "\taddl\t%ecx, %eax\t\t# eax = left + right\n";

    } else if (op == "-") {
        *asm_saida << "\tsubl\t%eax, %ecx\t\t# ecx = left - right\n";
        *asm_saida << "\tmovl\t%ecx, %eax\n";

    } else if (op == "*") {
        *asm_saida << "\timull\t%ecx, %eax\t\t# eax = left * right\n";

    } else if (op == "/") {
        *asm_saida << "\txchgl\t%eax, %ecx\t\t# troca: eax=dividendo, ecx=divisor\n";
        *asm_saida << "\tcdq\t\t\t\t# estende sinal eax -> edx:eax\n";
        *asm_saida << "\tidivl\t%ecx\t\t\t# eax = left / right\n";
    }
}

void NoDeclaracao::gerarAssembly() {
    if (asm_escopo == "global") {
        *asm_saida << "\t.align\t4\n";
        *asm_saida << "\t.globl\t" << nome << "\n";
        *asm_saida << nome << ":\n";
        if (tipo == "float") {
            float fval = 0.0f;
            if (NoFloat* nf = dynamic_cast<NoFloat*>(valorInicial))
                fval = nf->valor;
            *asm_saida << "\t.float\t" << fval << "\n";
        } else {
            int ival = 0;
            if (NoNumero* ni = dynamic_cast<NoNumero*>(valorInicial))
                ival = ni->valor;
            *asm_saida << "\t.long\t" << ival << "\n";
        }
        return;
    }

    *asm_saida << "\t# " << tipo << " " << nome;
    if (valorInicial) *asm_saida << " = " << valorInicial->paraExpressao();
    *asm_saida << "\n";

    if (!valorInicial) {
        *asm_saida << "\tmovl\t$0, " << refVar(nome) << "\n";
    } else if (NoNumero* n = dynamic_cast<NoNumero*>(valorInicial)) {
        *asm_saida << "\tmovl\t$" << n->valor << ", " << refVar(nome) << "\n";
    } else {
        valorInicial->gerarAssembly();
        *asm_saida << "\tmovl\t%eax, " << refVar(nome) << "\n";
    }
}

void NoAtribuicao::gerarAssembly() {
    *asm_saida << "\t# " << nome << " = " << expressao->paraExpressao() << "\n";
    if (expressao) expressao->gerarAssembly();
    *asm_saida << "\tmovl\t%eax, " << refVar(nome) << "\n";
}

void NoRetorno::gerarAssembly() {
    *asm_saida << "\t# return " << (expressao ? expressao->paraExpressao() : "") << "\n";
    if (expressao) expressao->gerarAssembly();
    *asm_saida << "\tjmp\t" << asm_ret_label << "\n";
}

void NoBloco::gerarAssembly() {
    if (asm_escopo == "global") {
        bool temGlobal = false;
        for (No* c : comandos)
            if (dynamic_cast<NoDeclaracao*>(c)) { temGlobal = true; break; }

        if (temGlobal) {
            *asm_saida << "\t.section\t.data\n";
            for (No* c : comandos)
                if (dynamic_cast<NoDeclaracao*>(c)) c->gerarAssembly();
        }

        *asm_saida << "\n\t.section\t.text\n";
        for (No* c : comandos)
            if (dynamic_cast<NoFuncao*>(c)) c->gerarAssembly();
    } else {
        for (No* c : comandos)
            c->gerarAssembly();
    }
}

void NoFuncao::gerarAssembly() {
    std::string escopoAnterior = asm_escopo;
    asm_escopo    = nome;
    asm_ret_label = ".L" + nome + "_ret";

    construirOffsets(nome);

    std::set<std::string> locais;
    if (corpo) {
        for (No* c : corpo->comandos)
            if (NoDeclaracao* d = dynamic_cast<NoDeclaracao*>(c))
                locais.insert(d->nome);
    }

    std::vector<EntradaSimbolo> vars = asm_tabela->obterPorEscopo(nome);
    std::sort(vars.begin(), vars.end(),
              [](const EntradaSimbolo& a, const EntradaSimbolo& b){
                  return a.endereco < b.endereco;
              });

    std::vector<std::string> params;
    for (const auto& v : vars)
        if (locais.find(v.nome) == locais.end())
            params.push_back(v.nome);

    int numVars   = static_cast<int>(vars.size());
    int frameSize = numVars > 0 ? (((numVars * 4) + 15) / 16) * 16 : 0;

    *asm_saida << "\n\t.globl\t" << nome << "\n";
    *asm_saida << nome << ":\n";

    *asm_saida << "\tpushq\t%rbp\n";
    *asm_saida << "\tmovq\t%rsp, %rbp\n";
    if (frameSize > 0)
        *asm_saida << "\tsubq\t$" << frameSize << ", %rsp\t\t# "
                   << numVars << " variavel(is) * 4 bytes\n";

    static const char* regParam[] = {"%edi","%esi","%edx","%ecx","%r8d","%r9d"};
    for (size_t i = 0; i < params.size() && i < 6; ++i) {
        *asm_saida << "\tmovl\t" << regParam[i] << ", " << refVar(params[i])
                   << "\t\t# parametro: " << params[i] << "\n";
    }

    if (corpo) corpo->gerarAssembly();

    *asm_saida << asm_ret_label << ":\n";
    *asm_saida << "\tmovq\t%rbp, %rsp\n";
    *asm_saida << "\tpopq\t%rbp\n";
    *asm_saida << "\tret\n";

    asm_escopo = escopoAnterior;
}
