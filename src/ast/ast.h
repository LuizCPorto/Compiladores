#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iosfwd>

class TabelaSimbolos;

void inicializarGeradorAssembly(std::ostream& saida, TabelaSimbolos& tabela);

inline std::string astRamo(bool isLast)   { return isLast ? "L-- " : "|-- "; }
inline std::string astIndent(bool isLast) { return isLast ? "    " : "|   "; }

void resetarContadorAST();

class No {
public:
    virtual ~No() {}

    void imprimir() { imprimirNo("", true); }

    virtual void imprimirNo(const std::string& pre, bool isLast) = 0;
    virtual std::string gerarCodigo() = 0;
    virtual std::string paraExpressao() const = 0;
    virtual No* otimizar() { return this; }
    virtual void gerarAssembly() {}
};

class NoNumero : public No {
public:
    int valor;
    explicit NoNumero(int v);
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    void gerarAssembly() override;
};

class NoFloat : public No {
public:
    float valor;
    explicit NoFloat(float v);
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    void gerarAssembly() override;
};

class NoIdentificador : public No {
public:
    std::string nome;
    explicit NoIdentificador(const std::string& n);
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    void gerarAssembly() override;
};

class NoOperacaoBinaria : public No {
public:
    std::string op;
    No* esq;
    No* dir;
    NoOperacaoBinaria(const std::string& o, No* e, No* d);
    ~NoOperacaoBinaria() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    No* otimizar() override;
    void gerarAssembly() override;
};

class NoDeclaracao : public No {
public:
    std::string tipo;
    std::string nome;
    No* valorInicial;
    NoDeclaracao(const std::string& tipo, const std::string& nome, No* val = nullptr);
    ~NoDeclaracao() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    No* otimizar() override;
    void gerarAssembly() override;
};

class NoAtribuicao : public No {
public:
    std::string nome;
    No* expressao;
    NoAtribuicao(const std::string& nome, No* expr);
    ~NoAtribuicao() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    No* otimizar() override;
    void gerarAssembly() override;
};

class NoRetorno : public No {
public:
    No* expressao;
    explicit NoRetorno(No* expr);
    ~NoRetorno() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    No* otimizar() override;
    void gerarAssembly() override;
};

class NoBloco : public No {
public:
    std::vector<No*> comandos;
    void adicionar(No* cmd);
    ~NoBloco() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    No* otimizar() override;
    void gerarAssembly() override;
};

class NoFuncao : public No {
public:
    std::string tipo;
    std::string nome;
    NoBloco* corpo;
    NoFuncao(const std::string& tipo, const std::string& nome, NoBloco* corpo);
    ~NoFuncao() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string paraExpressao() const override;
    No* otimizar() override;
    void gerarAssembly() override;
};

#endif
