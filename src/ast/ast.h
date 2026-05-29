#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

class TabelaSimbolos;

inline std::string astRamo(bool isLast) {
    return isLast ? "L-- " : "|-- ";
}

inline std::string astIndent(bool isLast) {
    return isLast ? "    " : "|   ";
}

struct ParametroFuncao {
    std::string tipo;
    std::string nome;
};

class No {
public:
    virtual ~No() {}

    void imprimir() {
        imprimirNo("", true);
    }

    virtual void imprimirNo(const std::string& pre, bool isLast) = 0;
    virtual std::string gerarCodigo() = 0;
    virtual std::string obterTipo() = 0;

    virtual bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) = 0;
};

class NoNumero : public No {
public:
    int valor;

    explicit NoNumero(int v);

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoFloat : public No {
public:
    float valor;

    explicit NoFloat(float v);

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoIdentificador : public No {
public:
    std::string nome;
    std::string tipo;

    NoIdentificador(const std::string& n, const std::string& t = "");

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoOperacaoBinaria : public No {
public:
    std::string op;
    No* esq;
    No* dir;
    std::string tipoResultado;

    NoOperacaoBinaria(const std::string& o, No* e, No* d);
    ~NoOperacaoBinaria() override;

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoOperacao : public NoOperacaoBinaria {
public:
    NoOperacao(const std::string& o, No* e, No* d)
        : NoOperacaoBinaria(o, e, d) {}
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
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoAtribuicao : public No {
public:
    std::string nome;
    No* expressao;
    std::string tipoDestino;

    NoAtribuicao(const std::string& nome, No* expr);
    ~NoAtribuicao() override;

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoRetorno : public No {
public:
    No* expressao;

    explicit NoRetorno(No* expr);
    ~NoRetorno() override;

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoBloco : public No {
public:
    std::vector<No*> comandos;

    void adicionar(No* cmd);
    ~NoBloco() override;

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

class NoFuncao : public No {
public:
    std::string tipo;
    std::string nome;
    std::vector<ParametroFuncao> parametros;
    NoBloco* corpo;

    NoFuncao(
        const std::string& tipo,
        const std::string& nome,
        const std::vector<ParametroFuncao>& parametros,
        NoBloco* corpo
    );

    NoFuncao(const std::string& tipo, const std::string& nome, NoBloco* corpo);
    ~NoFuncao() override;

    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
    std::string obterTipo() override;

    bool analisarSemantica(
        TabelaSimbolos& tabela,
        const std::string& escopo,
        const std::string& tipoFuncao,
        std::vector<std::string>& erros
    ) override;
};

#endif
